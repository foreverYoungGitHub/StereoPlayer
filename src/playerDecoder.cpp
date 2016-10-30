#include "playerDecoder.h"



playerDecoder::playerDecoder()
{
	dst_width_ = 1920;
	dst_height_ = 1080;
}

playerDecoder::playerDecoder(std::vector<std::string> stream_source, int width, int height)
{
	stream_info_ = stream_source;
	camera_source_ = { stream_source[0] };

	camera_count_ = camera_source_.size();
	dst_width_ = width;
	dst_height_ = height;
	decode_core_ = 1;
	thread_ = 0;
	av_register_all();
	avformat_network_init();
	//initialSuccess_ = startCapture_thread();
	initialSuccess_ = startCapture();
}

playerDecoder::~playerDecoder()
{
}

bool playerDecoder::startCapture()
{
	if (thread_ == 0)
	{
		if (decode_core_ == 0)
			return startCapture_ffmpeg();
		if (decode_core_ == 1)
			return startCapture_cv();
	}
	if (thread_ == 1)
	{
		if (decode_core_ == 0)
			return startCapture_ffmpeg_thread();
		if (decode_core_ == 1)
			return startCapture_cv_thread();
	}
	return false;
}

bool playerDecoder::startCapture_thread()
{
	if (decode_core_ == 0)
		return startCapture_ffmpeg_thread();
	if (decode_core_ == 1)
		return startCapture_cv_thread();
	return false;
}

bool playerDecoder::startCapture_ffmpeg()
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
		std::string url = camera_source_[i];

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
		if (!isCurrent_)
		{
			concurrency::concurrent_queue<AVFrame *> * q;
			FrameRGB_queue_.push_back(q);
		}
	}

	return true;
}

bool playerDecoder::startCapture_cv()
{
	cv::VideoCapture *capture;
	cv::Mat frame;
	bool writeState = true;

	for (int i = 0; i < camera_count_; i++)
	{
		
		//Make VideoCapture instance
		if (!isUSBCamera) {
			std::string url = camera_source_[i];
			capture = new cv::VideoCapture(url);
			std::cout << "Camera Setup: " << url << std::endl;
		}
		else {
			int idx = camera_index_[i];
			capture = new cv::VideoCapture(idx);
			std::cout << "Camera Setup: " << std::to_string(idx) << std::endl;
		}

		//Put VideoCapture to the vector
		camera_capture_.push_back(capture);

		//Put Mat to the vector
		frame_cv_.push_back(frame);
	}
	return true;
}


bool playerDecoder::startCapture_ffmpeg_thread()
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
		std::string url = camera_source_[i];

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
		if (!isCurrent_)
		{
			concurrency::concurrent_queue<AVFrame *> * q;
			FrameRGB_queue_.push_back(q);
		}

		//put the thread to vector
		t = new std::thread(&playerDecoder::Capture, this, i);
		camera_thread_.push_back(t);
	}

	return true;
}


bool playerDecoder::startCapture_cv_thread()
{
	cv::VideoCapture *capture;
	std::thread *t;
	concurrency::concurrent_queue<cv::Mat> *q;
	//concurrency::concurrent_queue<unsigned char *> *p;

	for (int i = 0; i < camera_count_; i++)
	{
		//Make VideoCapture instance
		if (!isUSBCamera) {
			std::string url = camera_source_[i];
			capture = new cv::VideoCapture(url);
			std::cout << "Camera Setup: " << url << std::endl;
		}
		else {
			int idx = camera_index_[i];
			capture = new cv::VideoCapture(idx);
			std::cout << "Camera Setup: " << std::to_string(idx) << std::endl;
		}

		//Put VideoCapture to the vector
		camera_capture_.push_back(capture);

		//Make a queue instance
		q = new concurrency::concurrent_queue<cv::Mat>;

		//Put queue to the vector
		frame_queue_.push_back(q);

		//Make thread instance
		if (camera_count_ == 1 && stereo_ == true)
		{
			//t = new std::thread(&playerDecoder::captureStereoFrame, this);
		}
		else
		{
			t = new std::thread(&playerDecoder::Capture_cv_thread, this, i);
			//t = new thread(&captureVideo::convert2render, this, i);
		}

		//Put thread to the vector
		camera_thread_.push_back(t);
	}
	return true;
}

bool playerDecoder::Capture(int index)
{
	if (thread_ == 0)
	{
		if (decode_core_ == 0)
			return Capture_ffmpeg(index);
		if (decode_core_ == 1)
			return Capture_cv(index);
	}
	/*
	opencv:since the capture part is already contain in a thread
	it will not be call in this part
	*/
	//if (decode_core_ == 1)
	//	return Capture_cv(index);
	return false;
}

