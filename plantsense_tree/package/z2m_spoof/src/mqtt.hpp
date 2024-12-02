#ifndef MQTT_HPP
#define MQTT_HPP

#include "mqtt/async_client.h"
#include <string>

struct mqtt_client_config
{
    std::string address;
    size_t port;
    size_t timeout_s;
};

enum mqtt_QOS {
    AT_MOST_ONCE = 0,
    AT_LEAT_ONCE = 1,
    EXACTLY_ONCE = 2
};

#endif //MQTT_HPP