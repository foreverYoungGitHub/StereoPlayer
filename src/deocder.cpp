#include "../inc/decoder.h"
#include "../inc/controller.h"


Decoder * g_decoder;

Decoder::Decoder() {

	input_address_ = g_decode_status.input_address;
	camera_count_ = input_address_.size();
	dst_width_ = g_window_size.width;
	dst_height_ = g_window_size.height;
	decode_core_ = g_decode_status.decode_core;
	thread_ = g_decode_status.multi_thread;
	video_mode_ = g_decode_status.video_mode;
	write_file_ = g_decode_status.write_file;

	if (write_file_ == true)
	{
		output_address_ = g_decode_status.output_address;
	}

	g_decode_status.visual_status = Init();
}

Decoder::~Decoder() {

}

int Decoder::Init() {
	if (thread_ == 0)
	{
		if (decode_core_ == 0)
			return Init_ffmpeg();
		if (decode_core_ == 1)
			return Init_cv();
	}
	if (thread_ == 1)
	{
		if (decode_core_ == 0)
			return Init_ffmpeg_thread();
		if (decode_core_ == 1)
			return Init_cv_thread();
	}
	return false;
}

int Decoder::Init_ffmpeg()
{
	//AVFormatContext *pFormatCtx;
	int				 videoindex;
	AVCodecContext  *pCodecCtx;
	AVCodec         *pCodec;
	AVFrame			*pFrame, *pFrameRGB;
	//AVPacket		*packet;
	struct SwsContext *img_convert_ctx;

	av_register_all();
	avformat_network_init();

	for (int i = 0; i < camera_count_; i++)
	{
		std::string url = input_address_[i];

		//put the pFormatCtx to vector
		AVFormatContext * pFormatCtx = avformat_alloc_context();

		if (avformat_open_input(&pFormatCtx, url.c_str(), NULL, NULL) != 0)
		{
			printf("Couldn't open input stream.\n");
			return -1;
		}
		if (avformat_find_stream_info(pFormatCtx, NULL) < 0)
		{
			printf("Couldn't find stream information.\n");
			return -1;
		}
		FormatCtx_.push_back(pFormatCtx);

		//TO DO :: add the init of remuxer there
		{
			//output
			AVFormatContext *ofmt_ctx = NULL;
			avformat_alloc_output_context2(&ofmt_ctx, NULL, NULL, output_address_[i].c_str());
			if (!ofmt_ctx)
			{
				printf("Could not create output context\n");
				return -1;
			}
			//push the ofmt_ctx to the vector write_FormatCtx_
			out_FormatCtx_.push_back(ofmt_ctx);

			AVOutputFormat * ofmt = NULL;
			ofmt = ofmt_ctx->oformat;
			for (int i = 0; i < pFormatCtx->nb_streams; i++)
			{
				AVStream *in_stream = pFormatCtx->streams[i];
				AVStream *out_stream = avformat_new_stream(ofmt_ctx, in_stream->codec->codec);
				if (!out_stream)
				{
					printf("Failed allocating output stream\n");
					return -1;
				}
				//copy the setting of avcodecco
				if (avcodec_copy_context(out_stream->codec, in_stream->codec) < 0)
				{
					printf("Failed to copy context from input to output stream codec context\n");
					return -1;
				}
				out_stream->codec->codec_tag = 0;
				if (ofmt_ctx->oformat->flags & AVFMT_GLOBALHEADER)
					out_stream->codec->flags |= CODEC_FLAG_GLOBAL_HEADER;
			}

			//open output file
			if (!(ofmt->flags & AVFMT_NOFILE))
			{
				if (avio_open(&ofmt_ctx->pb, output_address_[i].c_str(), AVIO_FLAG_WRITE))
				{
					printf("Could not open output URL '%s'", output_address_[i]);
					return -1;
				}
			}

			//write file header
			if (avformat_write_header(ofmt_ctx, NULL) < 0)
			{
				printf("Error occurred when opening output URL\n");
				return -1;
			}

			//push the ofmt to the vector write_Format_
			out_Format_.push_back(ofmt);

		}

		//put the videoindex to vector
		videoindex = -1;
		for (int i = 0; i < pFormatCtx->nb_streams; i++)
		{
			if (pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO)
			{
				videoindex = i;
				break;
			}
		}
		if (videoindex == -1)
		{
			printf("Didn't find a video stream.\n");
			return -1;
		}
		videoindex_.push_back(videoindex);

		//put the pCodecCtx and pCodec to vector
		AVCodecContext *pCodecCtx = pFormatCtx->streams[videoindex]->codec;
		AVCodec *pCodec = avcodec_find_decoder(pCodecCtx->codec_id);
		if (pCodec == NULL) {
			printf("Codec not found.\n");
			return -1;
		}
		if (avcodec_open2(pCodecCtx, pCodec, NULL)<0) {
			printf("Could not open codec.\n");
			return -1;
		}
		CodecCtx_.push_back(pCodecCtx);
		Codec_.push_back(pCodec);

		//put the pFrame and pFrameRGB to vector
		pFrame = av_frame_alloc();
		pFrameRGB = av_frame_alloc();
		unsigned char * out_buffer = (unsigned char *)av_malloc(av_image_get_buffer_size(AV_PIX_FMT_RGB32, dst_width_, dst_height_, 1));
		av_image_fill_arrays(pFrameRGB->data, pFrameRGB->linesize, out_buffer, AV_PIX_FMT_RGB32, dst_width_, dst_height_, 1);
		Frame_.push_back(pFrame);
		FrameRGB_current_.push_back(pFrameRGB);

		//put the packet to vector
		AVPacket * packet = (AVPacket *)av_malloc(sizeof(AVPacket));
		packet_.push_back(packet);

		//put the packet to vector
		img_convert_ctx = sws_getContext(pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt, dst_width_, dst_height_, AV_PIX_FMT_RGB32, SWS_BICUBIC, NULL, NULL, NULL);
		img_convert_ctx_.push_back(img_convert_ctx);
	}

	return true;
}

