#include "Server.h"
#include "Logger.h"

#include <iostream> 
#include <boost/asio.hpp>
#include <sstream> // for ostringstream used in logger
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

int main(){

	// Create a root
	boost::property_tree::ptree root;
	// Load the json file in this ptree
	boost::property_tree::read_json("../config.json", root);
	std::string type = root.get<std::string> ("LogLevel");
	
	Logger logger1(type);

    logger1.log("    \"Proxy Server\"", "info");
    logger1.log("    \"Mahdi Hejrati\"", "info");

	// create a buffer to make log with it using ostringstream
	std::ostringstream tmp;  

	int port = 1080;
	size_t buffer_size = 8192;

	try {

		boost::asio::io_service io_service;
		Server server(io_service, port, buffer_size, logger1.getConfigType());
		server.start();
		io_service.run();
	
	} catch (std::exception& e) {

		tmp << "exception: " << e.what();
		logger1.log(tmp.str(), "error");

	}

	return 0;
}
