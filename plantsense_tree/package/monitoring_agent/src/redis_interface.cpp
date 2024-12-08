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

    std::vector<device_info> devs;

    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(5));
        while (!dev_queue->empty())
        {
            device_info dev = dev_queue->pop_front();
            devs.push_back(dev);
            std::cout << "Got from other thread::\n";
            std::cout << dev.get_z2m_addr() << " :: N? = " << dev.getName() << " :: R = " << dev.get_returned_data() << "\n";
        }
        debug(        
        if (devs.size() == 0){
            std::cout << "No devices\n";
        }
        );
        for (size_t i = 0; i < devs.size(); i++)
        {
            data_packet d = retrieve_last_period(context, ieee_to_hex(devs[i].get_ieee_address()), devs[i].get_returned_data(), devs[i].get_ieee_address());
            debug_print(d.gist());
        }
        

    }
    cvar->notify_all();
    
    redisFree(context);
    
    return EXIT_SUCCESS;
}

data_packet retrieve_last_period(redisContext * c, std::string key, returned_data_t type, ieee_addr_t addr){
    redisReply * reply;

    data_packet d;
    d.types = type;
    d.addr = addr;
    d.temp_ts = 0;
    d.temp = 0;
    d.soil_m_ts = 0;
    d.soil_m = 0;
    d.light_ts = 0;
    d.light = 0;
    

    if ((type & returned_data_t::SOIL_MOISTURE) != 0){

        debug_print((key + redis_suffixes::soil_suffix + redis_suffixes::aggr_suffix).c_str() << '\n');

        reply = (redisReply*)redisCommand(c, "TS.GET %s LATEST", (key + redis_suffixes::soil_suffix + redis_suffixes::aggr_suffix).c_str());
        
        if (reply->elements == 0){
            d.types = (returned_data_t) (d.types & (~returned_data_t::SOIL_MOISTURE));
        } else {

            d.soil_m_ts = reply->element[0]->integer;
            
            (void)sscanf(reply->element[1]->str, "%lf", &d.soil_m);

            debug_print("Soil Moist - ts: " << d.soil_m_ts << " v: " << d.soil_m << '\n');

        }
        freeReplyObject(reply);
    }

    if ((type & returned_data_t::TEMPERATURE) != 0){
        reply = (redisReply*)redisCommand(c, "TS.GET %s LATEST", (key + redis_suffixes::temp_suffix + redis_suffixes::aggr_suffix).c_str());
        
        if (reply->elements == 0){
            d.types = (returned_data_t) (d.types & (~returned_data_t::TEMPERATURE));
        }
        else {
            d.temp_ts = reply->element[0]->integer;
            (void)sscanf(reply->element[1]->str, "%lf", &d.temp);

            debug_print("Temp - ts: " << d.temp_ts << " v: " << d.temp << '\n');

        }
        freeReplyObject(reply);
    }

    if ((type & returned_data_t::LIGHT) != 0){
        reply = (redisReply*)redisCommand(c, "TS.GET %s LATEST", (key + redis_suffixes::light_suffix + redis_suffixes::aggr_suffix).c_str());
        
        if (reply->elements == 0){
            d.types = (returned_data_t) (d.types & (~returned_data_t::LIGHT));
        }
        else {
            d.light_ts = reply->element[0]->integer;
            
            (void)sscanf(reply->element[1]->str, "%lf", &d.light);
            
            debug_print("Light - ts: " << d.light_ts << " v: " << d.light << '\n');

        }
        freeReplyObject(reply);
    }

    return d;

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

void push_double_to_key(redisContext * c, std::string key, double value){
    redisReply * reply;

    reply = (redisReply*)redisCommand(c, "TS.ADD %s * %f", key.c_str(), value);

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

void create_sensor_series(redisContext * c, std::string key, returned_data_t data){
    if ((data & returned_data_t::SOIL_MOISTURE) != 0)
    {
        redis_create_object(c, key + redis_suffixes::soil_suffix);
    }
    if ((data & returned_data_t::TEMPERATURE) != 0)
    {
        redis_create_object(c, key + redis_suffixes::temp_suffix);
    }
    if ((data & returned_data_t::LIGHT) != 0)
    {
        redis_create_object(c, key + redis_suffixes::light_suffix);
    }   
}

void create_aggregation_for_device(redisContext * c, std::string key, returned_data_t data){
    redisReply* reply;

    if ((data & returned_data_t::SOIL_MOISTURE) != 0)
    {
        reply = (redisReply*)redisCommand(c, "TS.CREATE %s", (key + redis_suffixes::soil_suffix + redis_suffixes::aggr_suffix).c_str() );
        debug_print(reply->str);
        freeReplyObject(reply);
        reply = (redisReply*)redisCommand(c, "TS.CREATERULE %s %s AGGREGATION avg 1000", (key + redis_suffixes::soil_suffix).c_str(), (key+redis_suffixes::soil_suffix + redis_suffixes::aggr_suffix).c_str() );
        debug_print(reply->str);
        freeReplyObject(reply); 
    }
    if ((data & returned_data_t::TEMPERATURE) != 0)
    {
        reply = (redisReply*)redisCommand(c, "TS.CREATE %s", (key + redis_suffixes::temp_suffix + redis_suffixes::aggr_suffix).c_str() );
        debug_print(reply->str);
        freeReplyObject(reply);
        reply = (redisReply*)redisCommand(c, "TS.CREATERULE %s %s AGGREGATION avg 1000", (key + redis_suffixes::temp_suffix).c_str(), (key+redis_suffixes::temp_suffix + redis_suffixes::aggr_suffix).c_str() );
        debug_print(reply->str);
        freeReplyObject(reply); 
    }
    if ((data & returned_data_t::LIGHT) != 0)
    {
        reply = (redisReply*)redisCommand(c, "TS.CREATE %s", (key + redis_suffixes::light_suffix + redis_suffixes::aggr_suffix).c_str() );
        debug_print(reply->str);
        freeReplyObject(reply);
        reply = (redisReply*)redisCommand(c, "TS.CREATERULE %s %s AGGREGATION avg 1000", (key + redis_suffixes::light_suffix).c_str(), (key+redis_suffixes::light_suffix + redis_suffixes::aggr_suffix).c_str() );
        debug_print(reply->str);
        freeReplyObject(reply); 
    }   
}

double last_second_from_key(redisContext * c, std::string key){
    redisReply * reply;

    reply = (redisReply*)redisCommand(c, "TS.RANGE %s - + + AGGREGATION abg 1000", key.c_str());

    std::string val = reply->element[reply->elements-1]->element[1]->str;


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