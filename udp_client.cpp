#include <stdio.h>
#include <vector>
#include <opencv2/opencv.hpp>
#include <opencv2/imgcodecs.hpp>
#ifdef RASP_PI
#include <raspicam/raspicam_cv.h>
using namespace raspicam;
#endif
#include "UDPMessenger.hpp"
using namespace cv;
using std::vector;

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

	UDPMessenger udpmsg((char*)"0", 1701); //ugh
	udpmsg.setDest(argv[1], atoi(argv[2]));
	
#ifdef RASP_PI
	RaspiCam_Cv cam;
	if (!cam.open())
	{
		printf("Couldn't open camera!\n");
		return 1;
	}
#else	
	VideoCapture cam;
	if (!cam.open(0))
	{
		printf("Couldn't open camera!\n");
		return 1;
	}
#endif
	

	vector<int> param(2);
	param[0] = IMWRITE_JPEG_QUALITY;
	param[1] = 80;

	while (waitKey(1) != 27)
	{	
		Mat img;

#ifdef RASP_PI
		cam.grab();
		cam.retrieve(img);
#else
		cam >> img;
#endif

		//Encode to jpeg, quality 80%
		vector<uchar> encodeBuffer;
		imencode(".jpg", img, encodeBuffer, param);

		udpmsg.sendChunks((const char*)&encodeBuffer[0], encodeBuffer.size(), 1);
		waitKey(16); //wait 16ms
	}
	cam.release();
	return 0;
}

