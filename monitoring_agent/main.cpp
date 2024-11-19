#include <iostream>
#include <sys/wait.h>
///// ===>>> !!! this is set explicitly to override the fact that vscode cannot comprehend that it is set by boost itself
// this should be removed on prod
#define BOOST_ASIO_HAS_CO_AWAIT 1

#include <boost/asio/as_tuple.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/signal_set.hpp>
#include <boost/asio/use_awaitable.hpp>

#include <boost/asio/ip/tcp.hpp>

#include <async_mqtt5.hpp>

#include <hiredis/hiredis.h>

constexpr auto use_nothrow_awaitable = boost::asio::as_tuple(boost::asio::use_awaitable);

using mqtt_client_t = async_mqtt5::mqtt_client<boost::asio::ip::tcp::socket>;

struct mqtt_clinet_config
{
    std::string host;
    int port;
};


struct mqtt_message {
    std::string topic;
    std::string payload;
};

boost::asio::awaitable<bool> subscribe(mqtt_client_t &client, std::vector<std::string> topics) {
    const async_mqtt5::subscribe_options sub_opts = async_mqtt5::subscribe_options{
            async_mqtt5::qos_e::exactly_once,
            async_mqtt5::no_local_e::no,
            async_mqtt5::retain_as_published_e::retain,
            async_mqtt5::retain_handling_e::send
    };

    std::vector<async_mqtt5::subscribe_topic> topic_structs;

    for (long long i = 0; i < topics.size(); i++)
    {
        topic_structs.push_back(
            async_mqtt5::subscribe_topic {
                topics[i],
                sub_opts
            }
        );
    }

	auto&& [ec, sub_codes, sub_props] = co_await client.async_subscribe(
		topic_structs, async_mqtt5::subscribe_props {} , use_nothrow_awaitable
	);

    if (ec)
		std::cout << "Subscribe error occurred: " << ec.message() << std::endl;
	else
		std::cout << "Result of subscribe request: " << sub_codes[0].message() << std::endl;

	co_return !ec && !sub_codes[0]; // True if the subscription was successfully established.
    
}

boost::asio::awaitable<void> subscribe_and_observe(mqtt_client_t & c, std::vector<std::string> topics) {
    boost::asio::io_context ioc;

    c.brokers("localhost", 1883).async_run(boost::asio::detached);

    // c.async_publish<async_mqtt5::qos_e::at_most_once>("test_topic", "Hello World!", async_mqtt5::retain_e::no, async_mqtt5::publish_props {},
    //     [&c](async_mqtt5::error_code ec){
    //         std::cout << ec.message() << std::endl;
    //         c.async_disconnect(boost::asio::detached);
    //     }
    // );

    if ( !co_await subscribe(c, topics)) {
        co_return;
    }


    while (true)
    {
        auto && [ec, topic, payload, publish_groups] = co_await c.async_receive(use_nothrow_awaitable);

        if (ec == async_mqtt5::client::error::session_expired) {
            
            if ( co_await subscribe(c, topics )) {
                continue;
            } else { //resubscription failed
                break;
            }
        } else if (ec) {
            break;
        }
        
        std::cout << "Topic: " << topic << '\n' << "Message: " << payload << '\n';

    }

    co_return;
}


[[noreturn]] void mqtt_client_routine() {

    boost::asio::io_context ioc;

    mqtt_client_t client (ioc);

    boost::asio::signal_set signals(ioc, SIGINT, SIGTERM);

    signals.async_wait([&client](async_mqtt5::error_code ec, int signal){
        client.async_disconnect(async_mqtt5::asio::detached);
    });

    std::vector<std::string> topics = {"test1", "test2"};

    co_spawn(ioc, subscribe_and_observe(client, topics ), boost::asio::detached);
    
    ioc.run();

    exit(0);
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

void push_to_db(redisContext * c, std::string key, std::string value){
    redisReply * reply;

    reply = (redisReply*)redisCommand(c, "TS.ADD %s * %s", key.c_str(), value.c_str());

    freeReplyObject(reply);
}

void get_from_db (redisContext *c, std::string key){
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

[[noreturn]] void tsdb_extraction_routine() {
    
    redisReply *reply;
    redisContext *context;

    context = redisConnect("localhost", 6379);

    if (context->err)
    {
        std::cout << "R :: database connection error: " << context->errstr << "\n";
        exit (-1);
    }
    
    reply = (redisReply*)redisCommand(context, "PING %s", "Hello World!");
    if (reply == NULL)
    {
        std::cout << "R :: failed to ping redis server\n";
        exit(-1);
    }
    
    std::cout << "R :: resp: " << reply->str << '\n';

    freeReplyObject(reply);

    redis_create_object(context, "sensor_3");

    push_to_db(context, "sensor_3", "1233221");
    push_to_db(context, "sensor_3", "1233221");
    push_to_db(context, "sensor_3", "1233221");
    push_to_db(context, "sensor_3", "1233221");
    push_to_db(context, "sensor_3", "1233221");
    push_to_db(context, "sensor_3", "1233221");

    get_from_db(context, "sensor_3");

    redisFree(context);

    exit(0);    
}

int main(int argc, char const *argv[])
{
    // load config


    // verify mqtt broker is up

    boost::asio::io_context test_cont;



    pid_t child_pid_mqtt, child_pid_db, wait_pid;
    int wait_result = 0;

    if( (child_pid_mqtt = fork()) == 0 ) {
        mqtt_client_routine();
    }

    if ((child_pid_db = fork()) == 0) {
        tsdb_extraction_routine();
    }

    while ( ( wait_pid = wait(&wait_result)) > 0) {
        std::cout << "child " << wait_pid ;

        if (wait_pid == child_pid_mqtt)
        {
            std::cout << " (mqtt client)";
        }

        std::cout << " died with result " << wait_result << "\n";

        if (wait_result != 0) {
            std::cout << "exitted with error -- terminating program\n";
            kill(child_pid_mqtt, SIGQUIT);
            kill(child_pid_db, SIGQUIT);
        }

    }

    std::cout << "Program Terminated\n";
    exit(EXIT_SUCCESS);
}