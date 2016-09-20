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

class playerDecoder
{
public:
	playerDecoder();
	playerDecoder(std::vector<std::string> stream_source, int width, int height);
	~playerDecoder();

	bool startCapture();
	bool Capture(int index);
	void stopCapture();

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

	

	bool isCurrent_ = true;
	int videoMode = 0; // video mode same to the ui part, if video mode  0 = left, 1 = right, 2 = stereo

	int camera_count_;
	int write_count_;
	int dst_width_, dst_height_;
	bool initialSuccess_;
};

