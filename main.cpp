// include dependent class
#include "Proxy/Server.h"

#include <iostream> 
#include <boost/asio.hpp>

int main(){

    int port = 1080;
    size_t buffer_size = 8192;

	try {

		boost::asio::io_service io_service;
		Server server(io_service, port, buffer_size);
		io_service.run();
	
    } catch (std::exception& e) {

        std::cout << "exception:" << e.what() << "\n";
	}

	return 0;
}