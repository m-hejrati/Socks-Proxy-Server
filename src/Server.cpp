#include "Server.h"

#include <iostream>
#include <boost/asio.hpp>
#include <thread>
#include <mutex>

using boost::asio::ip::tcp;

// constructor of server class
Server::Server(boost::asio::io_service& io_service, short port, unsigned buffer_size, std::string logType) : 
    acceptor_(io_service, tcp::endpoint(tcp::v4(), port)), 
    in_socket_(io_service),
    buffer_size_(buffer_size),
    session_id_0 (0) {

        logger2.setConfigType(logType);

}

// run
void Server::start(){

    // read config file just once and use it in every session 
    ConfigReader configReader;
    configReader.readConf("../config.json");

    // create a new thread and log every minutes.
    std::thread logThread(&Server::logEveryMinute, this);

    //accept new connections
    do_accept(configReader);

    logThread.detach();
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

// print (log) the amount of logVariables
void Server::printLogVars (){

    logger2.log("Print statistics ...", "info");

    std::ostringstream passTmp;
    if (passTraffic < 1000) 
        passTmp << passTraffic << " B";
    else if (passTraffic >= 1000 && passTraffic < 1000000)
        passTmp << std::setprecision(4) << passTraffic / 1024.0 << " KB";
    else if (passTraffic >= 1000000)
        passTmp << std::setprecision(4) << passTraffic / 1048576.0 << " MB";

    std::ostringstream filterTmp;
    if (filterTraffic < 1000) 
        filterTmp << filterTraffic << " B";
    else if (filterTraffic >= 1000 && filterTraffic < 1000000)
        filterTmp << std::setprecision(4) << filterTraffic / 1024.0 << " KB";
    else if (filterTraffic >= 1000000)
        filterTmp << std::setprecision(4) << filterTraffic / 1048576.0 << " MB";

    std::ostringstream tmp;  
    tmp << "Pass Packet:    " << passPacket << "\t Pass Traffic:   " <<  passTmp.str();
    logger2.log(tmp.str(), "info");

    tmp.str("");
    tmp << "Active Session: " << activeSession << "\t Update Session: " << updateSession;
    logger2.log(tmp.str(), "info");   

    tmp.str("");
    tmp << "Finish Session: " << finishSession << "\t New Session:    " << newSession;
    logger2.log(tmp.str(), "info");

    tmp.str("");
    tmp << "Filter Packet:  " << filterPacket << "\t Filter Traffic: " << filterTmp.str();
    logger2.log(tmp.str(), "info");
}

// log every minute
void Server::logEveryMinute(){

    mutex mtx;

    while(true){

        // I put both read and reset functions in mutex,
        // because its possible to capture a packet after logging and before resetting, so the statistics will be wrong.
        mtx.lock();
        printLogVars();
        resetVariables();
        mtx.unlock();

        // hold logEveryMinute execution for 60 seconds
        std::this_thread::sleep_for(60s);
  }
}

// reset all the log variables
void Server::resetVariables(){

    passPacket = 0;
    passTraffic = 0;
    activeSession = 0;
    updateSession = 0;
    finishSession = 0;
    newSession = 0; 
    filterPacket = 0;
    filterTraffic = 0;
}

// tcp::acceptor acceptor_;
// tcp::socket in_socket_;
size_t buffer_size_;
unsigned session_id_0;
Logger logger2;
