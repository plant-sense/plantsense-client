#ifndef COMMON_HPP
#define COMMON_HPP
#include <syslog.h>
#include <iostream>
#include <csignal>

#include <condition_variable>
#include <mutex>
#include <queue>
#include <sstream>

#ifdef MONIT_DEBUG
#warning "debug ON"

#define debug(X) X
#define debug_print(X) std::cout << X << '\n';

#else
#define debug(X)
#define debug_print(X)
#endif

typedef unsigned long long ieee_addr_t;

struct mqtt_client_config
{
    std::string address;
    size_t port;
    size_t timeout_s;
    size_t retries;
};

enum mqtt_QOS {
    AT_MOST_ONCE = 0,
    AT_LEAT_ONCE = 1,
    EXACTLY_ONCE = 2
};

enum returned_data_t {
    NONE = 0,
    TEMPERATURE = 1 << 0,
    HUMIDITY = 1 << 1, 
    LIGHT = 1 << 2
};

inline returned_data_t operator|(returned_data_t a, returned_data_t b){
    return static_cast<returned_data_t>( static_cast<int>(a) | static_cast<int>(b) );
}

inline returned_data_t& operator|=(returned_data_t & a, const returned_data_t& oth){
    a = a | oth;
    return a;
}

enum class interaction_type_t {
    NONE = 0,
    LIGHT = 1
};
 

template <class T>
class threaded_queue
{
private:
    std::queue<T> int_data;
    mutable std::mutex m;
    std::condition_variable c;
public:
    void push_back (T val);
    T pop_front(void);
    size_t size(void) const;
    bool empty(void) const;

    inline threaded_queue(void) : int_data(), m(), c() {}
    inline ~threaded_queue() {}
};


template <class T>
void threaded_queue<T>::push_back(T val)
{
    std::unique_lock<std::mutex> l(this->m);
    this->int_data.push(val);
    return;
}

template <class T>
T threaded_queue<T>::pop_front(void)
{
    std::unique_lock<std::mutex> l(this->m);
    T v = this->int_data.front();
    this->int_data.pop();
    return v;
}

template <class T>
size_t threaded_queue<T>::size(void) const
{
    std::unique_lock<std::mutex> l(this->m);
    return this->int_data.size();
}

template <class T>
bool threaded_queue<T>::empty(void) const
{
    std::unique_lock<std::mutex> l(this->m);
    return this->int_data.empty();
}

inline std::string ieee_to_hex(const ieee_addr_t addr) {
    std::stringstream ss;
    ss << std::hex << addr;
    return ss.str();
}
class device_info {
    private:
    const ieee_addr_t ieee_addr; // These are unique globally per device, so we should use these as db keys
    std::string friendly_name;
    returned_data_t data_types;
    interaction_type_t interactions;
    
    public:
    static const std::string base_addr;
    device_info(ieee_addr_t ieee_addr, std::string friendly_name, returned_data_t returned_data, interaction_type_t interaction_type) 
        : ieee_addr(ieee_addr), friendly_name(friendly_name), data_types(returned_data), interactions(interaction_type) {};

    inline bool set_friendly_name(std::string new_name) {
        if (new_name.size() == 0){
            
            return false;
        }
        this->friendly_name = new_name;
        return true;
    }

    inline void unset_friendly_name(){
        this->friendly_name = "";
    }

    inline std::string getName() const { 
        if (this->friendly_name.size() == 0){
            return "0x" + ieee_to_hex(this->ieee_addr);
        }
        return this->friendly_name; 
    }

    inline std::string get_z2m_addr() const {
        return "zigbee2mqtt/" + this->getName();
    }

    inline ieee_addr_t get_ieee_address(void) const {
        return this->ieee_addr;
    }

    inline returned_data_t get_returned_data(void) const {
        return this->data_types;
    }

    inline interaction_type_t get_interaction_types(void) const {
        return this->interactions;
    }

};


#endif // COMMON_HPP