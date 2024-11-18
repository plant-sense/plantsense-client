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
            //connection lost, but client reconnected, resubscribe;
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

[[noreturn]] void tsdb_extraction_routine() {
    exit(0);    
}

int main(int argc, char const *argv[])
{
    // load config


    // verify mqtt broker is up

    boost::asio::io_context test_cont;



    pid_t child_pid_mqtt, wait_pid;
    int wait_result = 0;

    if( (child_pid_mqtt = fork()) == 0 ) {
        mqtt_client_routine();
    }

    while ( ( wait_pid = wait(&wait_result)) > 0) {
        std::cout << "child " << wait_pid ;

        if (wait_pid == child_pid_mqtt)
        {
            std::cout << " (mqtt client)";
        }
        

        std::cout << " died with result " << wait_result << "\n";
    }

    std::cout << "Program Terminated\n";
    exit(EXIT_SUCCESS);
}