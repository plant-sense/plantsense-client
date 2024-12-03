# monitoring agent

.DEFAULT_GOAL := rebuild

CXX = g++

REDIS_DIR = ../../../../hiredis-master

# CXXFLAGS = -Wall -Werror -std=c++20
CXXFLAGS = -Wall -std=c++20 -D MONIT_DEBUG
CXXLIBS = -lpaho-mqttpp3 -lpaho-mqtt3as -I$(REDIS_DIR) -lhiredis -ljsoncpp

SRC = ${wildcard *.cpp}
OBJ = ${patsubst %.cpp,build/%.o,${SRC}}

.PHONY:
	all clean rebuild

all: ${OBJ}
	$(CXX) ${CXXFLAGS} -o monitoring_agent ${CXXLIBS} $^

build/%.o: %.cpp
	$(CXX) ${CXXFLAGS} -o $@ -c $<

clean:
	@rm build/* 2>/dev/null || echo "Nothing to clean"

rebuild:
	$(MAKE) clean
	$(MAKE) all