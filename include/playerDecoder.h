#pragma once
extern "C"
{
#include "libavcodec/avcodec.h"  
#include "libavformat/avformat.h"  
#include "libswscale/swscale.h"  
#include "libavutil/imgutils.h" 
}
#include <string>
#include <thread>
#include <vector>
#include <concurrent_queue.h>
#include <fstream>
#include <opencv2\opencv.hpp>

class playerDecoder
{
public:
	playerDecoder();
	playerDecoder(std::vector<std::string> stream_source, int width, int height);
	~playerDecoder();

	
	
	bool startCapture();
	bool startCapture_cv();
	bool startCapture_ffmpeg();

	bool Capture(int index);
	bool Capture_cv(int index);
	bool Capture_ffmpeg(int index);	

	void stopCapture();
	void stopCapture_cv();
	void stopCapture_ffmpeg();

	void Decode();

	bool InitRemuxer();
	bool startRemuxer();
	bool Remuxer(int index);
	void stopRemuxer();

	bool writeCSV();

	std::vector<std::string> stream_info_;
	std::vector<std::string> camera_source_;
	std::vector<std::string> write_name_;
	std::vector<AVFormatContext *> FormatCtx_;
	std::vector<AVOutputFormat *> write_out_Format_;
	std::vector<AVFormatContext *> write_in_FormatCtx_, write_out_FormatCtx_;
	std::vector<AVCodecContext *> CodecCtx_;
	std::vector<AVCodec *> Codec_;
	std::vector<AVPacket *> packet_;
	std::vector<AVFrame *> Frame_;
	std::vector<concurrency::concurrent_queue<AVFrame *> *> FrameRGB_queue_;
	std::vector<AVFrame *> FrameRGB_current_;
	std::vector<std::thread *> camera_thread_;
	std::vector<bool> decodeState_;
	std::vector<struct SwsContext *> img_convert_ctx_;
	std::vector<int> videoindex_;
	std::vector<concurrency::concurrent_queue<unsigned char *>*> frame_ptr_; //the pointer to the frame;
	

	bool isCurrent_ = true;
	int videoMode = 0; // video mode same to the ui part, if video mode  0 = left, 1 = right, 2 = stereo

	int camera_count_;
	int write_count_;
	int dst_width_, dst_height_;
	bool initialSuccess_;

	//the variable used for opencv core
	std::vector<int> camera_index_; //holds usb camera indices
	std::vector<cv::VideoCapture*> camera_capture_; //holds OpenCV VideoCapture pointers
	std::vector<concurrency::concurrent_queue<cv::Mat>*> frame_queue; //holds queue(s) which hold images from each camera
	std::vector<std::thread*> camera_thread; //holds thread(s) which run the camera capture process

	bool isUSBCamera = false;
	bool realtime = false;
	bool stereo_ = true;

	int decode_core_ = 0; //the decoder core have two choice: ffmpeg (0) and opencv (1)

};

