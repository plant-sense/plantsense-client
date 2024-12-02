#include "mqtt_client.hpp"
#include <chrono>


std::string assemble_uri(std::string protocol, std::string addr, size_t port) {
    std::string uri = protocol + "://" + addr + ":" + std::to_string(port);
    return uri;
}

[[noreturn]] void mqtt_listener_subroutine(const mqtt_client_config conf) {
    
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

    #warning "TODO::Add return code check" 
    msg = client.try_consume_message_for(std::chrono::seconds(conf.timeout_s));

    if (msg) {
        std::cout << msg->get_payload_str() << '\n';
    }
    else
    {
        std::cout << "nomsg\n";
    }
    

    client.stop_consuming();
    client.disconnect()->wait();

    exit(EXIT_SUCCESS);
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
