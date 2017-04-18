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

struct DecodeStatus;

class Decoder {
public:
	Decoder();

	Decoder(DecodeStatus *);

	~Decoder();

	int Decode(int index, D3DLOCKED_RECT lockedrect);

private:
	int Init();
	int Init_ffmpeg();
	int Init_ffmpeg_thread();
	int Init_ffmpeg_audio_thread();
	int Init_cv();
	int Init_cv_thread();
	
	int Capture(int i);
	int Capture_ffmpeg(int index);
	int Capture_ffmpeg_thread(int index);
	int Capture_ffmpeg_audio_thread();
	int Capture_cv(int index);
	int Capture_cv_thread(int index);
	
	int stopCapture();
	int stopCapture_cv();
	int stopCapture_ffmpeg();
	int stopCapture_ffmpeg_audio();

	int Decode_cv(int index, D3DLOCKED_RECT lockedrect);
	int Decode_ffmpeg(int index, D3DLOCKED_RECT lockedrect);
	int Decode_cv_thread(int index, D3DLOCKED_RECT lockedrect);
	int Decode_ffmpeg_thread(int index, D3DLOCKED_RECT lockedrect);

	int InitRemux(int index);
	int InitRemux_ffmpeg(int index);
	int InitRemux_cv(int index);

	int Remux(int index);
	int Remux_ffmpeg(int index);
	int Remux_cv(int index);

	int stopRemux();
	int stopRemux_ffmpeg();
	int stopRemux_cv();
	

	//common variable copy from global
	DecodeStatus * decode_status_;
	std::vector<std::thread *> camera_thread_; //holds thread(s) which run the camera capture process
	bool hold_thread_;
	std::mutex mtx_;

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
	std::vector<concurrency::concurrent_queue<AVPacket *> *> packet_queue_;
	std::vector<AVFrame *> Frame_;
	std::vector<concurrency::concurrent_queue<AVFrame *> *> FrameRGB_queue_;
	std::vector<AVFrame *> FrameRGB_current_;
	std::vector<struct SwsContext *> img_convert_ctx_;
	std::vector<int> videoindex_;
	std::vector<int> stream_mapping_size_;
	std::vector<int *> stream_mapping_;

	//variable for audio
	AVFormatContext * FormatCtx_audio_;
	AVCodecContext * CodecCtx_audio_;
	AVFrame * Frame_audio_;
	int audio_index_;
	struct SwrContext * convert_ctx_audio_;
	uint8_t * out_buffer_audio_;
	int out_buffer_size_audio_;
	std::thread * audio_thread_;
};