// include dependent class
#include "Proxy/Server.h"
#include "Logger/Logger.h"

#include <iostream> 
#include <boost/asio.hpp>
#include <sstream> // for ostringstream used in logger


int main(){

	// set default log level to info, until reading config file
	Logger logger("info");

	// create a buffer to make log with it using ostringstream
	std::ostringstream tmp;  

    int port = 1080;
    size_t buffer_size = 8192;

	try {

		boost::asio::io_service io_service;
		Server server(io_service, port, buffer_size);
		io_service.run();
	
    } catch (std::exception& e) {

  		tmp << "exception: " << e.what();
	    logger.log(tmp.str(), "error");

	}

	return 0;
}