CXXFLAGS=-g -std=c++1y -Wall -Wextra -Werror -lopencv_highgui -lopencv_videoio -lopencv_imgproc -lopencv_core


all: DFTDemo

clean:
	rm -f *.o
	rm -f DFTDemo

DFTDemo: DFTDemo.cpp
	g++ $(CXXFLAGS) -o DFTDemo DFTDemo.cpp
