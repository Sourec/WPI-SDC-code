CXXFLAGS=-std=c++1y -Wall -Wextra -g -lopencv_highgui -lopencv_videoio -lopencv_imgproc -lopencv_core -lopencv_imgcodecs

all: client server

client: udp_client.cpp UDPMessenger.o
	g++ $(CXXFLAGS) -o client udp_client.cpp UDPMessenger.o
server: udp_server.cpp UDPMessenger.o
	g++ $(CXXFLAGS) -o server udp_server.cpp UDPMessenger.o
UDPMessenger.o: UDPMessenger.cpp UDPMessenger.hpp
	g++ $(CXXFLAGS) -c UDPMessenger.cpp

clean:
	rm -f *.o
	rm -f server
	rm -f client
