cmake_minimum_required(VERSION 3.0)
project(nettest)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -g -O3 -D RASP_PI -lraspicam -lraspicam_cv")

set(SERVER_SOURCES
	UDPMessenger.hpp
    	UDPMessenger.cpp
    	udp_server.cpp)

set(CLIENT_SOURCES
        UDPMessenger.hpp
    	UDPMessenger.cpp
	udp_client.cpp)

find_package( OpenCV REQUIRED )
add_executable(udp_client ${CLIENT_SOURCES})
add_executable(udp_server ${SERVER_SOURCES})
target_link_libraries( udp_server ${OpenCV_LIBS} )
target_link_libraries( udp_client ${OpenCV_LIBS} )

