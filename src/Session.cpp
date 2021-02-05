#include "Session.h"
#include "LogVariables.h"

#include <boost/asio.hpp>
#include <inttypes.h>

using boost::asio::ip::tcp;

// constructor of session class
Session::Session(tcp::socket in_socket, unsigned session_id, size_t buffer_size, std::string logType, ConfigReader configReader_):
    in_socket_(std::move(in_socket)), 
    out_socket_(in_socket.get_io_service()), 
    resolver(in_socket.get_io_service()),
    in_buf_(buffer_size), 
    out_buf_(buffer_size), 
    session_id_(session_id),
	configReader (configReader_),
	isFilter (0){

        logger3.setConfigType(logType);

		mtx.lock();
		activeSessionsID.push_back(session_id);
		newSession ++;
		mtx.unlock();
}


// this function should be separated from constructor
void Session::start(){

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
					tmp << session_id_ << ": Closing session; SOCKS5 handshake request is invalid.";
					logger3.log(tmp.str(), "error");
					logFinishSession();
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
				
				if (in_buf_[1] == 0xFF){

					std::ostringstream tmp;  
					tmp << session_id_ << ": Closing Session; No appropriate auth method found. ";
					logger3.log(tmp.str(), "error");
					logFinishSession();
					return;
				}
			
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
					tmp << session_id_ << ": Closing session; SOCKS5 request is invalid. ";
					logger3.log(tmp.str(), "error");
					logFinishSession();
					return;
				}

				// byte 4 is address type
				uint8_t addr_type = in_buf_[3], host_length;

				switch (addr_type){
					
					case 0x01: // IP V4 addres
						if (length != 10) { 
							std::ostringstream tmp;  
							tmp << session_id_ << ": Closing session; SOCKS5 request length is invalid. ";
							logger3.log(tmp.str(), "error");
							logFinishSession();
							return; 
						}
						remote_host_ = boost::asio::ip::address_v4(ntohl(*((uint32_t*)&in_buf_[4]))).to_string();
						remote_port_ = std::to_string(ntohs(*((uint16_t*)&in_buf_[8])));
						break;
						
					case 0x03: // DOMAINNAME
						host_length = in_buf_[4];
						if (length != (size_t)(5 + host_length + 2)) {
							std::ostringstream tmp;  
							tmp << session_id_ << ": Closing session; SOCKS5 request length is invalid. ";
							logger3.log(tmp.str(), "error");
							logFinishSession();
							return; 
						}
						remote_host_ = std::string(&in_buf_[5], host_length);
						remote_port_ = std::to_string(ntohs(*((uint16_t*)&in_buf_[5 + host_length])));
						break;
						
					default:
						std::ostringstream tmp;  
						tmp << session_id_ << ": Closing session; unsupport address type in SOCKS5 request. ";
						logger3.log(tmp.str(), "error");
						logFinishSession();
						return;
				}

				storeDomainName();

				do_resolve();

			}else{
				
				std::ostringstream tmp;  
				tmp << session_id_ << ": SOCKS5 request read, " << ec.message();
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

				// filter
				int status = configReader.checkFilter(remote_host_, remote_port_);
				
				if (status == 0){

					isFilter = 0;
				
				} else {

					isFilter = 1;

					std::ostringstream tmp;  
					tmp << session_id_ << ": ";

					if (status == 1){

						tmp << "Filter by IP";
						logger3.log(tmp.str(), "warn");

					}else if (status == 2){
						
						tmp << "Filter by Port";
						logger3.log(tmp.str(), "warn");

					}else if (status == 3){
						
						tmp << "Filter by Protocol";
						logger3.log(tmp.str(), "warn");
					
					}else if (status == 4){

						tmp << "Filter by pair of Ip & Port";
						logger3.log(tmp.str(), "warn");
					}
				}

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


// after connection establish, proxy sends a response to client to show that it was successful
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
				// start communication.
				// reading messages from both cilent and server sides
				do_read(3);

			}else{
				std::ostringstream tmp; 
				tmp << session_id_ << ": SOCKS5 response write " << remote_host_ << ":" << remote_port_ << ec.message();
				logger3.log(tmp.str(), "error");
			}
		});
}


