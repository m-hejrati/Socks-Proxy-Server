#include <string>
#include <boost/asio.hpp>

using boost::asio::ip::tcp;

#ifndef SERVER_H
#define SERVER_H

class Server {

public:

	Server(boost::asio::io_service& io_service, short port, unsigned buffer_size);

private:

	void do_accept();

	tcp::acceptor acceptor_;
	tcp::socket in_socket_;
	size_t buffer_size_;
	unsigned session_id_;
};

#endif