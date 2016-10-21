#pragma once

#include <opencv2\opencv.hpp>
#include <thread>
#include <concurrent_queue.h>

using namespace cv;
using namespace std;
using namespace concurrency;

//#define LEFTRIGHT 0
//#define UPDOWN 1

enum Mode { LEFTRIGHT, UPDOWN };

class captureVideo {

public:
	//constructor function and deconstructor function
	captureVideo(std::string urlL, Mode mode = LEFTRIGHT, bool stereo = true);
	captureVideo(string urlL, string urlR);
	captureVideo(string urlL, string urlR, int screenWidth, int screenHeight);
	captureVideo(vector<string> source, Mode mode = LEFTRIGHT, bool stereo = true);
	captureVideo(vector<int> index);
	~captureVideo();
	
	void startCapture(); //initialize and start the camera capturing process(es)
	void stopCapture(); //release all camera capture resource(s)
	void captureFrame(int index); //capture the frames from each stream with single video
	void captureStereoFrame(); //capture the frames from the stream with stereo video

	//void convert2render(int index);

	void setStereo(bool stereo);
	
	
	vector<string> camera_source; //holds camera stream urls
	vector<int> camera_index; //holds usb camera indices
	vector<VideoCapture*> camera_capture; //holds OpenCV VideoCapture pointers
	vector<concurrent_queue<unsigned char *>*> frame_ptr_; //the pointer to the frame;
	vector<concurrent_queue<Mat>*> frame_queue; //holds queue(s) which hold images from each camera
	vector<Mat> frames_; //holds the image frames from each camera 
	vector<thread*> camera_thread; //holds thread(s) which run the camera capture process
	vector<bool> writeState_;// the state to check is this image is writing now: true means it is writing, which can not use. false means it is not writing, which can be use

	Mode mode_ = LEFTRIGHT; // mode of stereo video
	int width_ = 1920, height_ = 1080; //the width and height of screen, for the full screen display

	bool isUSBCamera = false;
	bool realtime = false;
	bool stereo_ = true;
	int camera_count;

};