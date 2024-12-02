#ifndef MQTT_CLIENT_HPP
#define MQTT_CLIENT_HPP

#include <string>
#include <mqtt/async_client.h>

struct mqtt_client_config
{
    std::string address;
    size_t port;
    size_t timeout_s;
    size_t retries;
};

enum mqtt_QOS {
    AT_MOST_ONCE = 0,
    AT_LEAT_ONCE = 1,
    EXACTLY_ONCE = 2
};

[[noreturn]] void mqtt_listener_subroutine(const mqtt_client_config conf);
bool z2m_blocking_health_check(mqtt::async_client * const client, const mqtt_client_config * const conf);

#endif //MQTT_CLIENT_HPP