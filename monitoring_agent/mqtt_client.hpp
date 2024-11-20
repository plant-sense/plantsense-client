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

[[noreturn]] void mqtt_listener_subroutine(const mqtt_client_config);