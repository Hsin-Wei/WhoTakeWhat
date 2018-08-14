// Standard Library
#include <iostream>

#include <fstream>
#include <string>

// OpenCV Header
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>

// Kinect for Windows SDK Header
#include <Kinect.h>

// For web connect
#define CURL_STATICLIB 
#include <curl/curl.h>

using namespace std;
using namespace cv;
// Parameters


int main(int argc, char** argv)
{
	// read images as video frames
	VideoCapture cap("D:/GoogleDrive/Graduate/Research/Research_HsinWei/Programs/data/ObjectDetection/rawData/original.avi");
	Mat frame;

	if (!cap.isOpened()) {
		std::cout << "Cannot open the video file on C++ API" << std::endl;
		return -1;
	}

	// save frames as video
	// cv::VideoWriter writer;
	// writer.open("C:/Research_temp/original.avi", CV_FOURCC('M', 'J', 'P', 'G'), 30, cv::Size(800, 600), true);

	// For web
	CURL *curl;
	CURLcode res;
	/* In windows, this will init the winsock stuff */
	curl_global_init(CURL_GLOBAL_ALL);
	/* get a curl handle */
	curl = curl_easy_init();
	/* First set the URL that is about to receive our POST. This URL can
	just as well be a https:// URL if that is what should receive the
	data. */
	curl_easy_setopt(curl, CURLOPT_URL, "http://140.113.193.8:8896/update");


	int i = 0;
	while (true) 
	{
		// read frame
		cap >> frame;
		if (frame.empty() || i == 2) 
		{
			break;
		}
		stringstream ss;
		ss << "D:/GoogleDrive/Graduate/Research/Research_HsinWei/Programs/data/ObjectDetection/rawData/ori/" << i << ".jpg";
		cv:imwrite(ss.str(), frame);

		/* Now specify the POST data */
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, frame);
		/* Perform the request, res will get the return code */
		res = curl_easy_perform(curl);

		/* Check for errors */
		if (res != CURLE_OK)
			fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));

		i++;
	}
	/* always cleanup */
	curl_easy_cleanup(curl);
	curl_global_cleanup();
}

