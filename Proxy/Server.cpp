#include "Server.h"

#include <iostream>
#include <boost/asio.hpp>

using boost::asio::ip::tcp;

// constructor of server class
Server::Server(boost::asio::io_service& io_service, short port, unsigned buffer_size, std::string logType) : 
    acceptor_(io_service, tcp::endpoint(tcp::v4(), port)), 
    in_socket_(io_service),
    buffer_size_(buffer_size),
    session_id_(0) {

        logger.setConfigType(logType);

        do_accept();
}

//asynchronously accept new connections and create session
void Server::do_accept(){

    acceptor_.async_accept(in_socket_, [this](boost::system::error_code ec){

            if (!ec){
                std::cout << "Create Session ...";

            } else{

             	std::ostringstream tmp;  
                tmp << session_id_ << " socket accept error " << ec.message();
                logger.log(tmp.str(), "error");
            }

            do_accept();
        });
}

// tcp::acceptor acceptor_;
// tcp::socket in_socket_;
size_t buffer_size_;
short verbose_;
unsigned session_id_;
Logger logger;