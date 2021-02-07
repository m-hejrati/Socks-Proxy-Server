#include "LogVariables.h"

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <string>
#include <regex>

#ifndef CONF_H
#define CONF_H


// this class read config file and filter 
class ConfigReader {

private:

    std::vector<std::string> filIps;
    std::vector<std::string> filPorts;
    std::vector<std::string> filProtocol;
    std::vector< std::pair<std::string, std::string> > filIPPort;

public:

    // read config file and store filter ip and port
    void readConf(std::string address);

    // check ip and port and domain constraint
    int checkFilter(std::string remote_host_, std::string remote_port_, std::vector<std::string> domains);

	// check if domain name is in log domain list
	std::string checkLog(std::vector<std::string> domains);
};


#endif