bool playerDecoder::Capture_ffmpeg(int index)
{
	if (!(av_read_frame(FormatCtx_[index], packet_[index]) >= 0))
		return false;
	if (!(packet_[index]->stream_index == videoindex_[index]))
		return false;

	int got_picture;
	if (avcodec_decode_video2(CodecCtx_[index], Frame_[index], &got_picture, packet_[index]) < 0)
	{
		printf("Decode Error.\n");
		return -1;
	}
	if (!got_picture)
	{
		return -1;
	}

	//sws_scale(img_convert_ctx_[index], Frame_[index]->data, Frame_[index]->linesize, 0, CodecCtx_[index]->height, FrameRGB_current_[index]->data, FrameRGB_current_[index]->linesize);
	return true;
}

bool playerDecoder::Capture_ffmpeg_thread(int index)
{
	AVFrame * frame;
	while (true)
	{
		frame = av_frame_alloc();
		if (!(av_read_frame(FormatCtx_[index], packet_[index]) >= 0))
			continue;
		if (!(packet_[index]->stream_index == videoindex_[index]))
			continue;

		int got_picture;
		if (avcodec_decode_video2(CodecCtx_[index], frame, &got_picture, packet_[index]) < 0)
		{
			printf("Decode Error.\n");
			continue;
		}
		if (!got_picture)
		{
			continue;
		}
		
		FrameRGB_queue_[index]->push(frame);
	}
	return true;
}

bool playerDecoder::Capture_cv(int index)
{
	cv::VideoCapture *capture = camera_capture_[index];
	cv::Mat frame;

	//Grab frame from camera capture
	(*capture) >> frame;

	if (frame.empty())
		return false;

	//convert color which can display in DirectX;
	cv::cvtColor(frame, frame, cv::COLOR_BGR2BGRA);

	//resize it to suitable size
	if (!(frame.rows == dst_height_ && frame.cols == dst_width_))
		cv::resize(frame, frame, cv::Size(dst_width_, dst_height_));

	//put it into the 
	frame_cv_[index] = frame;

	//relase frame resource
	frame.release();
	return true;
}

bool playerDecoder::Capture_cv_thread(int index)
{
	cv::VideoCapture *capture = camera_capture_[index];
	cv::Mat frame;
	while (true)
	{
		//Grab frame from camera capture
		(*capture) >> frame;
		if (!frame.empty())
		{
			cv::cvtColor(frame, frame, cv::COLOR_BGR2BGRA);
			//resize it to suitable size
			if (!(frame.rows == dst_height_ && frame.cols == dst_width_))
				cv::resize(frame, frame, cv::Size(dst_width_, dst_height_));
			//Put frame to the queue
			frame_queue_[index]->push(frame);
			while (frame_queue_[index]->unsafe_size() > 5)
			{
				frame_queue_[index]->try_pop(frame);
			}
		}
	}
	//relase frame resource
	frame.release();
	return true;
}

void playerDecoder::stopCapture()
{
	if (decode_core_ == 0)
		return stopCapture_ffmpeg();
	if (decode_core_ == 1)
		return stopCapture_cv();
	return;
}

void playerDecoder::stopCapture_ffmpeg()
{
	if (camera_source_.size() == 0)
		return;
	camera_source_.clear();
	for (int i = 0; i < camera_count_; i++)
	{
		sws_freeContext(img_convert_ctx_[i]);

		av_frame_free(&FrameRGB_current_[i]);
		av_frame_free(&Frame_[i]);
		avcodec_close(CodecCtx_[i]);
		avformat_close_input(&FormatCtx_[i]);
	}
}

void playerDecoder::stopCapture_cv()
{
	cv::VideoCapture *cap;
	for (int i = 0; i < camera_count_; i++)
	{
		cap = camera_capture_[i];
		if (cap->isOpened()) {
			//Relase VideoCapture resource
			cap->release();
			std::cout << "Capture " << i << " released" << std::endl;
		}
	}
}

void playerDecoder::Decode(unsigned char * imagedata, int index)
{
	if (thread_ == 0)
	{
		if (decode_core_ == 0)
		{
			Decode_ffmpeg(imagedata, index);
			return;
		}
		if (decode_core_ == 1)
		{
			Decode_cv(imagedata, index);
			return;
		}
	}
	if (thread_ == 1)
	{
		if (decode_core_ == 0)
		{
			Decode_ffmpeg_thread(imagedata, index);
			return;
		}
		if (decode_core_ == 1)
		{
			Decode_cv_thread(imagedata, index);
			return;
		}
	}
}

