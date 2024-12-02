

.PHONY: all

all: main.o mqtt.o
	g++ -std=c++20 -o z2m_spoof -lpaho-mqttpp3 -lpaho-mqtt3as -lformat $^

main.o: main.cpp
	g++ -std=c++20 -o main.o -c main.cpp

mqtt.o: mqtt.cpp
	g++ -std=c++20 -o mqtt.o -c mqtt.cpp