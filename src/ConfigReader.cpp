#include "ConfigReader.h"


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

    for (boost::property_tree::ptree::value_type &domain : root.get_child("logDomain")){

        domainSession.insert(make_pair(domain.second.data(), 0));
        domainTraffic.insert(make_pair(domain.second.data(), 0));
    }
}


// check ip and port and domain constraint
int ConfigReader::checkFilter(std::string remote_host_, std::string remote_port_, std::vector<std::string> domains){

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

	// check filtered domains
    for (std::string& domain1 : filDomains)
		for (std::string& domain2 : domains)
			if (std::regex_match (domain2, std::regex(domain1)))
				return 5;

    return 0;
}


// check if domain name is in log domain list
std::string ConfigReader::checkLog( std::vector<std::string> domains){

    std::map<std::string, int>::iterator it;
    for(it = domainSession.begin(); it != domainSession.end(); ++it)
		for (std::string& domain : domains)
			if (std::regex_match (domain, std::regex(it->first)))
                return it->first;
            
    return "NO";
}