void playerDecoder::Decode_ffmpeg(unsigned char * imagedata, int index)
{
	//TO DO
	return;
}

void playerDecoder::Decode_cv(unsigned char * imagedata, int index)
{
	if (!Capture(index))
		return;

	cv::Mat frame;
	frame = frame_cv_[index];

	if (frame.empty() || frame.at<uchar>(0, 0) == NULL) //make sure it works well
		return;

	int linesize = frame.cols * 4;
	//imagedata = (unsigned char *)malloc(linesize * frame.rows);

	for (int j = 0; j < frame.rows; j++)
	{
		uchar* srcData = frame.ptr<uchar>(j);
		if (srcData != NULL)
		{
			memcpy(imagedata + j * linesize, srcData, linesize);
		}
	}
	return;
}

void playerDecoder::Decode_ffmpeg_thread(unsigned char * imagedata, int index)
{
	//TO DO
}

void playerDecoder::Decode_cv_thread(unsigned char * imagedata, int index)
{
	//unsigned char * imagedata;
	if (frame_queue_[index]->unsafe_size() == 0)
		return;

	cv::Mat frame;
	frame_queue_[index]->try_pop(frame);

	if (frame.empty() || frame.at<uchar>(0, 0) == NULL) //make sure it works well
		return;

	int linesize = frame.cols * 4;
	//imagedata = (unsigned char *)malloc(linesize * frame.rows);

	for (int j = 0; j < frame.rows; j++)
	{
		uchar* srcData = frame.ptr<uchar>(j);
		if (srcData != NULL)
		{
			memcpy(imagedata + j * linesize, srcData, linesize);
		}
	}
	return;
}

void playerDecoder::Decode(int index, D3DLOCKED_RECT lockedrect)
{
	if (thread_ == 0)
	{
		if (decode_core_ == 0)
			return Decode_ffmpeg(index, lockedrect);
		if (decode_core_ == 1)
			return Decode_cv(index, lockedrect);
	}
	if (thread_ == 1)
	{
		if (decode_core_ == 0)
			return Decode_ffmpeg_thread(index, lockedrect);
		if (decode_core_ == 1)
			return Decode_cv_thread(index, lockedrect);
	}
	return;
}

void playerDecoder::Decode_ffmpeg(int index, D3DLOCKED_RECT lockedrect)
{
	if (!Capture(index))
		return;

	//this code is replaced by the above code to reduce time 
	BYTE* imagedata = (BYTE *)lockedrect.pBits;
	avpicture_fill((AVPicture *)FrameRGB_current_[index], imagedata, AV_PIX_FMT_RGB32, dst_width_, dst_height_);
	sws_scale(img_convert_ctx_[index], Frame_[index]->data, Frame_[index]->linesize, 0, CodecCtx_[index]->height, FrameRGB_current_[index]->data, FrameRGB_current_[index]->linesize);

}

void playerDecoder::Decode_cv(int index, D3DLOCKED_RECT lockedrect)
{
	if (!Capture(index))
		return;

	cv::Mat frame;
	frame = frame_cv_[index];

	if (frame.empty() || frame.at<uchar>(0, 0) == NULL) //make sure it works well
		return;

	BYTE * imagedata = (BYTE *)lockedrect.pBits;

	//this code is replaced by the above code to reduce time 
	for (int j = 0; j < dst_height_; j++)
	{
		uchar* srcData = frame.ptr<uchar>(j);
		if (srcData != NULL)
			memcpy(imagedata + j * lockedrect.Pitch, srcData, dst_width_ * 4);
	}
}

void playerDecoder::Decode_ffmpeg_thread(int index, D3DLOCKED_RECT lockedrect)
{
	//TO DO
}

void playerDecoder::Decode_cv_thread(int index, D3DLOCKED_RECT lockedrect)
{
	if (frame_queue_[index]->unsafe_size() == 0)
		return;

	cv::Mat frame;
	frame_queue_[index]->try_pop(frame);

	if (frame.empty() || frame.at<uchar>(0, 0) == NULL) //make sure it works well
		return;

	BYTE * imagedata = (BYTE *)lockedrect.pBits;

	//this code is replaced by the above code to reduce time 
	for (int j = 0; j < dst_height_; j++)
	{
		uchar* srcData = frame.ptr<uchar>(j);
		if (srcData != NULL)
			memcpy(imagedata + j * lockedrect.Pitch, srcData, dst_width_ * 4);
	}
}


