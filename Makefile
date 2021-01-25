CXX = g++
CXXFLAGS = -std=c++11 -Wall -c
LXXFLAGS = -std=c++11
OBJECTS = main.o Server.o Logger.o
TARGET = main

$(TARGET): $(OBJECTS)
	$(CXX) $(LXXFLAGS) $(OBJECTS) -o $(TARGET) -lboost_system -lboost_thread -lpthread -I "spdlog/include"
Logger.o: Logger/Logger.cpp Logger/Logger.h
	$(CXX) $(CXXFLAGS) "Logger/Logger.cpp" -I "spdlog/include"
Server.o: Proxy/Server.cpp Proxy/Server.h
	$(CXX) $(CXXFLAGS) "Proxy/Server.cpp" -lboost_system -lboost_thread -lpthread  -I "spdlog/include"
main.o: main.cpp
	$(CXX) $(CXXFLAGS) main.cpp -lboost_system -lboost_thread -lpthread -I "spdlog/include"

