CXX = g++
CXXFLAGS = -std=c++11 -Wall -c
LXXFLAGS = -std=c++11
OBJECTS = main.o Logger.o Session.o Server.o ConfigReader.o
TARGET = main

$(TARGET): $(OBJECTS)
	$(CXX) $(LXXFLAGS) $(OBJECTS) -o $(TARGET) -lboost_system -lboost_thread -lpthread -I "spdlog/include"
Logger.o: Logger/Logger.cpp Logger/Logger.h
	$(CXX) $(CXXFLAGS) "Logger/Logger.cpp" -I "spdlog/include"
ConfigReader.o: Input/ConfigReader.cpp Input/ConfigReader.h
	$(CXX) $(CXXFLAGS) "Input/ConfigReader.cpp"
Session.o: Proxy/Session.cpp Proxy/Session.h
	$(CXX) $(CXXFLAGS) "Proxy/Session.cpp" -lboost_system -lboost_thread -lpthread  -I "spdlog/include"
Server.o: Proxy/Server.cpp Proxy/Server.h
	$(CXX) $(CXXFLAGS) "Proxy/Server.cpp" -lboost_system -lboost_thread -lpthread  -I "spdlog/include"
main.o: main.cpp
	$(CXX) $(CXXFLAGS) main.cpp -lboost_system -lboost_thread -lpthread -I "spdlog/include"

