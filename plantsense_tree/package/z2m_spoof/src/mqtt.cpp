#include "mqtt.hpp"

std::string assemble_uri(std::string protocol, std::string addr, size_t port) {
    std::string uri = protocol + "://" + addr + ":" + std::to_string(port);
    return uri;
}

mqtt::async_client * init_connection(const mqtt_client_config conf){
    mqtt::connect_options opts = mqtt::connect_options_builder().clean_session(true).finalize();
    mqtt::async_client *client = new mqtt::async_client(assemble_uri("mqtt", conf.address, conf.port), "");

    client->start_consuming();

    mqtt::connect_response resp = client->connect(opts)->get_connect_response();

    

    return client;
}