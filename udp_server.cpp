#include <vector>
#include <opencv2/opencv.hpp>
#include <opencv2/imgcodecs.hpp>
#include "UDPMessenger.hpp"
using namespace cv;
using std::vector;

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
	char data[UDP_MAX_SIZE];
	while (true)
	{
		Mat recvImage = Mat::zeros(720, 1280, CV_8UC3);
		int imgsize = udpmsg.getMsg(data, UDP_MAX_SIZE);

		vector<uchar> decodeBuffer;
		for (int i = 0; i < imgsize; i++)
			decodeBuffer.push_back(data[i]);
		imdecode(decodeBuffer, CV_LOAD_IMAGE_COLOR, &recvImage);

		imshow("Recieved image", recvImage);
		waitKey(16);
	}
}

