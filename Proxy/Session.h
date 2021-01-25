#include "../Logger/Logger.h"

#include <boost/asio.hpp>
#include <iostream>

using boost::asio::ip::tcp;

#ifndef SESSION_H
#define SESSION_H


class Session : public std::enable_shared_from_this<Session> {

public:

// constructor of session class
    Session(tcp::socket in_socket, unsigned session_id, size_t buffer_size, std::string logType);

private:

	// proxy reads handshake that comes from client
	// it contains methods that client supports.
	void read_socks5_handshake();
	
	// proxy write handshake to client.
	// it contains choose method by proxy.
	void write_socks5_handshake();

	tcp::socket in_socket_;

	std::vector<char> in_buf_;
	std::vector<char> out_buf_;
	int session_id_;
    Logger logger3;

};


#endif