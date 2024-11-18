
.PHONY:
	all

SRC_FILES = main.cpp

BOOST_DIR = ../boost_1_86_0/boost

ASYNC_MQTT5_DIR = ../async-mqtt5/include

all:
	g++ -std=c++20 $(SRC_FILES) -o prog -pthread -fcoroutines -I$(BOOST_DIR) -I$(ASYNC_MQTT5_DIR)