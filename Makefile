CXX = g++
CXXFLAGS = -std=c++11 -Wall -c
LXXFLAGS = -std=c++11
OBJECTS = main.o Logger.o Session.o Server.o ConfigReader.o
TARGET = main

$(TARGET): $(OBJECTS)
	$(CXX) $(LXXFLAGS) $(OBJECTS) -o $(TARGET) -lboost_system -lboost_thread -lpthread -I "spdlog/include"
Logger.o: src/Logger.cpp include/Logger.h
	$(CXX) $(CXXFLAGS) "src/Logger.cpp" -I "spdlog/include"
ConfigReader.o: src/ConfigReader.cpp include/ConfigReader.h
	$(CXX) $(CXXFLAGS) "src/ConfigReader.cpp"
Session.o: src/Session.cpp include/Session.h
	$(CXX) $(CXXFLAGS) "src/Session.cpp" -lboost_system -lboost_thread -lpthread  -I "spdlog/include"
Server.o: src/Server.cpp include/Server.h
	$(CXX) $(CXXFLAGS) "src/Server.cpp" -lboost_system -lboost_thread -lpthread  -I "spdlog/include"
main.o: src/main.cpp
	$(CXX) $(CXXFLAGS) "src/main.cpp" -lboost_system -lboost_thread -lpthread -I "spdlog/include"

