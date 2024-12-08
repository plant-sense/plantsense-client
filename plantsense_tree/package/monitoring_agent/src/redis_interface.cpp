#include "redis_interface.hpp"
#include <iostream>

int tsdb_extraction_routine(redis_interface_config conf, std::condition_variable * cvar, threaded_queue<device_info> * dev_queue) {
    
    redisReply *reply;
    redisContext *context;

    context = redisConnect("localhost", 6379);

    if (context->err)
    {
        std::cout << "R :: database connection error: " << context->errstr << "\n";
        cvar->notify_all();
        return EXIT_FAILURE;
    }
    
    reply = (redisReply*)redisCommand(context, "PING %s", "Hello World!");
    if (reply == NULL)
    {
        std::cout << "R :: failed to ping redis server\n";
        cvar->notify_all();
        return EXIT_FAILURE;
    }
    
    std::cout << "R :: resp: " << reply->str << '\n';

    freeReplyObject(reply);

    redis_create_object(context, "sensor_3");

    push_to_key(context, "sensor_3", "1");
    push_to_key(context, "sensor_3", "2");
    push_to_key(context, "sensor_3", "3");
    push_to_key(context, "sensor_3", "4");
    push_to_key(context, "sensor_3", "5");
    push_to_key(context, "sensor_3", "6");

    get_all_from_key(context, "sensor_3");

    last_minute_from_key(context, "sensor_3");

    redisFree(context);

    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        if (dev_queue->empty())
        {
            continue;
        }
        while (!dev_queue->empty())
        {
            device_info dev = dev_queue->pop_front();
            std::cout << "Got from other thread::\n";
            std::cout << dev.get_z2m_addr() << " :: N? = " << dev.getName() << " :: R = " << dev.get_returned_data() << "\n";

        }
        
    }
    cvar->notify_all();
    
    
    return EXIT_SUCCESS;
}

bool redis_create_object(redisContext *c, std::string name) {
    redisReply * reply;

    reply = (redisReply*)redisCommand(c, "TS.CREATE %s RETENTION 2678400000 LABELS l1 %s", name.c_str(), "east");
    if (reply == NULL ) {
        std::cout << "R :: failed to create object\n";
        return false;
    }

    std::cout << "R :: type: " << std::string(reply->str) << "\n";
    return true;
}

void push_to_key(redisContext * c, std::string key, std::string value){
    redisReply * reply;

    reply = (redisReply*)redisCommand(c, "TS.ADD %s * %s", key.c_str(), value.c_str());

    freeReplyObject(reply);
}

void get_all_from_key (redisContext *c, std::string key){
    redisReply* reply;

    reply = (redisReply*)redisCommand(c, "TS.RANGE %s - +", key.c_str());

    std::cout << reply->elements << '\n';

    for (int i = 0; i < reply->elements; i++)
    {
        for (int j = 0; j < reply->element[i]->elements; j++)
        {
            std::cout << reply->element[i]->element[j]->integer << " ";
        }
        std::cout << '\n';
    }

    freeReplyObject(reply);
    
}

void create_aggregation_rule(redisContext* c, std::string key, std::string aggr_name){
    redisReply * reply;


}

void last_minute_from_key(redisContext* c, std::string key) {
    redisReply* reply;

    reply = (redisReply*)redisCommand(c, "TS.RANGE %s - + + AGGREGATION avg 60000", key.c_str());

    if (!reply)
    {
        std::cout << "Failed to get data from Redis\n";
        return;
    }

    std::cout << "Got last minute:: Time: " << reply->element[reply->elements-1]->element[0]->integer << " Value: " << reply->element[reply->elements-1]->element[1]->str << '\n';
    freeReplyObject(reply);
}