// Standard Library
#include "stdafx.h"
#include <iostream>

#include <fstream>
#include <string>
#include <time.h>

// OpenCV Header
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>

// Kinect for Windows SDK Header
#include <Kinect.h>

using namespace std;

// Parameters
int countdown = 10;
int step = 10000; // How many frames do you want to record
string csvPath = "C:/Users/NCTU/Desktop/WhoTakeWhat/data/Kinect/original.csv";
string framePath = "C:/Users/NCTU/Desktop/WhoTakeWhat/data/Kinect/";


// global objects
IKinectSensor* sensor;             // Kinect sensor
IMultiSourceFrameReader* reader;   // Kinect data source
ICoordinateMapper* mapper;         // Converts between depth, color, and 3d coordinates
INT64 timestemp;

int		iColorWidth = 0, iColorHeight = 0;
UINT	uDepthPointNum = 0;
UINT	uColorPointNum = 0;
UINT	uColorBufferSize = 0;

UINT16*	pDepthBuffer = nullptr;
BYTE*	pColorBuffer = nullptr;
CameraSpacePoint* pCSPoints = nullptr;

IMultiSourceFrame* frame = NULL;

// vector of frames
vector<cv::Mat> frames;
// vector of timestamps
vector<string> timestamps;
// For demo show
cv::Mat demo_frame;


bool initKinect();
bool initFrameTimeRelated();


int main(int argc, char** argv)
{
	if (initKinect() == false) {
		cerr << "init error";
		return 0;
	}

	if (initFrameTimeRelated() == false) {
		cerr << "init frame error";
		return 0;
	}

	// prepare timestamp
	char time_str[13];
	memset(time_str, '\0', 13);
	SYSTEMTIME st;


	// Prepare OpenCV data
	cv::Mat	mImg(iColorHeight, iColorWidth, CV_8UC4);

	// file output stream
	ofstream csvfile;
	csvfile.open(csvPath);


	// countdown to start
	cout << "Prepare hardward...\n";
	while (countdown > 0) {
		cout << countdown << endl;
		countdown--;
		Sleep(1000);
	}

	// Enter main loop
	int count = 0;
	while (count < step) {
		//cout << count;
		if (reader->AcquireLatestFrame(&frame) == S_OK) {
			//Get Depth data
			{
				IDepthFrame* depthframe;
				IDepthFrameReference* Dframeref = NULL;
				frame->get_DepthFrameReference(&Dframeref);
				Dframeref->AcquireFrame(&depthframe);
				if (Dframeref) Dframeref->Release();
				if (!depthframe) continue;

				// Get data from frame
				if (depthframe->CopyFrameDataToArray(uDepthPointNum, pDepthBuffer) != S_OK) {
					continue;
				}

				if (depthframe) depthframe->Release();
			}

			//Get Rgb data
			{
				IColorFrame* colorframe;
				IColorFrameReference* Cframeref = NULL;
				frame->get_ColorFrameReference(&Cframeref);
				Cframeref->AcquireFrame(&colorframe);

				//Get timestamp based on color frame
				GetLocalTime(&st);
				sprintf_s(time_str, 13, "%02d:%02d:%02d:%03d", st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);
				timestamps.push_back(string(time_str));

				if (Cframeref) Cframeref->Release();
				if (!colorframe) continue;

				// Get data from frame
				if (colorframe->CopyConvertedFrameDataToArray(uColorBufferSize, pColorBuffer, ColorImageFormat_Bgra) != S_OK) {
					continue;
				}

	
				mImg.data = pColorBuffer;

				//Demo video in this screen
				cv::resize(mImg, demo_frame, cv::Size(mImg.cols / mImg.cols * 800, mImg.rows / mImg.rows * 600), CV_INTER_LINEAR);
				cv::imshow("Recorder", demo_frame);
				frames.push_back(demo_frame.clone());
				
				cv::waitKey(1);
				

				//std::stringstream fmt;
				//fmt << framePath << count << ".jpg";
				//cv::imwrite(fmt.str(), demo_frame);
				


				
				// if get rgb frame
				count++;

				if (colorframe) colorframe->Release();
			}

			//Get coordinate based on rgb frame
			{
				if (mapper->MapColorFrameToCameraSpace(uDepthPointNum, pDepthBuffer, uColorPointNum, pCSPoints) != S_OK)
				{
					cerr << "Map to camera space error" << endl;
					continue;
				}

				//Do something...
			}
		}
	}

	// save frames as video
	//cv::VideoWriter writer;
	//writer.open("C:/Users/NCTU/Desktop/WhoTakeWhat/data/Kinect/Video_Demo.avi", CV_FOURCC('M', 'J', 'P', 'G'), 30, cv::Size(800, 600), true);

	cout << "Recording finish";
    // save frames as jpg
	for (int i = 0; i < frames.size(); i++) {
		cv::cvtColor(frames[i], frames[i], cv::COLOR_BGRA2BGR);
		std::stringstream fmt;
		fmt << framePath << i << ".jpg";
		cv::imwrite(fmt.str(), frames[i]);
	}

	// save timestamps correspond with frames as csv
	for (int i = 0; i < timestamps.size(); i++) {
		csvfile << i << "," << timestamps[i] << "\n";
	}

}

