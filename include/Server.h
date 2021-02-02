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

	// print (log) the amount of logVariables
	void printLogVars ();

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
