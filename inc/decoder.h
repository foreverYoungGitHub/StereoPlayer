#pragma once
extern "C"
{
#include "libavcodec/avcodec.h"  
#include "libavformat/avformat.h"  
#include "libswscale/swscale.h"  
#include "libavutil/imgutils.h" 
}

#include <string>
#include <vector>
#include <thread>
#include <fstream>
#include <opencv2\opencv.hpp>
#include <d3d9.h>
#include <d3dx9.h>
#include <concurrent_queue.h>
#include <mutex>

class Decoder {
public:
	Decoder();

	~Decoder();

	int Init();
	int Init_ffmpeg();
	int Init_ffmpeg_thread();
	int Init_cv();
	int Init_cv_thread();


	int update_window_status();

	//int Demux();
	int Demux(int index);

	int Decode();

private:

	//common variable copy from global
	std::vector<std::string> input_address_;
	std::vector<std::string> output_address_;
	int camera_count_;
	bool write_file_;
	std::vector<std::thread *> camera_thread_; //holds thread(s) which run the camera capture process
	int dst_width_, dst_height_;
	int decode_core_; //the decoder core have two choice: ffmpeg (0) and opencv (1)
	int thread_;
	int video_mode_;


	//the variable used for opencv core
	std::vector<int> camera_index_; //holds usb camera indices
	std::vector<cv::VideoCapture*> camera_capture_; //holds OpenCV VideoCapture pointers
	std::vector<concurrency::concurrent_queue<cv::Mat>*> frame_queue_; //holds queue(s) which hold images from each camera
	std::vector<cv::Mat> frame_cv_;

	//the variable used for ffmpeg core
	std::vector<AVFormatContext *> FormatCtx_, out_FormatCtx_;
	std::vector<AVOutputFormat *> out_Format_;
	//std::vector<AVFormatContext *> write_in_FormatCtx_, write_out_FormatCtx_;
	std::vector<AVCodecContext *> CodecCtx_;
	std::vector<AVCodec *> Codec_;
	std::vector<AVPacket *> packet_;
	std::vector<AVFrame *> Frame_;
	std::vector<concurrency::concurrent_queue<AVFrame *> *> FrameRGB_queue_;
	std::vector<AVFrame *> FrameRGB_current_;
	std::vector<struct SwsContext *> img_convert_ctx_;
	std::vector<int> videoindex_;
};

extern Decoder * g_decoder;