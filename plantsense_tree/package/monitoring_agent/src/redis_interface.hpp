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


int tsdb_extraction_routine(redis_interface_config conf, std::condition_variable * cvar, threaded_queue<device_info> * dev_queue);
bool redis_create_object(redisContext *c, std::string name);
void push_to_key(redisContext * c, std::string key, std::string value);
void get_all_from_key (redisContext *c, std::string key);
void last_minute_from_key(redisContext* c, std::string key);

#endif //REDIS_INTEFACE_HPP