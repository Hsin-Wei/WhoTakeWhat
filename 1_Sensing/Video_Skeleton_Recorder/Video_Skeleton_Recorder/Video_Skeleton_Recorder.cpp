// Created by WeiChun @ 2017 / 07 / 03
// This sample is used to read information of body joint nad draw with OpenCV.
//
// Modified by HsinWei @ 2018 / 05 / 08


// Standard Library
//#include "stdafx.h"
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
int countdown = 7;
int step = 1600; // How many frames do you want to record
int demoWidth = 800;
int demoHeight = 600;
string csvPath = "C:/Users/NCTU/Desktop/WhoTakeWhat/data/Skeleton/VSFile.csv";
string csv_timePath = "C:/Users/NCTU/Desktop/WhoTakeWhat/data/Skeleton/frameTime.csv";
string videoPath = "C:/Users/NCTU/Desktop/WhoTakeWhat/data/Skeleton/Video_Demo.avi";


// global objects
IKinectSensor* sensor;             // Kinect sensor
IMultiSourceFrameReader* reader;   // Kinect data source
ICoordinateMapper* mapper;         // Converts between depth, color, and 3d coordinates
INT64 timestemp;

int		iColorWidth = 0, iColorHeight = 0;
UINT	uDepthPointNum = 0;
UINT	uColorPointNum = 0;
UINT	uColorBufferSize = 0;
INT32   iBodyCount = 0;

UINT16*	pDepthBuffer = nullptr;
BYTE*	pColorBuffer = nullptr;
CameraSpacePoint* pCSPoints = nullptr;
IBody** aBodyData = nullptr;
ICoordinateMapper* pCoordinateMapper = nullptr;

IMultiSourceFrame* frame = NULL;


// vector of frames
vector<cv::Mat> frames;
// vector of timestamps
vector<string> timestamps;
// For demo show
cv::Mat demo_frame;
// temporary coordinate
float tmpWR_JointPos[3];


