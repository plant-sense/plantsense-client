#include <iostream>
#include <sys/wait.h>

#include "common.hpp"
#include "mqtt_client.hpp"
#include "redis_interface.hpp"

volatile sig_atomic_t term = 0;

extern "C" {
void handleSIGTERM(int sig){
    term = true;
} 
}

int main(int argc, char const *argv[])
{
    std::signal(SIGTERM, handleSIGTERM);
    std::signal(SIGINT, handleSIGTERM);

    pid_t child_pid_mqtt, child_pid_db, wait_pid;
    int wait_result = 0;

    const mqtt_client_config m_conf {
        .address = "localhost",
        .port = 1883,
        .timeout_s = 10,
        .retries = 5
    };

    if( (child_pid_mqtt = fork()) == 0 ) {
        mqtt_listener_subroutine(m_conf);
    }

    if ((child_pid_db = fork()) == 0) {
        tsdb_extraction_routine();
    }

    while ( ( wait_pid = wait(&wait_result)) > 0 || errno == EINTR) {

        if (wait_pid == -1 && errno == EINTR && term != 0)
        {
            errno = 0;
            std::cout << "Termination request recieved : killing all children";
            kill(child_pid_db, SIGTERM);
            kill(child_pid_mqtt, SIGTERM);
            continue;
        }
        
        std::cout << "child " << wait_pid ;

        if (wait_pid == child_pid_mqtt){
            std::cout << " (mqtt client)";
        }
        if (wait_pid == child_pid_db){
            std::cout << " (db subroutine)";
        }

        std::cout << " died with result " << wait_result << "\n";

        if (wait_pid == child_pid_db)
        {
            continue;
        }
        

        if (wait_result != 0) {
            std::cout << "exitted with error -- terminating program\n";
            kill(child_pid_mqtt, SIGKILL);
            kill(child_pid_db, SIGKILL);
        }
    }

    std::cout << "Program Terminated\n";
    exit(EXIT_SUCCESS);
}