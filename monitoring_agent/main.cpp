#include <iostream>
#include <sys/wait.h>

#include "mqtt_client.hpp"
#include "redis_interface.hpp"

int main(int argc, char const *argv[])
{

    pid_t child_pid_mqtt, child_pid_db, wait_pid;
    int wait_result = 0;

    const mqtt_client_config m_conf {
        "localhost",
        1883,
        120
    };

    if( (child_pid_mqtt = fork()) == 0 ) {
        mqtt_listener_subroutine(m_conf);
    }

    if ((child_pid_db = fork()) == 0) {
        tsdb_extraction_routine();
    }

    while ( ( wait_pid = wait(&wait_result)) > 0) {
        std::cout << "child " << wait_pid ;

        if (wait_pid == child_pid_mqtt){
            std::cout << " (mqtt client)";
        }
        if (wait_pid == child_pid_db){
            std::cout << " (db subroutine)";
        }

        std::cout << " died with result " << wait_result << "\n";

        if (wait_result != 0) {
            std::cout << "exitted with error -- terminating program\n";
            kill(child_pid_mqtt, SIGKILL);
            kill(child_pid_db, SIGKILL);
        }
    }

    std::cout << "Program Terminated\n";
    exit(EXIT_SUCCESS);
}