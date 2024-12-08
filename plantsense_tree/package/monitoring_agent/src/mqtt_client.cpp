#include "mqtt_client.hpp"
#include <chrono>
#include <json/json.h>


std::string assemble_uri(std::string protocol, std::string addr, size_t port) {
    std::string uri = protocol + "://" + addr + ":" + std::to_string(port);
    return uri;
}

volatile sig_atomic_t halt;

extern "C" {
    void mqtt_client_signal_handler(int sig){
        halt = 1;
    }
}

int mqtt_listener_subroutine(const mqtt_client_config conf, std::condition_variable * cvar, threaded_queue<device_info> * dev_queue) {
    halt = 0;

    std::cout << "Init mqtt connection\n";
    
    mqtt::connect_options connection_options = mqtt::connect_options_builder()
        .clean_session(true)
        .automatic_reconnect(true)
        .finalize();


    mqtt::async_client client(assemble_uri("mqtt", conf.address, conf.port), "");
    client.start_consuming();

    std::cout << "a\n";
    try
    {
        mqtt::connect_response resp = client.connect(connection_options)->get_connect_response();
    }
    catch(const mqtt::exception& e)
    {
        std::cerr << "MQTT Connecton Failiure\n";
        std::cerr << e.what() << '\n';
        cvar->notify_all();
        return EXIT_FAILURE;
    }
    
    //check z2m --> next stage
    size_t count = 0;
    while (count < conf.retries)
    {
        if (z2m_blocking_health_check(&client, &conf)) {
            break;
        }
    }
    if (count == conf.retries)
    {
        // Failiure, z2m is down for good
        client.stop_consuming();
        (void)client.disconnect()->wait_for(conf.timeout_s);
        cvar->notify_all();
        return EXIT_FAILURE;
    }
    // https://www.zigbee2mqtt.io/guide/usage/mqtt_topics_and_messages.html#zigbee2mqtt-bridge-request

    client.subscribe("zigbee2mqtt/bridge/devices", mqtt_QOS::AT_LEAT_ONCE);

    mqtt::const_message_ptr msg;

    msg = client.try_consume_message_for(std::chrono::seconds(conf.timeout_s));
    std::vector<device_info> devices;
    if (msg) {
        std::cout << msg->get_payload_str() << '\n';
        devices = register_devices(msg);
        notify_other_thread(&devices, dev_queue);
    }
    else
    {
        std::cout << "Couldn't get device list from zigbee2mqtt, even though the health check passed\n";
        debug(std::cout << "nomsg\n";)
    }
    std::cout << "subs\n";
    if (!subscribe_to_devices(&client, &conf, &devices) ) {
        std::cerr << "subscription error\n";
    }

    #warning add check for return value
    while (!halt)
    {
        msg = client.try_consume_message_for(std::chrono::seconds(conf.timeout_s));
        if (!msg)
        {
            debug(std::cerr << "no message\n";);
            continue;
        }
        std::string topic = msg->get_topic();
        if (topic == z2m_devices_addr) {
            std::cout << msg->get_payload_str() << '\n';
        }
        else
        {
            std::cout << "From: " << msg->get_topic() << '\n';
            std::cout << "Message: \n" << msg->get_payload_str() << "\n===\n"; 
        }
        
    }

    std::cout << "mqtt client :: terminating\n";

    client.stop_consuming();
    client.disconnect()->wait();

    cvar->notify_all();
    return EXIT_SUCCESS;
}

std::vector<device_info> register_devices(mqtt::const_message_ptr msg){
    Json::CharReaderBuilder builder;
    Json::Value root;
    JSONCPP_STRING error;
    const std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
    if (! reader->parse(msg->get_payload_str().c_str(), msg->get_payload_str().c_str() + msg->get_payload_str().length(), &root, &error)) {
        std::cerr << "json_err :: " << error << '\n';
        #warning TODO:: implement error hadling
        return std::vector<device_info>();
    }
    
    std::vector<device_info> devices;

    // std::cout << root.isArray() << '\n';
    // std::cout << root[0] << '\n';
    
    for (auto i = root.begin(); i != root.end(); i++)
    {
        returned_data_t data = returned_data_t::NONE;
        Json::Value exposes = (*i)["definition"]["exposes"];
        for (auto j = exposes.begin(); j != exposes.end(); j++)
        {
            Json::String type = (*j)["property"].asString();
            if (type == "temperature")
            {
                data |= returned_data_t::TEMPERATURE;
                continue;
            }
            if (type == "soil_moisture")
            {
                data |= returned_data_t::HUMIDITY;
                continue;
            }
            if (type == "light_intensity")
            {
                data |= returned_data_t::LIGHT;
                continue;
            }
            
        }


        device_info tmp( (ieee_addr_t)std::stoull( (*i)["ieee_address"].asString(), NULL, 16 ), "",  data, interaction_type_t::NONE);
        devices.push_back(tmp);
        std::cout << (*i) << '\n';
        std::cout << (*i)["ieee_address"] << '\n';
        std::cout << "==\n";
    }
    

    return devices;
}

bool z2m_blocking_health_check(mqtt::async_client *const client, const mqtt_client_config * const conf)
{
    client->subscribe("zigbee2mqtt/bridge/response/health_check", mqtt_QOS::AT_LEAT_ONCE);

    std::cout << "Sending health check request\n";

    mqtt::message_ptr z2m_health_msg = mqtt::make_message("zigbee2mqtt/bridge/request/health_check", "");
    z2m_health_msg->set_qos(mqtt_QOS::EXACTLY_ONCE);
    z2m_health_msg->set_retained(true);
    client->publish(z2m_health_msg)->wait();

    std::cout << "Listening for health response\n";

    mqtt::const_message_ptr health_msg = client->try_consume_message_for(std::chrono::seconds(conf->timeout_s));

    client->unsubscribe("zigbee2mqtt/bridge/response/health_check");

    if (!health_msg){
        std::cout << "Failed to get health check\n";
        return false;
    }
    std::cout << "got health check: " << health_msg.get()->get_payload_str() << "\n";
    return true;
}

bool subscribe_to_devices(mqtt::async_client *const client, const mqtt_client_config *const conf, const std::vector<device_info> * const devices)
{
    bool ok = true;
    std::vector<mqtt::token_ptr> tokens;
    for (auto i = 0; i < (int)devices->size(); i++)
    {
        debug(std::cout << "g\n");
        debug(std::cout << "addr: " << devices->at(i).get_z2m_addr() << '\n' );
        auto token = client->subscribe( devices->at(i).get_z2m_addr(), mqtt_QOS::AT_LEAT_ONCE);
    }
    
    return ok;
}

void notify_other_thread(const std::vector<device_info> * const  devs, threaded_queue<device_info> * const dev_queue){
    for (const device_info &dev : *devs)
    {
        dev_queue->push_back(dev);
    }
}