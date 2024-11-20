#include "mqtt_client.hpp"
#include <chrono>
#include "mqtt/async_client.h"


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
    client.subscribe("zigbee2mqtt/bridge/response/health_check", mqtt_QOS::AT_LEAT_ONCE);

    std::cout << "Sending health check request\n";

    mqtt::message_ptr z2m_health_msg = mqtt::make_message("zigbee2mqtt/bridge/request/health_check", "");
    z2m_health_msg->set_qos(mqtt_QOS::EXACTLY_ONCE);
    z2m_health_msg->set_retained(true);
    client.publish(z2m_health_msg)->wait();

    std::cout << "Listening for health response\n";

    mqtt::const_message_ptr health_msg = client.try_consume_message_for(std::chrono::seconds(conf.timeout_s));

    if (!health_msg){
        std::cout << "Failed to get health check\n";
        client.disconnect()->wait_for(60000);
        exit(EXIT_FAILURE);
    }
    else {
        std::cout << "got health check: " << health_msg.get()->get_payload_str() << "\n";
    }

    client.stop_consuming();
    client.disconnect()->wait();

    exit(EXIT_SUCCESS);
}