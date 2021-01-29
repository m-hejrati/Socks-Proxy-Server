#include "../include/ConfigReader.h"

#include <string>


std::vector<std::string> filIps;
std::vector<std::string> filPorts;


// read config file and store filter ip and port
void ConfigReader::readConf(std::string address){

    // Create a root
    boost::property_tree::ptree root;

    // Load the json file in this ptree
    boost::property_tree::read_json(address, root);

    for (boost::property_tree::ptree::value_type &ip : root.get_child("filteredIP")){

        // ip.first contain the string ""
        filIps.push_back(ip.second.data());
    }

    for (boost::property_tree::ptree::value_type &port : root.get_child("filteredPort")){

        filPorts.push_back(port.second.data());
    }

}


// check ip and port constraint
bool ConfigReader::checkFilter(std::string remote_host_, std::string remote_port_){

    for (std::string& ip : filIps)
        if (ip.compare(remote_host_))
            return 1;

    for (std::string& port : filPorts)
        if (port.compare(remote_port_))
            return 1;

    return 0;
}

