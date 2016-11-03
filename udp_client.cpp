#include <stdio.h>
#include <opencv2/opencv.hpp>
#include <opencv2/imgcodecs.hpp>
#include "UDPMessenger.hpp"
using namespace cv;

//udp_client demo
//Grabs an image from the camera and sends it to
//the server using UDPMessenger.
int main(int argc, char* argv[])
{
	if (argc < 3)
	{
		printf("Not enough arguments!\n");
		printf("Usage: ./client [dest] [port]\n");
		return 1;
	}

	UDPMessenger udpmsg((char*)"127.0.0.1", 1701); //ugh
	udpmsg.setDest(argv[1], atoi(argv[2]));
	
	VideoCapture cam;
	if (!cam.open(0))
	{
		printf("Couldn't open camera!\n");
		return 1;
	}

	while (waitKey(1) != 27)
	{	
		Mat img;
		cam >> img;
		img = img.reshape(0,1);
		udpmsg.sendChunks((const char*)img.data, img.total() * img.elemSize(), 10, 200);
		waitKey(16); //wait 16ms
	}
	cam.release();
	return 0;
}

