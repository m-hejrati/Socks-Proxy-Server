#include "Session.h"

#include <boost/asio.hpp>

using boost::asio::ip::tcp;

// constructor of session class
Session::Session(tcp::socket in_socket, unsigned session_id, size_t buffer_size, std::string logType) :
    in_socket_(std::move(in_socket)), 
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

				// it is at least 3 byte and the first byte is socks version, second byte is number of methods.
				if (length < 3 || in_buf_[0] != 0x05){

					std::ostringstream tmp;  
					tmp << session_id_ << ": SOCKS5 handshake request is invalid. Closing session...";
					logger3.log(tmp.str(), "error");
					return;
				}

				uint8_t num_methods = in_buf_[1];
				// Prepare answer
				in_buf_[1] = 0xFF;

				// Only 0x00 - 'NO AUTHENTICATION REQUIRED' is now support
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
			
			} else {
				
				std::ostringstream tmp;
				tmp << session_id_ << ": SOCKS5 handshake response write, " << ec.message();
				logger3.log(tmp.str(), "error");
			}
		});
}


// tcp::socket in_socket_;

std::vector<char> in_buf_;
std::vector<char> out_buf_;
int session_id_;
Logger logger3;
