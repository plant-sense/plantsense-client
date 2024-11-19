
.PHONY:
	all

SRC_FILES = main.cpp

BOOST_DIR = ../boost_1_86_0/boost

ASYNC_MQTT5_DIR = ../async-mqtt5/include

REDIS_DIR = ../hiredis-master

REDIS_STATIC_LIB = libhiredis.a

all:
	g++ -std=c++20 -o prog -pthread -fcoroutines -I$(REDIS_DIR) -I$(BOOST_DIR) -I$(ASYNC_MQTT5_DIR) -lhiredis $(SRC_FILES)