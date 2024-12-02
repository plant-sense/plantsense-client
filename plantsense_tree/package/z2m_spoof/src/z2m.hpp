#ifndef Z2M_HPP
#define Z2M_HPP

#include <string>
#include <vector>
#include <format>
#include <algorithm>

typedef unsigned long long ieee_name_t;
typedef unsigned long long ull;

enum zigbee_device_type {
    coordinator = 0,
    router = 1
};

enum expose_type{
    binary,
    numeric,
    text,
};

std::string ullToHex(ull num) {
    ull rem;
    std::string res;

    while (num != 0)
    {
        rem = num % 16;
        char c;
        if (rem >= 10)
        {
            c = rem + 55;
        }
        else {
            c = rem + 48;
        }
        res += c;
        num /= 16;
    }
    std::reverse(res.begin(), res.end());
    return res;
}

class zigbee_expose {
    protected:
    expose_type type;
    std::string name;
    std::string label;
    std::string property;

    zigbee_expose(expose_type type, std::string name, std::string label, std::string property){
        this->type = type;
        this->name = name;
        this->label = label;
        this->property = property;
    }
    public:

    virtual std::string getExposedJSON() = 0;
    virtual std::string getValueJSON() = 0;
};

class zigbee_expose_temp : public zigbee_expose {
    private:

    ull value;

    public:

    zigbee_expose_temp(std::string unit, ull init_val) : zigbee_expose(expose_type::numeric, "temperature", "Temperature", "temperature") {

    }

};

class z2m_device
{
private:
    ieee_name_t ieee_name;
    zigbee_device_type type;
    std::string friendly_name;
    std::string description;
    //definition
    std::string model;
    std::string vendor;
    std::string desc;
    //std::vector<std::string> options;
    std::vector<std::string> exposes;
public:
    z2m_device(/* args */);
    ~z2m_device();
    std::string getDeviceJSON() {
        std::string val;
        val += "{";
        val += "\"ieee_address\":\"0x" + ullToHex(this->ieee_name) + "\",";
        val += "\"type\":\"Router\",";
        val += "\"friendly_name\":\"" + this->friendly_name + "\",";
        val += "\"description\":\"" + this->description + "\",";
        val += "\"definition\":{\"model\":\"FAKE\",\"vendor\":\"FAKE\",\"description\":\"Dummy device\",";
        val += "\"options\":[],";
        val += "\"exposrs\":" + this->getExposedJSON() + ",";
        val += "}";
        val += "}";
    }
    virtual std::string getExposedJSON() = 0;
    virtual std::string getValueJSON() = 0;

};

z2m_device::z2m_device(/* args */)
{

}

z2m_device::~z2m_device()
{
}


std::string assemble_device_message();

#endif //Z2M_HPP