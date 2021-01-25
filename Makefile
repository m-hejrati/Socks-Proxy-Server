CXX = g++
CXXFLAGS = -std=c++11 -Wall -c
LXXFLAGS = -std=c++11
OBJECTS = main.o Server.o
TARGET = main

$(TARGET): $(OBJECTS)
	$(CXX) $(LXXFLAGS) $(OBJECTS) -o $(TARGET) -lboost_system -lboost_thread -lpthread
Server.o: Proxy/Server.cpp Proxy/Server.h
	$(CXX) $(CXXFLAGS) "Proxy/Server.cpp" -lboost_system -lboost_thread -lpthread
main.o: main.cpp
	$(CXX) $(CXXFLAGS) main.cpp -lboost_system -lboost_thread -lpthread

