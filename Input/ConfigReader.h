#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>


#ifndef CONF_H
#define CONF_H


// this class read config file and filter 
class ConfigReader {

private:

    std::vector<std::string> filIps;
    std::vector<std::string> filPorts;

public:

    // read config file and store filter ip and port
    void readConf(std::string address);

    // check ip and port constraint
    bool checkFilter(std::string remote_host_, std::string remote_port_);

};


#endif