#include "Logger.h"
#include "ConfigReader.h"
#include "Session.h"
#include "LogVariables.h"

#include <string>
#include <boost/asio.hpp>

using boost::asio::ip::tcp;

#ifndef SERVER_H
#define SERVER_H

class Server {

public:

	// constructor of server class
	Server(boost::asio::io_service& io_service, short port, unsigned buffer_size, std::string logType);

	void start();
	
	// print (log) the amount of logVariables
	void printLogVars (vector<int> preActive);

	// log every minute
	void logEveryMinute();

	// reset all the log variables
	void resetVariables();

	// get data and convert to its appropriate unit
	std::string unitConversion(int data);
	
private:

	//asynchronously accept new connections and create session
	void do_accept(ConfigReader configReader);

	tcp::acceptor acceptor_;
	tcp::socket in_socket_;
	size_t buffer_size_;
	unsigned session_id_0;
	Logger logger2;
};

#endif
