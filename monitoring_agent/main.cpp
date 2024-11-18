#include <iostream>

#include <boost/asio/io_context.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/ip/tcp.hpp>

#include <async_mqtt5.hpp>

int main(int argc, char const *argv[])
{
    boost::asio::io_context ioc;

    async_mqtt5::mqtt_client<boost::asio::ip::tcp::socket> c(ioc);

    c.brokers("localhost", 1883).async_run(boost::asio::detached);
    
    c.async_publish<async_mqtt5::qos_e::at_most_once>("test_topic", "Hello World!", async_mqtt5::retain_e::no, async_mqtt5::publish_props {},
        [&c](async_mqtt5::error_code ec){
            std::cout << ec.message() << std::endl;
        }
    );

    ioc.run();


    return 0;
}
