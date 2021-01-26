#include "Session.h"

#include <boost/asio.hpp>

using boost::asio::ip::tcp;

// constructor of session class
Session::Session(tcp::socket in_socket, unsigned session_id, size_t buffer_size, std::string logType) :
    in_socket_(std::move(in_socket)), 
    out_socket_(in_socket.get_io_service()), 
    resolver(in_socket.get_io_service()),
    in_buf_(buffer_size), 
    out_buf_(buffer_size), 
    session_id_(session_id) {

        logger3.setConfigType(logType);

		// read handshake
		read_socks5_handshake();
}


// proxy reads handshake that comes from client.
// it contains methods that client supports.
void Session::read_socks5_handshake()
{
	auto self(shared_from_this());

	// proxy receive version identifier/method selection message and store it in in_buf_
	in_socket_.async_receive(boost::asio::buffer(in_buf_),
		[this, self](boost::system::error_code ec, std::size_t length) {
			
			if (!ec){

					// it has at least 3 byte and the first byte is socks version, second byte is number of methods.
				if (length < 3 || in_buf_[0] != 0x05){

					std::ostringstream tmp;  
					tmp << session_id_ << ": SOCKS5 handshake request is invalid. Closing session...";
					logger3.log(tmp.str(), "error");
					return;
				}

				uint8_t num_methods = in_buf_[1];
				// Prepare answer
				in_buf_[1] = 0xFF;

				// Only 0x00 - 'NO AUTHENTICATION REQUIRED' is now supported
				for (uint8_t method = 0; method < num_methods; ++method)
					if (in_buf_[2 + method] == 0x00) {
						in_buf_[1] = 0x00;
						break; 
					}
				
				write_socks5_handshake();
			
			}else{
				
				std::ostringstream tmp;  
				tmp << session_id_ << ": SOCKS5 handshake request, " << ec.message();
				logger3.log(tmp.str(), "error");
			}
		});
}


// proxy write handshake to client.
// it contains choose method by proxy.
void Session::write_socks5_handshake(){

	auto self(shared_from_this());

	boost::asio::async_write(in_socket_, boost::asio::buffer(in_buf_, 2), // Always 2-byte according to RFC1928
		[this, self](boost::system::error_code ec, std::size_t length){
			
			if (!ec){
				
				if (in_buf_[1] == 0xFF)
					return; // No appropriate auth method found. Close session.
			
				read_socks5_request();
				
			}else{
				
				std::ostringstream tmp;
				tmp << session_id_ << ": SOCKS5 handshake response write, " << ec.message();
				logger3.log(tmp.str(), "error");
			}
		});
}


// after initial handshake, client sends a request to proxy
// which contains port and IP of the server that it wants to connect
void Session::read_socks5_request(){
	
	auto self(shared_from_this());

	in_socket_.async_receive(boost::asio::buffer(in_buf_),
		[this, self](boost::system::error_code ec, std::size_t length){
			
			if (!ec){
				
				// it has at least 5 byte and the first byte is socks version. second byte is method, here just CONNECT supported
				if (length < 5 || in_buf_[0] != 0x05 || in_buf_[1] != 0x01){
					
					std::ostringstream tmp;  
					tmp << session_id_ << ": SOCKS5 request is invalid. Closing session... ";
					logger3.log(tmp.str(), "error");
					return;
				}

				// byte 4 is address type
				uint8_t addr_type = in_buf_[3], host_length;

				switch (addr_type){
					
					case 0x01: // IP V4 addres
						if (length != 10) { 
							std::ostringstream tmp;  
							tmp << session_id_ << ": SOCKS5 request length is invalid. Closing session... ";
							logger3.log(tmp.str(), "error");
							return; 
						}
						remote_host_ = boost::asio::ip::address_v4(ntohl(*((uint32_t*)&in_buf_[4]))).to_string();
						remote_port_ = std::to_string(ntohs(*((uint16_t*)&in_buf_[8])));
						break;
						
					case 0x03: // DOMAINNAME
						host_length = in_buf_[4];
						if (length != (size_t)(5 + host_length + 2)) {
							std::ostringstream tmp;  
							tmp << session_id_ << ": SOCKS5 request length is invalid. Closing session... ";
							logger3.log(tmp.str(), "error");
							return; 
						}
						remote_host_ = std::string(&in_buf_[5], host_length);
						remote_port_ = std::to_string(ntohs(*((uint16_t*)&in_buf_[5 + host_length])));
						break;
						
					default:
						std::ostringstream tmp;  
						tmp << session_id_ << ": unsupport_ed address type in SOCKS5 request. Closing session... ";
						logger3.log(tmp.str(), "error");
						break;
				}

				do_resolve();
			
			}else{
				
				std::ostringstream tmp;  
				tmp << session_id_ << ": SOCKS5 request read " << ec.message();
				logger3.log(tmp.str(), "error");
			}
		});
}


// resolve entered address
// this function get pair of host (IP or name) and port, and returns list of possible endpoints to connect to.
void Session::do_resolve(){
	
	auto self(shared_from_this());

	resolver.async_resolve(tcp::resolver::query({ remote_host_, remote_port_ }),
		[this, self](const boost::system::error_code& ec, tcp::resolver::iterator it){
			
			if (!ec){
				
				do_connect(it);
			
			}else{
			
				std::ostringstream tmp;
				tmp << session_id_ << ": failed to resolve " << remote_host_ << ":" << remote_port_;
				logger3.log(tmp.str(), "error");
			}
		});
}


// connect to specified endpoint 
void Session::do_connect(tcp::resolver::iterator& it){
	
	auto self(shared_from_this());
	out_socket_.async_connect(*it, 
		[this, self](const boost::system::error_code& ec){
			
			if (!ec){
				
				std::ostringstream tmp; 
				tmp << session_id_ << ": connected to " << remote_host_ << ":" << remote_port_;
				logger3.log(tmp.str(), "info");
				write_socks5_response();
			
			}else{
			
				std::ostringstream tmp; 
				tmp << session_id_ << ": failed to connect " << remote_host_ << ":" << remote_port_ << ec.message();
				logger3.log(tmp.str(), "error");
			}
		});

}


// after connection stablish, proxy sends a response to client to show that it was successful
void Session::write_socks5_response(){
	
	auto self(shared_from_this());

	in_buf_[0] = 0x05; // socks version
	in_buf_[1] = 0x00; // reply field, 0 means succeeded
	in_buf_[2] = 0x00; // reserved
	in_buf_[3] = 0x01; // address type, 1 means IPv4
	
	uint32_t realRemoteIP = out_socket_.remote_endpoint().address().to_v4().to_ulong();
	uint16_t realRemoteport = htons(out_socket_.remote_endpoint().port());

	//  byte 5-8 IP, byte 9-10 port
	std::memcpy(&in_buf_[4], &realRemoteIP, 4);
	std::memcpy(&in_buf_[8], &realRemoteport, 2);

	boost::asio::async_write(in_socket_, boost::asio::buffer(in_buf_, 10), // Always 10-byte according to RFC1928
		[this, self](boost::system::error_code ec, std::size_t length){
			
			if (!ec){
				// start communication ...
			
			}else{
				std::ostringstream tmp; 
				tmp << session_id_ << ": SOCKS5 response write " << remote_host_ << ":" << remote_port_ << ec.message();
			}
		});
}


// tcp::socket in_socket_;
// tcp::socket out_socket_;
// tcp::resolver resolver;

std::string remote_host_;
std::string remote_port_;
std::vector<char> in_buf_;
std::vector<char> out_buf_;
int session_id_;
Logger logger3;
