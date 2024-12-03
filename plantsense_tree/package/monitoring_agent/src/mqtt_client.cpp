#include "mqtt_client.hpp"
#include <chrono>
#include <json/json.h>

const std::string device_info::base_addr = "zigbee2mqtt";

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

[[noreturn]] void mqtt_listener_subroutine(const mqtt_client_config conf) {
    halt = 0;
    std::signal(SIGTERM, mqtt_client_signal_handler);
    std::signal(SIGINT, mqtt_client_signal_handler);

    std::cout << "Init mqtt connection\n";
    
    mqtt::connect_options connection_options = mqtt::connect_options_builder()
        .clean_session(true)
        .finalize();

    mqtt::async_client client(assemble_uri("mqtt", conf.address, conf.port), "");

    client.start_consuming();
    mqtt::connect_response resp = client.connect(connection_options)->get_connect_response();

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
        // Failire, z2m is down for good
        client.stop_consuming();
        (void)client.disconnect()->wait_for(conf.timeout_s);
        exit(EXIT_FAILURE);
    }
    // https://www.zigbee2mqtt.io/guide/usage/mqtt_topics_and_messages.html#zigbee2mqtt-bridge-request

    client.subscribe("zigbee2mqtt/bridge/devices", mqtt_QOS::AT_LEAT_ONCE);

    mqtt::const_message_ptr msg;

    msg = client.try_consume_message_for(std::chrono::seconds(conf.timeout_s));

    if (msg) {
        std::cout << msg->get_payload_str() << '\n';
        register_devices(msg);
    }
    else
    {
        std::cout << "Couldn't get device list from zigbee2mqtt, even though the health check passed\n";
        debug(std::cout << "nomsg\n";)
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
    }

    std::cout << "mqtt client :: terminating\n";

    client.stop_consuming();
    client.disconnect()->wait();

    exit(EXIT_SUCCESS);
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
    
    std::cout << root.isArray() << '\n';

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

std::vector<device_info> get_and_subscribe_connected_devices(mqtt::async_client *const client, const mqtt_client_config *const conf)
{
    client->subscribe("", mqtt_QOS::AT_LEAT_ONCE);
    return std::vector<device_info>();
}
