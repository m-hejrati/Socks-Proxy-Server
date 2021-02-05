#include "ConfigReader.h"

#include <string>


std::vector<std::string> filIps;
std::vector<std::string> filPorts;
std::vector<std::string> filProtocols;
std::vector< std::pair<std::string, std::string> > filIPPort;
std::vector<std::string> filDomains;


// read config file and store filter ip and port
void ConfigReader::readConf(std::string address){

    // Create a root
    boost::property_tree::ptree root;

    // Load the json file in this ptree
    boost::property_tree::read_json(address, root);

    for (boost::property_tree::ptree::value_type &ip : root.get_child("IP")){

        // ip.first contain the string ""
        filIps.push_back(ip.second.data());
    }

    for (boost::property_tree::ptree::value_type &port : root.get_child("Port")){

        filPorts.push_back(port.second.data());
    }

    for (boost::property_tree::ptree::value_type &protocol : root.get_child("Protocol")){

        if (protocol.second.data().compare("http") == 0)
            filProtocols.push_back("80");
        else if (protocol.second.data().compare("https") == 0)
            filProtocols.push_back("443");
    }

    for (boost::property_tree::ptree::value_type &ipport : root.get_child("IPPort")) {

        std::string ip = ipport.first;
        std::string port = ipport.second.data();
        filIPPort.push_back(std::make_pair(ip, port));
    }

    for (boost::property_tree::ptree::value_type &domain : root.get_child("filterDomain")){

        filDomains.push_back(domain.second.data());
    }
}


// check ip and port and domain constraint
int ConfigReader::checkFilter(std::string remote_host_, std::string remote_port_, std::string r_domain_){

    for (std::string& ip : filIps)
        if (ip.compare(remote_host_) == 0)
            return 1;

    for (std::string& port : filPorts)
        if (port.compare(remote_port_) == 0)
            return 2;

    for (std::string& protocol : filProtocols)
        if (protocol.compare(remote_port_) == 0)
            return 3;

    for (std::pair<std::string, std::string>& ipport : filIPPort)
        if ((ipport.first.compare(remote_host_) == 0) && (ipport.second.compare(remote_port_) == 0))
            return 4;

    for (std::string& domain : filDomains)
        if (domain.find(r_domain_) == std::string::npos)
            return 5;

    return 0;
}

