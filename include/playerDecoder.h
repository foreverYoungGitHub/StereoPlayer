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
#include <d3d9.h>
#include <d3dx9.h>
#include <mutex>

class playerDecoder
{
public:
	playerDecoder();
	playerDecoder(std::vector<std::string> stream_source, int width, int height);
	~playerDecoder();

	
	
	bool startCapture();
	bool startCapture_thread();
	bool startCapture_cv();
	bool startCapture_cv_thread();
	bool startCapture_ffmpeg();
	bool startCapture_ffmpeg_thread();

	bool Capture(int index);
	bool Capture_cv(int index);
	bool Capture_cv_thread(int index);
	bool Capture_ffmpeg(int index);	
	bool Capture_ffmpeg_thread(int index);

	void stopCapture();
	void stopCapture_cv();
	void stopCapture_ffmpeg();
	

	void Decode(unsigned char * imagedata, int index);
	void Decode_cv(unsigned char * imagedata, int index);
	void Decode_ffmpeg(unsigned char * imagedata, int index);
	void Decode_cv_thread(unsigned char * imagedata, int index);
	void Decode_ffmpeg_thread(unsigned char * imagedata, int index);

	void Decode(int index, D3DLOCKED_RECT lockedrect);
	void Decode_cv(int index, D3DLOCKED_RECT lockedrect);
	void Decode_ffmpeg(int index, D3DLOCKED_RECT lockedrect);
	void Decode_cv_thread(int index, D3DLOCKED_RECT lockedrect);
	void Decode_ffmpeg_thread(int index, D3DLOCKED_RECT lockedrect);

	bool InitRemuxer();
	bool startRemuxer();
	bool Remuxer(int index);
	void stopRemuxer();

	bool writeCSV();

	//common variable
	std::vector<std::thread *> camera_thread_; //holds thread(s) which run the camera capture process
	std::vector<std::string> stream_info_;
	std::vector<std::string> camera_source_;
	std::vector<std::string> write_name_;
	int decode_core_ = 1; //the decoder core have two choice: ffmpeg (0) and opencv (1)
	int thread_ = 1;

	//the variable used for ffmpeg core
	std::vector<AVFormatContext *> FormatCtx_;
	std::vector<AVOutputFormat *> write_out_Format_;
	std::vector<AVFormatContext *> write_in_FormatCtx_, write_out_FormatCtx_;
	std::vector<AVCodecContext *> CodecCtx_;
	std::vector<AVCodec *> Codec_;
	std::vector<AVPacket *> packet_;
	std::vector<AVFrame *> Frame_;
	std::vector<concurrency::concurrent_queue<AVFrame *> *> FrameRGB_queue_;
	std::vector<AVFrame *> FrameRGB_current_;
	
	std::vector<bool> decodeState_;
	std::vector<struct SwsContext *> img_convert_ctx_;
	std::vector<int> videoindex_;
	std::vector<concurrency::concurrent_queue<unsigned char *>*> frame_ptr_; //the pointer to the frame;
	

	//bool isCurrent_ = true;
	int videoMode = 0; // video mode same to the ui part, if video mode  0 = left, 1 = right, 2 = stereo

	int camera_count_;
	int write_count_;
	int dst_width_, dst_height_;
	bool initialSuccess_;

	//the variable used for opencv core
	std::vector<int> camera_index_; //holds usb camera indices
	std::vector<cv::VideoCapture*> camera_capture_; //holds OpenCV VideoCapture pointers
	std::vector<concurrency::concurrent_queue<cv::Mat>*> frame_queue_; //holds queue(s) which hold images from each camera
	std::vector<cv::Mat> frame_cv_;

	bool isUSBCamera = false;
	bool realtime = false;
	bool stereo_ = false;

	std::mutex mtx_;

};

