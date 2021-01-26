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
	// it contains methods that client supports
	void read_socks5_handshake();
	
	// proxy write handshake to client
	// it contains choose method by proxy
	void write_socks5_handshake();

	// after initial handshake, client sends a request to proxy
	// which contains port and IP of the server that it wants to connect
	void read_socks5_request();

	// resolve entered address
	// this function get pair of host (IP or name) and port, and returns list of possible endpoints to connect to.
	void do_resolve();

	// connect to specified endpoint 
	void do_connect(tcp::resolver::iterator& it);

	// after connection stablish, proxy sends a response to client to show that it was successful
	void write_socks5_response();


	tcp::socket in_socket_;
	tcp::socket out_socket_;
	tcp::resolver resolver;

	std::string remote_host_;
	std::string remote_port_;
	std::vector<char> in_buf_;
	std::vector<char> out_buf_;
	int session_id_;
    Logger logger3;

};


#endif