bool playerDecoder::InitRemuxer()
{
	std::string left = "C:\\3DPlayerSavedVideo\\" + stream_info_[2] + "_" + stream_info_[3] + "_left_" + ".mp4";
	write_name_.push_back(left);
	std::string right = "C:\\3DPlayerSavedVideo\\" + stream_info_[2] + "_" + stream_info_[3] + "_right_" + ".mp4";
	write_name_.push_back(right);

	write_count_ = write_name_.size();

	return startRemuxer();
	
}

bool playerDecoder::startRemuxer()
{

	for (int i = 0; i < write_count_; i++)
	{
		AVOutputFormat * ofmt = NULL;
		AVFormatContext *ifmt_ctx = NULL, *ofmt_ctx = NULL;
		std::string in_filename = camera_source_[i];
		std::string out_filename = write_name_[i];

		//input
		if (avformat_open_input(&ifmt_ctx, in_filename.c_str(), 0, 0) < 0)
		{
			return -1;
		}
		if (avformat_find_stream_info(ifmt_ctx, 0) < 0)
		{
			return -1;
		}
		//push ifmt_ctx to the vector write_in_FormatCtx_
		write_in_FormatCtx_.push_back(ifmt_ctx);

		//output
		avformat_alloc_output_context2(&ofmt_ctx, NULL, NULL, out_filename.c_str());
		if (!ofmt_ctx)
		{
			printf("Could not create output context\n");
			return -1;
		}
		//push the ofmt_ctx to the vector write_FormatCtx_
		write_out_FormatCtx_.push_back(ofmt_ctx);

		ofmt = ofmt_ctx->oformat;
		for (int i = 0; i < ifmt_ctx->nb_streams; i++) 
		{
			AVStream *in_stream = ifmt_ctx->streams[i];
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
		//push the ofmt to the vector write_Format_
		write_out_Format_.push_back(ofmt);

		//open output file
		if (!(ofmt->flags & AVFMT_NOFILE))
		{
			if (avio_open(&ofmt_ctx->pb, out_filename.c_str(), AVIO_FLAG_WRITE))
			{
				printf("Could not open output URL '%s'", out_filename);
				return -1;
			}
		}

		//write file header
		if (avformat_write_header(ofmt_ctx, NULL) < 0) 
		{
			printf("Error occurred when opening output URL\n");
			return -1;
		}
	}
	
}

bool playerDecoder::Remuxer(int index)
{
	AVStream * in_stream, *out_stream;
	AVPacket pkt;
	if (av_read_frame(write_in_FormatCtx_[index], &pkt))
		return -1;
	in_stream = write_in_FormatCtx_[index]->streams[pkt.stream_index];
	out_stream = write_out_FormatCtx_[index]->streams[pkt.stream_index];

	pkt.pts = av_rescale_q_rnd(pkt.pts, in_stream->time_base, out_stream->time_base, (AVRounding)(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
	pkt.dts = av_rescale_q_rnd(pkt.dts, in_stream->time_base, out_stream->time_base, (AVRounding)(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
	pkt.duration = av_rescale_q(pkt.duration, in_stream->time_base, out_stream->time_base);
	pkt.pos = -1;

	if (av_interleaved_write_frame(write_out_FormatCtx_[index], &pkt) < 0)
	{
		return -1;
	}
	av_free_packet(&pkt);
	return true;
}

void playerDecoder::stopRemuxer()
{
	if (write_name_.size() == 0)
		return;
	write_name_.clear();
	for (int i = 0; i < write_count_; i++)
	{
		av_write_trailer(write_out_FormatCtx_[i]);
		if (write_out_FormatCtx_[i] && !(write_out_Format_[i]->flags & AVFMT_NOFILE))
			avio_close(write_out_FormatCtx_[i]->pb);
		avformat_free_context(write_out_FormatCtx_[i]);
	}

	//write the file information for the CSV, which can be found in the file list in ui.
	writeCSV();
}

bool playerDecoder::writeCSV()
{
	std::string fileName = "C:\\3DPlayerSavedVideo\\index.csv";
	std::fstream file(fileName, std::ios::app);

	if (!file)
		return -1;
	
	if (write_name_.size() == 0)
		return -1;

	file << "\n" << write_name_[0] << "," << write_name_[1] << "," << stream_info_[2] << "," << stream_info_[3] ;

	file.close();
}