bool initKinect() {
	if (FAILED(GetDefaultKinectSensor(&sensor))) {
		return false;
	}
	if (sensor) {
		sensor->get_CoordinateMapper(&mapper);

		sensor->Open();
		sensor->OpenMultiSourceFrameReader(
			FrameSourceTypes::FrameSourceTypes_Depth | FrameSourceTypes::FrameSourceTypes_Color,
			&reader);
		return reader;
	}
	else {
		return false;
	}
}

bool initFrameTimeRelated() {

	//reserve frames and timestamps for recording
	frames.reserve(step);
	timestamps.reserve(step);

	// Get Color frame source
	{
		IColorFrameSource* pFrameSource = nullptr;
		if (sensor->get_ColorFrameSource(&pFrameSource) != S_OK)
		{
			cerr << "Can't get color frame source" << endl;
			return false;
		}
		// Get Color frame description
		cout << "get color frame description" << endl;
		IFrameDescription* pFrameDescription = nullptr;
		if (pFrameSource->get_FrameDescription(&pFrameDescription) == S_OK)
		{
			pFrameDescription->get_Width(&iColorWidth);
			pFrameDescription->get_Height(&iColorHeight);

			uColorPointNum = iColorWidth * iColorHeight;
			uColorBufferSize = uColorPointNum * 4 * sizeof(BYTE);

			pCSPoints = new CameraSpacePoint[uColorPointNum];
			pColorBuffer = new BYTE[4 * uColorPointNum];
		}

		pFrameDescription->Release();
		pFrameDescription = nullptr;

		// release Frame source
		cout << "Release Color frame source" << endl;
		pFrameSource->Release();
		pFrameSource = nullptr;
	}


	// Get Depth frame source
	{
		IDepthFrameSource* pFrameSource = nullptr;
		if (sensor->get_DepthFrameSource(&pFrameSource) != S_OK)
		{
			cerr << "Can't get depth frame source" << endl;
			return -1;
		}

		// Get frame description
		cout << "get depth frame description" << endl;
		IFrameDescription* pFrameDescription = nullptr;
		if (pFrameSource->get_FrameDescription(&pFrameDescription) == S_OK)
		{
			int	iDepthWidth = 0,
				iDepthHeight = 0;
			pFrameDescription->get_Width(&iDepthWidth);
			pFrameDescription->get_Height(&iDepthHeight);
			uDepthPointNum = iDepthWidth * iDepthHeight;
			pDepthBuffer = new UINT16[uDepthPointNum];
		}
		pFrameDescription->Release();
		pFrameDescription = nullptr;

		// release Frame source
		cout << "Release Depth frame source" << endl;
		pFrameSource->Release();
		pFrameSource = nullptr;
	}
}

/*
void getKinectData() {
IMultiSourceFrame* frame = NULL;
reader->AcquireLatestFrame(&frame);
// ...
getDepthData(frame, dest);
// ...
getColorData(frame, dest);
}

void getDepthData(IMultiSourceFrame* frame, GLubyte* dest) {
IDepthFrame* depthframe;
IDepthFrameReference* frameref = NULL;
frame->get_DepthFrameReference(&frameref);
frameref->AcquireFrame(&depthframe);
if (frameref) frameref->Release();
if (!depthframe) return;

// Process depth frame data...

if (depthframe) depthframe->Release();
}
*/