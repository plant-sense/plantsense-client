
.PHONY:
	all

SRC_FILES = mqtt_client.cpp main.cpp 

REDIS_DIR = ../hiredis-master

all: mqtt_client.o redis_interface.o main.o
	g++ -std=c++20 -o monitoring_agent -lpaho-mqttpp3 -lpaho-mqtt3as -I$(REDIS_DIR) -lhiredis main.o mqtt_client.o redis_interface.o

main.o: main.cpp
	g++ -std=c++20 -o main.o -c main.cpp

mqtt_client.o: mqtt_client.cpp
	g++ -std=c++20 -o mqtt_client.o -c mqtt_client.cpp

redis_interface.o: redis_interface.cpp
	g++ -std=c++20 -o redis_interface.o -c redis_interface.cpp