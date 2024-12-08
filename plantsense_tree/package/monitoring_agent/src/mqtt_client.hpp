#ifndef MQTT_CLIENT_HPP
#define MQTT_CLIENT_HPP

#include <string>
#include <mqtt/async_client.h>
#include <sstream>
#include "common.hpp"
#include "redis_interface.hpp"

const std::string z2m_devices_addr("zigbee2mqtt/bridge/devices"); 

extern "C" {
    void mqtt_client_signal_handler(int sig);
}

class sensor_reading {
    const returned_data_t type;
    const double temperature;
    const double humidity;
    const double light;

    public:
    static const int nothing = 0;

    sensor_reading(returned_data_t type, double temp, double humidity, double light) 
    : type(type), temperature(temp), humidity(humidity), light(light) {}

    inline const returned_data_t getTypes() { return this->type; }
    inline const double getTemp() { return this->temperature; }
    inline const double getSoilMoisture() { return this->humidity; }
    inline const double getLight() { return this->light; }
};

int mqtt_listener_subroutine(const mqtt_client_config conf, std::condition_variable * cvar, threaded_queue<device_info> * dev_queue);
bool z2m_blocking_health_check(mqtt::async_client * const client, const mqtt_client_config * const conf);
bool subscribe_to_devices(mqtt::async_client *const client, const mqtt_client_config *const conf, const std::vector<device_info> * const devices);
std::vector<device_info> register_devices(mqtt::const_message_ptr msg);
void notify_other_thread(const std::vector<device_info> * const devs, threaded_queue<device_info> * const dev_queue);
sensor_reading parse_sensor_message(returned_data_t types, mqtt::string payload);
void write_reading_to_db(redisContext * c, std::string key, sensor_reading read);

#endif //MQTT_CLIENT_HPP