bool initKinect();
bool initFrameTimeRelated();
void DrawLine(cv::Mat& rImg, const Joint& rJ1, const Joint& rJ2, ICoordinateMapper* pCMapper);
void DrawJoint(cv::Mat& rImg, const Joint& rJ1, ICoordinateMapper* pCMapper);
void writeInfo(int ID, ofstream& csvout, Joint *aJoints, ICoordinateMapper* pCMapper, char* time_str);

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

	// frame time file output stream
	ofstream csvfile_frame_time;
	csvfile_frame_time.open(csv_timePath);


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

			//Get Rgb data
			IColorFrame* colorframe;
			IColorFrameReference* Cframeref = NULL;
			frame->get_ColorFrameReference(&Cframeref);
			Cframeref->AcquireFrame(&colorframe);

			if (Cframeref) Cframeref->Release();
			if (!colorframe) continue;

			// Get data from frame
			if (colorframe->CopyConvertedFrameDataToArray(uColorBufferSize, pColorBuffer, ColorImageFormat_Bgra) != S_OK) {
				continue;
			}

			if (colorframe) colorframe->Release();

			mImg.data = pColorBuffer;

			//Get Body data
			IBodyFrame* pbodyframe;
			IBodyFrameReference* Bframeref = NULL;
			frame->get_BodyFrameReference(&Bframeref);
			Bframeref->AcquireFrame(&pbodyframe);
			if (Bframeref) Bframeref->Release();
			if (!pbodyframe) continue;

			// Get data from frame
			if (pbodyframe->GetAndRefreshBodyData(iBodyCount, aBodyData) != S_OK) {
				continue;
			}

			if (pbodyframe) pbodyframe->Release();
			frame->Release();

			// for each body
			GetLocalTime(&st);
			memset(time_str, '\0', 13);
			sprintf_s(time_str, 13, "%02d:%02d:%02d:%03d", st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);
			for (int i = 0; i < iBodyCount; ++i)
			{
				IBody* pBody = aBodyData[i];
				// check if is tracked
				BOOLEAN bTracked = false;
				if ((pBody->get_IsTracked(&bTracked) == S_OK) && bTracked)
				{
					// get joint position
					Joint aJoints[JointType::JointType_Count];
					if (pBody->GetJoints(JointType::JointType_Count, aJoints) == S_OK)
					{
						DrawLine(mImg, aJoints[JointType_SpineBase], aJoints[JointType_SpineMid], pCoordinateMapper);
						DrawLine(mImg, aJoints[JointType_SpineMid], aJoints[JointType_SpineShoulder], pCoordinateMapper);
						DrawLine(mImg, aJoints[JointType_SpineShoulder], aJoints[JointType_Neck], pCoordinateMapper);
						DrawLine(mImg, aJoints[JointType_Neck], aJoints[JointType_Head], pCoordinateMapper);

						DrawLine(mImg, aJoints[JointType_SpineShoulder], aJoints[JointType_ShoulderLeft], pCoordinateMapper);
						DrawLine(mImg, aJoints[JointType_ShoulderLeft], aJoints[JointType_ElbowLeft], pCoordinateMapper);
						DrawLine(mImg, aJoints[JointType_ElbowLeft], aJoints[JointType_WristLeft], pCoordinateMapper);
						DrawLine(mImg, aJoints[JointType_WristLeft], aJoints[JointType_HandLeft], pCoordinateMapper);
						DrawLine(mImg, aJoints[JointType_HandLeft], aJoints[JointType_HandTipLeft], pCoordinateMapper);
						DrawLine(mImg, aJoints[JointType_HandLeft], aJoints[JointType_ThumbLeft], pCoordinateMapper);

						DrawLine(mImg, aJoints[JointType_SpineShoulder], aJoints[JointType_ShoulderRight], pCoordinateMapper);
						DrawLine(mImg, aJoints[JointType_ShoulderRight], aJoints[JointType_ElbowRight], pCoordinateMapper);
						DrawLine(mImg, aJoints[JointType_ElbowRight], aJoints[JointType_WristRight], pCoordinateMapper);
						DrawLine(mImg, aJoints[JointType_WristRight], aJoints[JointType_HandRight], pCoordinateMapper);
						DrawLine(mImg, aJoints[JointType_HandRight], aJoints[JointType_HandTipRight], pCoordinateMapper);
						DrawLine(mImg, aJoints[JointType_HandRight], aJoints[JointType_ThumbRight], pCoordinateMapper);

						DrawLine(mImg, aJoints[JointType_SpineBase], aJoints[JointType_HipLeft], pCoordinateMapper);
						DrawLine(mImg, aJoints[JointType_HipLeft], aJoints[JointType_KneeLeft], pCoordinateMapper);
						DrawLine(mImg, aJoints[JointType_KneeLeft], aJoints[JointType_AnkleLeft], pCoordinateMapper);
						DrawLine(mImg, aJoints[JointType_AnkleLeft], aJoints[JointType_FootLeft], pCoordinateMapper);

						DrawLine(mImg, aJoints[JointType_SpineBase], aJoints[JointType_HipRight], pCoordinateMapper);
						DrawLine(mImg, aJoints[JointType_HipRight], aJoints[JointType_KneeRight], pCoordinateMapper);
						DrawLine(mImg, aJoints[JointType_KneeRight], aJoints[JointType_AnkleRight], pCoordinateMapper);
						DrawLine(mImg, aJoints[JointType_AnkleRight], aJoints[JointType_FootRight], pCoordinateMapper);

						for (int j = 0; j < JointType::JointType_Count; ++j) {
							DrawJoint(mImg, aJoints[j], pCoordinateMapper);
						}

						writeInfo(i, csvfile, aJoints, pCoordinateMapper, time_str);

					}
				}
			}
			

			//Demo video in this screen
			cv::resize(mImg, demo_frame, cv::Size(mImg.cols / mImg.cols * demoWidth, mImg.rows / mImg.rows * demoHeight), CV_INTER_LINEAR);
			cv::imshow("Recorder", demo_frame);
			if (cv::waitKey(1) == VK_ESCAPE) {
				break;
			}

			frames.push_back(demo_frame.clone());
			timestamps.push_back(string(time_str));

			//cout << count;
			// if get rgb/body frame
			count++;

			////Get Depth data
			//{
			//	IDepthFrame* depthframe;
			//	IDepthFrameReference* Dframeref = NULL;
			//	frame->get_DepthFrameReference(&Dframeref);
			//	Dframeref->AcquireFrame(&depthframe);
			//	if (Dframeref) Dframeref->Release();
			//	if (!depthframe) continue;

			//	// Get data from frame
			//	if (depthframe->CopyFrameDataToArray(uDepthPointNum, pDepthBuffer) != S_OK) {
			//		continue;
			//	}

			//	if (depthframe) depthframe->Release();
			//}
			////Get coordinate based on rgb frame
			//{
			//	if (mapper->MapColorFrameToCameraSpace(uDepthPointNum, pDepthBuffer, uColorPointNum, pCSPoints) != S_OK)
			//	{
			//		cerr << "Map to camera space error" << endl;
			//		continue;
			//	}

			//	//Do something...
			//}
		}
 //else
 //{
	// cout << "Reader N";
 //}
	}

	// save timestamps correspond with frames as csv
	for (int i = 0; i < timestamps.size(); i++) {
		csvfile_frame_time << i << "," << timestamps[i] << "\n";
	}

	// First Release ..
	csvfile_frame_time.close();
	csvfile.close();
	// Delete body data array
	delete[] aBodyData;
	// Release reader
	reader->Release();
	reader = nullptr;
	// Close Sensor
	cout << "Close sensor" << endl;
	sensor->Close();
	// Release Sensor
	cout << "Release sensor" << endl;
	sensor->Release();
	sensor = nullptr;


	// save frames as video
	cout << "Storing Video";
	cv::VideoWriter writer;
	writer.open(videoPath, CV_FOURCC('M', 'J', 'P', 'G'), 30, cv::Size(demoWidth, demoHeight), true);
	for (int i = 0; i < frames.size(); i++) {
		cv::cvtColor(frames[i], frames[i], cv::COLOR_BGRA2BGR);
		writer.write(frames[i]);
	}

	writer.release();
	//return 0;
}