// this function get messages;
// direction 1: from client to in_socket and store in in_buf
// direction 2: from server to out_socket and store in out_buf
void Session::do_read(int direction){
	
	auto self(shared_from_this());

	// read from client
	if (direction & 0x1)
		in_socket_.async_receive(boost::asio::buffer(in_buf_),
			[this, self, direction](boost::system::error_code ec, std::size_t length){
				
				changeSessionsID.insert(session_id_);

				if (!ec){
					
					std::ostringstream tmp; 
					tmp << session_id_ << ": --> " << std::to_string(length) << " bytes";
					logger3.log(tmp.str(), "debug");

					if (isFilter){

						mtx.lock();
						filterPacket ++;
						filterTraffic += length;
						mtx.unlock();

						do_read(1);
					
					}else
						do_write(1, length);
				
				}else{ //if (ec != boost::asio::error::eof)
				
					std::ostringstream tmp; 
					tmp << session_id_ << ": closing session. Client socket read error ", ec.message();
					logger3.log(tmp.str(), "warn");
					// Most probably client closed socket. Let's close both sockets and exit session.
					in_socket_.close();
					out_socket_.close();
					logFinishSession();
				}

			});

	// read from server
	if (direction & 0x2)
		out_socket_.async_receive(boost::asio::buffer(out_buf_),
			[this, self, direction](boost::system::error_code ec, std::size_t length){
			
				changeSessionsID.insert(session_id_);

				if (!ec){
				
					std::ostringstream tmp; 
					tmp << session_id_ << ": <-- " << std::to_string(length) << " bytes";
					logger3.log(tmp.str(), "debug");

					if (isFilter){

						mtx.lock();
						filterPacket ++;
						filterTraffic += length;
						mtx.unlock();

						do_read(2);
					
					}else
						do_write(2, length);					

				}else{ //if (ec != boost::asio::error::eof)

					std::ostringstream tmp; 
					tmp << session_id_ << ": closing session. Remote socket read error ", ec.message();
					logger3.log(tmp.str(), "warn");
					// Most probably remote server closed socket. Let's close both sockets and exit session.
					in_socket_.close();
					out_socket_.close();
					logFinishSession();
				}
			});
}


// this function send messages;
// direction 1: send in_buf message from out_socket to server 
// direction 2: from out_buf message in_socket to client
void Session::do_write(int direction, std::size_t Length){

	auto self(shared_from_this());

	switch (direction){
	
		// send to server
		case 1:
			boost::asio::async_write(out_socket_, boost::asio::buffer(in_buf_, Length),
				[this, self, direction](boost::system::error_code ec, std::size_t length){
				
					changeSessionsID.insert(session_id_);

					if (!ec){

						mtx.lock();
						passPacket ++;
						passTraffic += length;
						mtx.unlock();

						do_read(direction);

					}else{
						std::ostringstream tmp; 
						tmp << session_id_ << ": closing session. Client socket write error ", ec.message();
						logger3.log(tmp.str(), "warn");
						// Most probably client closed socket. Let's close both sockets and exit session.
						in_socket_.close();
						out_socket_.close();
						logFinishSession();
					}
				});
			break;
			
		// send to client
		case 2:
			boost::asio::async_write(in_socket_, boost::asio::buffer(out_buf_, Length),
				[this, self, direction](boost::system::error_code ec, std::size_t length){
				
					changeSessionsID.insert(session_id_);

					if (!ec){

						mtx.lock();
						passPacket ++;
						passTraffic += length;
						mtx.unlock();
						
						do_read(direction);
					
					}else{
						std::ostringstream tmp; 
						tmp << session_id_ << ": closing session. Remote socket write error ", ec.message();
						logger3.log(tmp.str(), "warn");
						// Most probably remote server closed socket. Let's close both sockets and exit session.
						in_socket_.close();
						out_socket_.close();
						logFinishSession();
					}
				});
			break;
	}
}


// find the session in active list and remove it and also increase finish sessoin number
void Session::logFinishSession(){

	mtx.lock();

	std::vector<int>::iterator position = std::find(activeSessionsID.begin(), activeSessionsID.end(), session_id_);
	// position == end() means the element was not found
	if (position != activeSessionsID.end()){ 
	    activeSessionsID.erase(position);
		finishSession ++;
	}

	mtx.unlock();
}


// get IP and store domain name
void Session::storeDomainName(){

	std::ostringstream command; 
	command <<  "dig -x " << remote_host_ << " +short";

	std::cout << command.str() << endl;

	FILE *fpipe;
    char c = 0;

    if (0 == (fpipe = (FILE*)popen(command.str().c_str(), "r"))){

		std::ostringstream tmp; 
		tmp << "Error in popen";
		logger3.log(tmp.str(), "error");
        exit(1);
    }

    domain_name.str("");
    while (fread(&c, sizeof c, 1, fpipe)){
        domain_name << c;
    }

    pclose(fpipe);
}


// tcp::socket in_socket_;
// tcp::socket out_socket_;
// tcp::resolver resolver;

std::string remote_host_;
std::string remote_port_;
std::ostringstream domain_name;
std::vector<char> in_buf_;
std::vector<char> out_buf_;
int session_id_;
Logger logger3;
ConfigReader configReader;
bool isFilter;
mutex mtx;