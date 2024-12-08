#ifndef REDIS_INTEFACE_HPP
#define REDIS_INTEFACE_HPP

#include <hiredis/hiredis.h>
#include <string>
#include "common.hpp"

struct redis_interface_config
{
    std::string addr;
    int port;
};

struct data_packet{
    returned_data_t types;
    ieee_addr_t addr;
    unsigned long long temp_ts;
    double temp;
    unsigned long long soil_m_ts;
    double soil_m;
    unsigned long long light_ts;
    double light;


    inline std::string gist(void) const {
        std::stringstream ss;
        ss << "From:: " << ieee_to_hex(this->addr) << '\n';
        if ((this->types & returned_data_t::SOIL_MOISTURE) != 0)
        {
            ss << "Soil Moisture : " << this->soil_m << " @ " << this->soil_m_ts << '\n';
        }
        if ((this->types & returned_data_t::TEMPERATURE) != 0)
        {
            ss << "Temperature : " << this->temp << " @ " << this->temp_ts << '\n';
        }
        if ((this->types & returned_data_t::LIGHT) != 0)
        {
            ss << "Light intensity : " << this->light << " @ " << this->light_ts << '\n';
        }
        return ss.str();
    }
};

namespace redis_suffixes
{
    const std::string aggr_suffix = "_comp";
    const std::string temp_suffix = "_temp";
    const std::string soil_suffix = "_sm";
    const std::string light_suffix = "_light";
} // namespace redis_suffixes

void create_aggregation_for_device(redisContext * c, std::string key, returned_data_t data);
void create_sensor_series(redisContext * c, std::string key, returned_data_t data);

int tsdb_extraction_routine(redis_interface_config conf, std::condition_variable * cvar, threaded_queue<device_info> * dev_queue);
bool redis_create_object(redisContext *c, std::string name);
void push_to_key(redisContext * c, std::string key, std::string value);
void push_double_to_key(redisContext * c, std::string key, double value);
void get_all_from_key (redisContext *c, std::string key);
void last_minute_from_key(redisContext* c, std::string key);
data_packet retrieve_last_period(redisContext * c, std::string key, returned_data_t type, ieee_addr_t addr);

#endif //REDIS_INTEFACE_HPP