bool initKinect() {
	if (FAILED(GetDefaultKinectSensor(&sensor))) {
		return false;
	}
	if (sensor) {
		sensor->get_CoordinateMapper(&mapper);

		sensor->Open();
		sensor->OpenMultiSourceFrameReader(
			FrameSourceTypes::FrameSourceTypes_Body |
			FrameSourceTypes::FrameSourceTypes_Color,
			// FrameSourceTypes::FrameSourceTypes_Depth,
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

	// Get Body frame source
	{
		IBodyFrameSource* pFrameSource = nullptr;
		if (sensor->get_BodyFrameSource(&pFrameSource) != S_OK)
		{
			cerr << "Can't get body frame source" << endl;
			return false;
		}

		// Get the number of body
		if (pFrameSource->get_BodyCount(&iBodyCount) != S_OK)
		{
			cerr << "Can't get body count" << endl;
			return -1;
		}

		cout << " > Can trace " << iBodyCount << " bodies" << endl;
		aBodyData = new IBody*[iBodyCount];
		for (int i = 0; i < iBodyCount; ++i)
			aBodyData[i] = nullptr;

	}

	
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

	// Get CoordinateMapper
	{
		if (sensor->get_CoordinateMapper(&pCoordinateMapper) != S_OK)
		{
			cout << "Can't get coordinate mapper" << endl;
			return false;
		}
	}

	return true;
	//// Get Depth frame source
	//{
	//	IDepthFrameSource* pFrameSource = nullptr;
	//	if (sensor->get_DepthFrameSource(&pFrameSource) != S_OK)
	//	{
	//		cerr << "Can't get depth frame source" << endl;
	//		return -1;
	//	}

	//	// Get frame description
	//	cout << "get depth frame description" << endl;
	//	IFrameDescription* pFrameDescription = nullptr;
	//	if (pFrameSource->get_FrameDescription(&pFrameDescription) == S_OK)
	//	{
	//		int	iDepthWidth = 0,
	//			iDepthHeight = 0;
	//		pFrameDescription->get_Width(&iDepthWidth);
	//		pFrameDescription->get_Height(&iDepthHeight);
	//		uDepthPointNum = iDepthWidth * iDepthHeight;
	//		pDepthBuffer = new UINT16[uDepthPointNum];
	//	}
	//	pFrameDescription->Release();
	//	pFrameDescription = nullptr;

	//	// release Frame source
	//	cout << "Release Depth frame source" << endl;
	//	pFrameSource->Release();
	//	pFrameSource = nullptr;
	//}
}

void DrawLine(cv::Mat& rImg, const Joint& rJ1, const Joint& rJ2, ICoordinateMapper* pCMapper)
{
	if (rJ1.TrackingState == TrackingState_NotTracked || rJ2.TrackingState == TrackingState_NotTracked)
		return;

	ColorSpacePoint ptJ1, ptJ2;
	pCMapper->MapCameraPointToColorSpace(rJ1.Position, &ptJ1);
	pCMapper->MapCameraPointToColorSpace(rJ2.Position, &ptJ2);

	if ((ptJ1.X >= 0 && ptJ1.X <= rImg.cols) && (ptJ2.X >= 0 && ptJ2.X <= rImg.cols) && (ptJ1.Y >= 0 && ptJ1.Y <= rImg.rows) && (ptJ2.Y >= 0 && ptJ2.Y <= rImg.rows)) {
		cv::line(rImg, cv::Point(ptJ1.X, ptJ1.Y), cv::Point(ptJ2.X, ptJ2.Y), cv::Vec3b(0, 255, 0), 5);
	}
}

void DrawJoint(cv::Mat& rImg, const Joint& rJ1, ICoordinateMapper* pCMapper)
{
	if (rJ1.TrackingState == TrackingState_NotTracked)
		return;

	ColorSpacePoint ptJ1;
	pCMapper->MapCameraPointToColorSpace(rJ1.Position, &ptJ1);

	if ((ptJ1.X >= 0 && ptJ1.X <= rImg.cols) && (ptJ1.Y >= 0 && ptJ1.Y <= rImg.rows)) {
		cv::circle(rImg, cv::Point(ptJ1.X, ptJ1.Y), 6, cv::Vec3b(0, 0, 255), -1);
	}
}

// write joint information to file in csv format
void writeInfo(int ID, ofstream& csvout, Joint *aJoints, ICoordinateMapper* pCMapper, char* time_str) {
	const Joint& WR_JointPos = aJoints[JointType::JointType_WristRight];
	const Joint& WL_JointPos = aJoints[JointType::JointType_WristLeft];
	const Joint& ER_JointPos = aJoints[JointType::JointType_ElbowRight];
	const Joint& HR_JointPos = aJoints[JointType::JointType_HandRight];
	const Joint& HTR_JointPos = aJoints[JointType::JointType_HandTipRight];
	const Joint& TR_JointPos = aJoints[JointType::JointType_ThumbRight];
	const Joint& HEAD_JointPos = aJoints[JointType::JointType_Head];

	ColorSpacePoint ptJ_Head;
	pCMapper->MapCameraPointToColorSpace(HEAD_JointPos.Position, &ptJ_Head);

	if (WR_JointPos.TrackingState == TrackingState_NotTracked) {
		csvout << tmpWR_JointPos[0] << "," << tmpWR_JointPos[1] << "," << tmpWR_JointPos[2] << ","
			<< tmpWR_JointPos[0] << "," << tmpWR_JointPos[1] << "," << tmpWR_JointPos[2] << ","
			<< tmpWR_JointPos[0] << "," << tmpWR_JointPos[1] << "," << tmpWR_JointPos[2] << ","
			<< tmpWR_JointPos[0] << "," << tmpWR_JointPos[1] << "," << tmpWR_JointPos[2] << ","
			<< tmpWR_JointPos[0] << "," << tmpWR_JointPos[1] << "," << tmpWR_JointPos[2] << ","
			<< tmpWR_JointPos[0] << "," << tmpWR_JointPos[1] << "," << tmpWR_JointPos[2] << ","
			<< ID << "," << ptJ_Head.X / 1920 * demoWidth << "," << ptJ_Head.Y / 1080 * demoHeight << "," << time_str << "\n";
	}
	else {
		tmpWR_JointPos[0] = WR_JointPos.Position.X;
		tmpWR_JointPos[1] = WR_JointPos.Position.Y;
		tmpWR_JointPos[2] = WR_JointPos.Position.Z;
		csvout << WR_JointPos.Position.X << "," << WR_JointPos.Position.Y << "," << WR_JointPos.Position.Z << ","
			<< WL_JointPos.Position.X << "," << WL_JointPos.Position.Y << "," << WL_JointPos.Position.Z << ","
			<< ER_JointPos.Position.X << "," << ER_JointPos.Position.Y << "," << ER_JointPos.Position.Z << ","
			<< HR_JointPos.Position.X << "," << HR_JointPos.Position.Y << "," << HR_JointPos.Position.Z << ","
			<< HTR_JointPos.Position.X << "," << HTR_JointPos.Position.Y << "," << HTR_JointPos.Position.Z << ","
			<< TR_JointPos.Position.X << "," << TR_JointPos.Position.Y << "," << TR_JointPos.Position.Z << ","
			<< ID << "," << ptJ_Head.X / 1920 * demoWidth << "," << ptJ_Head.Y / 1080 * demoHeight << "," << time_str << "\n";
	}
}