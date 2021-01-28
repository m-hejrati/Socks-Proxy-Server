#include "Server.h"

#include <iostream>
#include <boost/asio.hpp>

using boost::asio::ip::tcp;

// constructor of server class
Server::Server(boost::asio::io_service& io_service, short port, unsigned buffer_size, std::string logType) : 
    acceptor_(io_service, tcp::endpoint(tcp::v4(), port)), 
    in_socket_(io_service),
    buffer_size_(buffer_size),
    session_id_0 (0) {

        logger2.setConfigType(logType);

        // read config file just once and use it in every session 
        ConfigReader configReader;
        configReader.readConf("config.json");

        //accept new connections
        do_accept(configReader);
}

//asynchronously accept new connections and create session
void Server::do_accept(ConfigReader configReader){

    acceptor_.async_accept(in_socket_, [this, configReader](boost::system::error_code ec){

        if (!ec){     

            // create new session and start handshakes...
            std::make_shared<Session>(std::move(in_socket_), session_id_0 ++, buffer_size_, logger2.getConfigType(), configReader)->start();

        } else{

            std::ostringstream tmp;  
            tmp << session_id_0 << " socket accept error " << ec.message();
            logger2.log(tmp.str(), "error");
        }

        do_accept(configReader);
    });
}

// tcp::acceptor acceptor_;
// tcp::socket in_socket_;
size_t buffer_size_;
unsigned session_id_0;
Logger logger2;