int Decoder::Init_cv()
{
	cv::VideoCapture *capture;
	cv::Mat frame;
	bool writeState = true;

	for (int i = 0; i < camera_count_; i++)
	{
		//Make VideoCapture instance
		std::string url = input_address_[i];
		capture = new cv::VideoCapture(url);
		std::cout << "Camera Setup: " << url << std::endl;

		//Put VideoCapture to the vector
		camera_capture_.push_back(capture);

		//Put Mat to the vector
		frame_cv_.push_back(frame);
	}
	return true;
}


int Decoder::Init_ffmpeg_thread()
{
	//AVFormatContext *pFormatCtx;
	int				 videoindex;
	AVCodecContext  *pCodecCtx;
	AVCodec         *pCodec;
	AVFrame			*pFrame, *pFrameRGB;
	//AVPacket		*packet;
	struct SwsContext *img_convert_ctx;

	std::thread * t;

	for (int i = 0; i < camera_count_; i++)
	{
		std::string url = input_address_[i];

		//put the pFormatCtx to vector
		AVFormatContext * pFormatCtx = avformat_alloc_context();

		if (avformat_open_input(&pFormatCtx, url.c_str(), NULL, NULL) != 0)
		{
			printf("Couldn't open input stream.\n");
			return -1;
		}
		if (avformat_find_stream_info(pFormatCtx, NULL) < 0)
		{
			printf("Couldn't find stream information.\n");
			return -1;
		}
		FormatCtx_.push_back(pFormatCtx);

		//put the videoindex to vector
		videoindex = -1;
		for (int i = 0; i < pFormatCtx->nb_streams; i++)
		{
			if (pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO)
			{
				videoindex = i;
				break;
			}
		}
		if (videoindex == -1)
		{
			printf("Didn't find a video stream.\n");
			return -1;
		}
		videoindex_.push_back(videoindex);

		//put the pCodecCtx and pCodec to vector
		AVCodecContext *pCodecCtx = pFormatCtx->streams[videoindex]->codec;
		AVCodec *pCodec = avcodec_find_decoder(pCodecCtx->codec_id);
		if (pCodec == NULL) {
			printf("Codec not found.\n");
			return -1;
		}
		if (avcodec_open2(pCodecCtx, pCodec, NULL)<0) {
			printf("Could not open codec.\n");
			return -1;
		}
		CodecCtx_.push_back(pCodecCtx);
		Codec_.push_back(pCodec);

		//put the pFrame and pFrameRGB to vector
		pFrame = av_frame_alloc();
		pFrameRGB = av_frame_alloc();
		unsigned char * out_buffer = (unsigned char *)av_malloc(av_image_get_buffer_size(AV_PIX_FMT_RGB32, dst_width_, dst_height_, 1));
		av_image_fill_arrays(pFrameRGB->data, pFrameRGB->linesize, out_buffer, AV_PIX_FMT_RGB32, dst_width_, dst_height_, 1);
		Frame_.push_back(pFrame);
		FrameRGB_current_.push_back(pFrameRGB);

		//put the packet to vector
		AVPacket * packet = (AVPacket *)av_malloc(sizeof(AVPacket));
		packet_.push_back(packet);

		//put the packet to vector
		img_convert_ctx = sws_getContext(pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt, dst_width_, dst_height_, AV_PIX_FMT_RGB32, SWS_BICUBIC, NULL, NULL, NULL);
		img_convert_ctx_.push_back(img_convert_ctx);

		//put the queue to vector
		concurrency::concurrent_queue<AVFrame *> * q;
		q = new concurrency::concurrent_queue<AVFrame *>;
		FrameRGB_queue_.push_back(q);

		//put the thread to vector
		t = new std::thread(&Decoder::Demux, this, i);
		camera_thread_.push_back(t);
	}

	return true;
}


int Decoder::Init_cv_thread()
{
	cv::VideoCapture *capture;
	std::thread *t;
	concurrency::concurrent_queue<cv::Mat> *q;
	//concurrency::concurrent_queue<unsigned char *> *p;

	for (int i = 0; i < camera_count_; i++)
	{
		//Make VideoCapture instance
		std::string url = input_address_[i];
		capture = new cv::VideoCapture(url);
		std::cout << "Camera Setup: " << url << std::endl;


		//Put VideoCapture to the vector
		camera_capture_.push_back(capture);

		//Make a queue instance
		q = new concurrency::concurrent_queue<cv::Mat>;

		//Put queue to the vector
		frame_queue_.push_back(q);

		//Make thread instance
		t = new std::thread(&Decoder::Demux, this, i);

		//Put thread to the vector
		camera_thread_.push_back(t);
	}
	return true;
}

int Decoder::Demux(int index) {
	return true;
}