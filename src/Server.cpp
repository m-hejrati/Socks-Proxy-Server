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
void Server::printLogVars (vector<int> preActive){

    logger2.log(" ", "info");
    logger2.log("Print statistics ...\n", "info");

    // find number of updated session; sessions that was avtive one minute ago and send data in last one minute.
    for (auto i : preActive)
        for (int j : changeSessionsID)
            if (i == j)
                updateSession ++;

    // print all variables
    std::ostringstream tmp;  
    tmp << "Pass Packet:    " << passPacket << "\t Pass Traffic:   " <<  unitConversion(passTraffic);
    logger2.log(tmp.str(), "info");

    tmp.str("");
    tmp << "Active Session: " << activeSessionsID.size() << "\t Update Session: " << updateSession;
    logger2.log(tmp.str(), "info");   

    tmp.str("");
    tmp << "Finish Session: " << finishSession << "\t New Session:    " << newSession;
    logger2.log(tmp.str(), "info");

    tmp.str("");
    tmp << "Filter Packet:  " << filterPacket << "\t Filter Traffic: " << unitConversion(filterTraffic);
    logger2.log(tmp.str(), "info");

    tmp.str("");
    std::map<std::string, int>::iterator it;
    for(it = domainSession.begin(); it != domainSession.end(); ++it){
        tmp.str("");
        tmp << "Specified Domains to log: " << it->first;
        logger2.log(tmp.str(), "info");
        tmp.str("");
        tmp << "Number of sessions: " << it->second << "\t Traffic of sessions: " << unitConversion(domainTraffic[it->first]);
        logger2.log(tmp.str(), "info");
    }

    logger2.log(" ", "info");
}

// log every minute
void Server::logEveryMinute(){

    mutex mtx;
    // create a new vector to hold sessions that was active one minute ago
    vector<int> preActive;

    while(true){

        // hold logEveryMinute execution for 60 seconds
        std::this_thread::sleep_for(60s);

        // I put both read and reset functions in mutex,
        // because its possible to capture a packet after logging and before resetting, so the statistics will be wrong.
        mtx.lock();
 
        printLogVars(preActive);
        resetVariables();
        // save active session, to use them one minute later
        preActive.clear();
        preActive = activeSessionsID;

        mtx.unlock();
  }
}


// reset all the log variables
void Server::resetVariables(){

    passPacket = 0;
    passTraffic = 0;
    updateSession = 0;
    finishSession = 0;
    newSession = 0; 
    filterPacket = 0;
    filterTraffic = 0;
    std::map<std::string, int>::iterator it;
    for(it = domainSession.begin(); it != domainSession.end(); ++it)
        it->second = 0;
    for(it = domainTraffic.begin(); it != domainTraffic.end(); ++it)
        it->second = 0;
}


// get data and convert to its appropriate unit
std::string Server::unitConversion(int data){
	
    std::ostringstream tmp;
    if (data < 1000) 
        tmp << data << " B";
    else if (data >= 1000 && data < 1000000)
        tmp << std::setprecision(4) << data / 1024.0 << " KB";
    else if (data >= 1000000)
        tmp << std::setprecision(4) << data / 1048576.0 << " MB";	
	
	return tmp.str();
}


// tcp::acceptor acceptor_;
// tcp::socket in_socket_;
size_t buffer_size_;
unsigned session_id_0;
Logger logger2;
