#include "Logger.h"
#include "ConfigReader.h"

#include <boost/asio.hpp>
#include <iostream>
#include <mutex>

using boost::asio::ip::tcp;

#ifndef SESSION_H
#define SESSION_H


class Session : public std::enable_shared_from_this<Session> {

public:

	// constructor of session class
    Session(tcp::socket in_socket, unsigned session_id, size_t buffer_size, std::string logType, ConfigReader configReader_);

	void start();


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

	// this function get messages;
	// direction 1: from client with in_socket
	// direction 2: from server with out_socket
	void do_read(int direction);

	// this function send messages;
	// direction 1: send in_buf message from out_socket to server 
	// direction 2: from out_buf message in_socket to client
	void do_write(int direction, std::size_t Length);

	// find the session in active list and remove it and also increase finish sessoin number
	void logFinishSession();

	// get IP and store domain name
	void storeDomainName();

	tcp::socket in_socket_; // a socket in client side
	tcp::socket out_socket_; // a socket in server side
	tcp::resolver resolver;
	std::string remote_host_;
	std::string remote_port_;
	std::vector<std::string> domains;
	std::vector<char> in_buf_; // a buffer to store client messages
	std::vector<char> out_buf_; // a buffer to store server messages
	int session_id_;
    Logger logger3;
	ConfigReader configReader;
	bool isFilter;
	std::string isLog;
	mutex mtx;

};


#endif
