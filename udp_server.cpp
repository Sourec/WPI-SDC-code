#include <opencv2/opencv.hpp>
#include <opencv2/imgcodecs.hpp>
#include "UDPMessenger.hpp"
using namespace cv;
using namespace std;

//udp_server demo
//Recieves an image from the client.
int main(int argc, char* argv[])
{
	if (argc < 3)
	{
		printf("Not enough arguments!\n");
		printf("Usage: ./server [listen] [port]\n");
		return 1;
	}

	UDPMessenger udpmsg(argv[1], atoi(argv[2]));

	//So opencv uses HxW instead of WxH. Who the f**k does that?!
	while (true)
	{
		Mat recvImage = Mat::zeros(480, 640, CV_8UC3);
		int imgsize = 0;
		char* data = udpmsg.getChunks((uint32_t*)&imgsize);

		//Basic technique of recomposing an image from an array
		//shamelessly lifted off a stackoverflow post.
		int ptr = 0;
		for (int x = 0; x < recvImage.rows; x++)
		{
			for (int y = 0; y < recvImage.cols; y++)
			{
				recvImage.at<Vec3b>(x,y) = Vec3b(data[ptr], data[ptr+1], data[ptr+2]);
				ptr += 3;
			}
		}
		free(data);
		imshow("Recieved image", recvImage);
		waitKey(16);
	}
}

