#ifndef MQTT_CLIENT_HPP
#define MQTT_CLIENT_HPP

#include <string>
#include <mqtt/async_client.h>
#include "common.hpp"

typedef unsigned long long ieee_addr_t;

const std::string z2m_devices_addr("zigbee2mqtt/bridge/devices"); 

extern "C" {
    void mqtt_client_signal_handler(int sig);
}
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

enum class returned_data_t {
    NONE = 0,
    TEMPERATURE = 1 << 0,
    HUMIDITY = 1 << 1, 
    LIGHT = 1 << 2
};

enum class interaction_type_t {
    NONE = 0,
    LIGHT = 1
};
 
class device_info {
    private:
    const ieee_addr_t ieee_addr; // These are unique globally per device, so we should use these as db keys
    std::string friendly_name;
    returned_data_t data_types;
    interaction_type_t interactions;
    
    public:
    static const std::string base_addr;
    device_info(ieee_addr_t ieee_addr, std::string friendly_name, returned_data_t returned_data, interaction_type_t interaction_type) 
        : ieee_addr(ieee_addr), friendly_name(friendly_name), data_types(returned_data), interactions(interaction_type) {};

    inline bool set_friendly_name(std::string new_name) {
        if (new_name.size() == 0){
            debug(std::cerr << "Device friendly name must not be empty\n";)
            return false;
        }
        this->friendly_name = new_name;
        return true;
    }

    inline const std::string getName() { return this->friendly_name; }

    std::string generate_z2m_get_addr();

};

class sensor_reading {
    const returned_data_t type;
    const double temperature;
    const double humidity;
    const double light;

    public:
    static const int nothing = 0;

    sensor_reading(returned_data_t type, double temp, double humidity, double light) 
    : type(type), temperature(temp), humidity(humidity), light(light) {}

    inline const double getTemp() { return this->temperature; }
    inline const double getHumidity() { return this->humidity; }
    inline const double getLight() { return this->light; }
};

[[noreturn]] void mqtt_listener_subroutine(const mqtt_client_config conf);
bool z2m_blocking_health_check(mqtt::async_client * const client, const mqtt_client_config * const conf);
std::vector<device_info> get_and_subscribe_connected_devices(mqtt::async_client * const client, const mqtt_client_config * const conf);


#endif //MQTT_CLIENT_HPP