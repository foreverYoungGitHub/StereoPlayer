#include "../inc/controller.hpp"
#include "../inc/decoder.hpp"
#include "../inc/ui.hpp"

Decoder::Decoder() {
	//input_address_ = g_decode_status.input_address;
	//camera_count_ = input_address_.size();
	//dst_width_ = g_window_size.width;
	//dst_height_ = g_window_size.height;
	//decode_status_->decode_core = g_decode_status.decode_core;
	//decode_status_->multi_thread = g_decode_status.multi_thread;
	//video_mode_ = g_decode_status.video_mode;
	//write_file_ = g_decode_status.write_file;

	//if (write_file_ == true)
	//{
	//	output_address_ = g_decode_status.output_address;
	//}

	//g_decode_status.visual_status = Init();

	
}

Decoder::Decoder(DecodeStatus * _decode_status) : Decoder() {
	decode_status_ = _decode_status;

	//change for test
	//std::vector<std::string> input_address = {
	//	"C:\\3DPlayerSavedVideo\\Titanic-left.mkv",
	//	"C:\\3DPlayerSavedVideo\\Titanic-right.mkv"
	//};
	//decode_status_->input_address = input_address;

	if (!Init()) {
		_decode_status = new DecodeStatus;
		this->~Decoder();
	}
}

Decoder::~Decoder() {
	stopCapture();
	if(decode_status_->write_file)
		stopRemux();
}

int Decoder::Init() {
	if (decode_status_->multi_thread == 0)
	{
		if (decode_status_->decode_core == 0)
			return Init_ffmpeg();
		if (decode_status_->decode_core == 1)
			return Init_cv();
	}
	if (decode_status_->multi_thread == 1)
	{
		hold_thread_ = true;
		if (decode_status_->decode_core == 0)
			return Init_ffmpeg_thread();
		if (decode_status_->decode_core == 1)
			return Init_cv_thread();
	}
	return false;
}

int Decoder::Init_ffmpeg()
{
	//AVFormatContext *pFormatCtx;
	int				 videoindex;
	AVFrame			*pFrame, *pFrameRGB;
	//AVPacket		*packet;
	struct SwsContext *img_convert_ctx;

	av_register_all();
	avformat_network_init();

	//info from decode_status_
	int camera_count = decode_status_->input_address.size();
	std::vector<std::string> input_address = decode_status_->input_address;
	int dst_width = decode_status_->width;
	int dst_height = decode_status_->height;

	for (int i = 0; i < camera_count; i++)
	{
		std::string url = input_address[i];

		//put the pFormatCtx to vector
		AVFormatContext * pFormatCtx = avformat_alloc_context();

		if (avformat_open_input(&pFormatCtx, url.c_str(), NULL, NULL) != 0)
		{
			printf("Could not open input file '%s'\n", url);
			return -1;
		}
		if (avformat_find_stream_info(pFormatCtx, NULL) < 0)
		{
			printf("Failed to retrieve input stream information.\n");
			return -1;
		}

		//output instream information
		//av_dump_format(pFormatCtx, 0, url.c_str(), 0);

		FormatCtx_.push_back(pFormatCtx);

		//TO DO :: add the init of remuxer there
		if(decode_status_->write_file)
		{
			InitRemux_ffmpeg(i);
		}

		//put the videoindex to vector
		videoindex = -1;
		for (int i = 0; i < pFormatCtx->nb_streams; i++)
		{
			if (pFormatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
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
		AVCodecContext *pCodecCtx = avcodec_alloc_context3(NULL);
		if (pCodecCtx == NULL)
		{
			printf("Could not allocate AVCodecContext\n");
			return -1;
		}
		avcodec_parameters_to_context(pCodecCtx, pFormatCtx->streams[videoindex]->codecpar);

		AVCodec *pCodec = avcodec_find_decoder(pCodecCtx->codec_id);
		if (pCodec == NULL)
		{
			printf("Codec not found.\n");
			return -1;
		}
		if (avcodec_open2(pCodecCtx, pCodec, NULL) < 0)
		{
			printf("Could not open codec.\n");
			return -1;
		}
		CodecCtx_.push_back(pCodecCtx);
		Codec_.push_back(pCodec);

		//put the pFrame and pFrameRGB to vector
		pFrame = av_frame_alloc();
		pFrameRGB = av_frame_alloc();
		unsigned char * out_buffer = (unsigned char *)av_malloc(av_image_get_buffer_size(AV_PIX_FMT_RGB32, dst_width, dst_height, 1));
		av_image_fill_arrays(pFrameRGB->data, pFrameRGB->linesize, out_buffer, AV_PIX_FMT_RGB32, dst_width, dst_height, 1);
		Frame_.push_back(pFrame);
		FrameRGB_current_.push_back(pFrameRGB);

		//put the packet to vector
		AVPacket * packet = (AVPacket *)av_malloc(sizeof(AVPacket));
		packet_.push_back(packet);

		//put the packet to vector
		img_convert_ctx = sws_getContext(pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt, dst_width, dst_height, AV_PIX_FMT_RGB32, SWS_BICUBIC, NULL, NULL, NULL);
		img_convert_ctx_.push_back(img_convert_ctx);
	}

	return true;
}

int Decoder::Init_cv()
{
	cv::VideoCapture *capture;
	cv::Mat frame;
	bool writeState = true;

	//info from decode_status_
	int camera_count = decode_status_->input_address.size();
	std::vector<std::string> input_address = decode_status_->input_address;
	std::vector<std::string> output_address = decode_status_->output_address;
	int dst_width = decode_status_->width;
	int dst_height = decode_status_->height;

	for (int i = 0; i < camera_count; i++)
	{
		//Make VideoCapture instance
		std::string url = input_address[i];
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

	av_register_all();
	avformat_network_init();

	//info from decode_status_
	int camera_count = decode_status_->input_address.size();
	std::vector<std::string> input_address = decode_status_->input_address;
	int dst_width = decode_status_->width;
	int dst_height = decode_status_->height;

	for (int i = 0; i < camera_count; i++)
	{
		std::string url = input_address[i];

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
		if (decode_status_->write_file)
		{
			InitRemux_ffmpeg(i);
		}

		//put the videoindex to vector
		videoindex = -1;
		for (int i = 0; i < pFormatCtx->nb_streams; i++)
		{
			if (pFormatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
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
		AVCodecContext *pCodecCtx = avcodec_alloc_context3(NULL);
		if (pCodecCtx == NULL)
		{
			printf("Could not allocate AVCodecContext\n");
			return -1;
		}
		avcodec_parameters_to_context(pCodecCtx, pFormatCtx->streams[videoindex]->codecpar);

		AVCodec *pCodec = avcodec_find_decoder(pCodecCtx->codec_id);
		if (pCodec == NULL)
		{
			printf("Codec not found.\n");
			return -1;
		}
		if (avcodec_open2(pCodecCtx, pCodec, NULL) < 0)
		{
			printf("Could not open codec.\n");
			return -1;
		}
		CodecCtx_.push_back(pCodecCtx);
		Codec_.push_back(pCodec);

		//put the pFrame and pFrameRGB to vector
		pFrame = av_frame_alloc();
		pFrameRGB = av_frame_alloc();
		unsigned char * out_buffer = (unsigned char *)av_malloc(av_image_get_buffer_size(AV_PIX_FMT_RGB32, dst_width, dst_height, 1));
		av_image_fill_arrays(pFrameRGB->data, pFrameRGB->linesize, out_buffer, AV_PIX_FMT_RGB32, dst_width, dst_height, 1);
		Frame_.push_back(pFrame);
		FrameRGB_current_.push_back(pFrameRGB);

		//put the packet to vector
		AVPacket * packet = (AVPacket *)av_malloc(sizeof(AVPacket));
		packet_.push_back(packet);

		//for multi thread
		concurrency::concurrent_queue<AVPacket *> * q_pkt;
		q_pkt = new concurrency::concurrent_queue<AVPacket *>;
		packet_queue_.push_back(q_pkt);

		//put the packet to vector
		img_convert_ctx = sws_getContext(pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt, dst_width, dst_height, AV_PIX_FMT_RGB32, SWS_BICUBIC, NULL, NULL, NULL);
		img_convert_ctx_.push_back(img_convert_ctx);

		//put the queue to vector
		concurrency::concurrent_queue<AVFrame *> * q;
		q = new concurrency::concurrent_queue<AVFrame *>;
		FrameRGB_queue_.push_back(q);

		//put the thread to vector
		t = new std::thread(&Decoder::Capture_ffmpeg_thread, this, i);
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

	//info from decode_status_
	int camera_count = decode_status_->input_address.size();
	std::vector<std::string> input_address = decode_status_->input_address;
	std::vector<std::string> output_address = decode_status_->output_address;
	int dst_width = decode_status_->width;
	int dst_height = decode_status_->height;

	for (int i = 0; i < camera_count; i++)
	{
		//Make VideoCapture instance
		std::string url = input_address[i];
		capture = new cv::VideoCapture(url);
		std::cout << "Camera Setup: " << url << std::endl;

		//Put VideoCapture to the vector
		camera_capture_.push_back(capture);

		//Make a queue instance
		q = new concurrency::concurrent_queue<cv::Mat>;

		//Put queue to the vector
		frame_queue_.push_back(q);

		//Make thread instance
		t = new std::thread(&Decoder::Capture_cv_thread, this, i);

		//Put thread to the vector
		camera_thread_.push_back(t);
	}
	return true;
}

int Decoder::Capture(int i) {
	if (decode_status_->multi_thread == 0){
		if (decode_status_->decode_core == 0)
			return Capture_ffmpeg(i);
		if (decode_status_->decode_core == 1)
			return Capture_cv(i);
	}
	return false;
}

int Decoder::Capture_ffmpeg(int index)
{
	if (!(av_read_frame(FormatCtx_[index], packet_[index]) >= 0))
		return false;
	if (!(packet_[index]->stream_index == videoindex_[index]))
		return false;

	//int got_picture;
	int ret;
	if (packet_[index]) {
		ret = avcodec_send_packet(CodecCtx_[index], packet_[index]);
		if (ret < 0 && ret != AVERROR_EOF) {
			return ret;
		}
	}

	ret = avcodec_receive_frame(CodecCtx_[index], Frame_[index]);
	if (ret < 0 && ret != AVERROR(EAGAIN)) {
		return ret;
	}
	/*if (avcodec_decode_video2(CodecCtx_[index], Frame_[index], &got_picture, packet_[index]) < 0)
	{
		printf("Decode Error.\n");
		return -1;
	}
	if (!got_picture)
	{
		return -1;
	}*/

	if (decode_status_->write_file == true) {
		Remux(index);
	}

	return true;
}

int Decoder::Capture_ffmpeg_thread(int index) {

	while (hold_thread_) {
		AVFrame * frame = av_frame_alloc();
		if (!(av_read_frame(FormatCtx_[index], packet_[index]) >= 0))
			continue;
		if (!(packet_[index]->stream_index == videoindex_[index]))
			continue;

		if (decode_status_->write_file == true) {
			Remux(index);
		}

		//int ret;
		//if (packet_[index]) {
		//	ret = avcodec_send_packet(CodecCtx_[index], packet_[index]);
		//	if (ret < 0 && ret != AVERROR_EOF) {
		//		return ret;
		//	}
		//}

		//ret = avcodec_receive_frame(CodecCtx_[index], Frame_[index]);
		//if (ret < 0 && ret != AVERROR(EAGAIN)) {
		//	return ret;
		//}
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

		mtx_.lock();
		FrameRGB_queue_[index]->push(frame);
		mtx_.unlock();
	}
	return true;
}


int Decoder::Capture_cv(int index)
{
	int dst_width = decode_status_->width;
	int dst_height = decode_status_->height;

	cv::VideoCapture *capture = camera_capture_[index];
	cv::Mat frame;

	//Grab frame from camera capture
	(*capture) >> frame;

	if (frame.empty())
		return false;

	//convert color which can display in DirectX;
	cv::cvtColor(frame, frame, cv::COLOR_BGR2BGRA);

	//resize it to suitable size
	if (!(frame.rows == dst_height && frame.cols == dst_width))
		cv::resize(frame, frame, cv::Size(dst_width, dst_height));

	//put it into the 
	frame_cv_[index] = frame;

	//relase frame resource
	frame.release();
	return true;
}

int Decoder::Capture_cv_thread(int index)
{
	int dst_width = decode_status_->width;
	int dst_height = decode_status_->height;

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
			if (!(frame.rows == dst_height && frame.cols == dst_width))
				cv::resize(frame, frame, cv::Size(dst_width, dst_height));
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


int Decoder::stopCapture()
{
	if (decode_status_->multi_thread == 1) {
		hold_thread_ = false;
		for (int i = 0; i < camera_thread_.size(); i++) {
			if (camera_thread_[i]->joinable())
				camera_thread_[i]->join();
		}
	}

	if (decode_status_->decode_core == 0)
		return stopCapture_ffmpeg();
	if (decode_status_->decode_core == 1)
		return stopCapture_cv();
	return false;
}

int Decoder::stopCapture_ffmpeg()
{
	if (decode_status_->input_address.size() == 0)
		return true;
	for (int i = 0; i < decode_status_->input_address.size(); i++)
	{
		sws_freeContext(img_convert_ctx_[i]);

		av_frame_free(&FrameRGB_current_[i]);
		av_frame_free(&Frame_[i]);
		avcodec_close(CodecCtx_[i]);
		avformat_close_input(&FormatCtx_[i]);
	}
	decode_status_->input_address.clear();
	return true;
}

int Decoder::stopCapture_cv()
{
	cv::VideoCapture *cap;
	for (int i = 0; i < decode_status_->input_address.size(); i++)
	{
		cap = camera_capture_[i];
		if (cap->isOpened()) {
			//Relase VideoCapture resource
			cap->release();
			std::cout << "Capture " << i << " released" << std::endl;
		}
	}
	return true;
}

int Decoder::Decode(int index, D3DLOCKED_RECT lockedrect)
{
	if (decode_status_->multi_thread == 0)
	{
		if (decode_status_->decode_core == 0)
			return Decode_ffmpeg(index, lockedrect);
		if (decode_status_->decode_core == 1)
			return Decode_cv(index, lockedrect);
	}
	if (decode_status_->multi_thread == 1)
	{
		if (decode_status_->decode_core == 0)
			return Decode_ffmpeg_thread(index, lockedrect);
		if (decode_status_->decode_core == 1)
			return Decode_cv_thread(index, lockedrect);
	}
	return true;
}

int Decoder::Decode_ffmpeg(int index, D3DLOCKED_RECT lockedrect)
{
	if (!Capture(index))
		return false;

	int dst_width = decode_status_->width;
	int dst_height = decode_status_->height;

	//this code is replaced by the above code to reduce time 
	BYTE* imagedata = (BYTE *)lockedrect.pBits;
	av_image_fill_arrays(FrameRGB_current_[index]->data, FrameRGB_current_[index]->linesize, imagedata, AV_PIX_FMT_RGB32, dst_width, dst_height, 1);
	//avpicture_fill((AVPicture *)FrameRGB_current_[index], imagedata, AV_PIX_FMT_RGB32, dst_width, dst_height);
	sws_scale(img_convert_ctx_[index], Frame_[index]->data, Frame_[index]->linesize, 0, CodecCtx_[index]->height, FrameRGB_current_[index]->data, FrameRGB_current_[index]->linesize);

	return true;
}

int Decoder::Decode_cv(int index, D3DLOCKED_RECT lockedrect)
{
	if (!Capture(index))
		return false;

	cv::Mat frame;
	frame = frame_cv_[index];

	if (frame.empty() || frame.at<uchar>(0, 0) == NULL) //make sure it works well
		return false;

	BYTE * imagedata = (BYTE *)lockedrect.pBits;

	int dst_width = decode_status_->width;
	int dst_height = decode_status_->height;

	//this code is replaced by the above code to reduce time 
	for (int j = 0; j < dst_height; j++)
	{
		uchar* srcData = frame.ptr<uchar>(j);
		if (srcData != NULL)
			memcpy(imagedata + j * lockedrect.Pitch, srcData, dst_width * 4);
	}
	
	return true;
}

int Decoder::Decode_ffmpeg_thread(int index, D3DLOCKED_RECT lockedrect)
{
	FrameRGB_queue_[index]->try_pop(Frame_[index]);

	int dst_width = decode_status_->width;
	int dst_height = decode_status_->height;

	BYTE* imagedata = (BYTE *)lockedrect.pBits;
	av_image_fill_arrays(FrameRGB_current_[index]->data, FrameRGB_current_[index]->linesize, imagedata, AV_PIX_FMT_RGB32, dst_width, dst_height, 1);
	//avpicture_fill((AVPicture *)FrameRGB_current_[index], imagedata, AV_PIX_FMT_RGB32, dst_width, dst_height);
	sws_scale(img_convert_ctx_[index], Frame_[index]->data, Frame_[index]->linesize, 0, CodecCtx_[index]->height, FrameRGB_current_[index]->data, FrameRGB_current_[index]->linesize);

	return true;
}

int Decoder::Decode_cv_thread(int index, D3DLOCKED_RECT lockedrect)
{
	if (frame_queue_[index]->unsafe_size() == 0)
		return false;

	cv::Mat frame;
	frame_queue_[index]->try_pop(frame);

	if (frame.empty() || frame.at<uchar>(0, 0) == NULL) //make sure it works well
		return false;

	BYTE * imagedata = (BYTE *)lockedrect.pBits;

	int dst_width = decode_status_->width;
	int dst_height = decode_status_->height;
	//this code is replaced by the above code to reduce time 
	for (int j = 0; j < dst_height; j++)
	{
		uchar* srcData = frame.ptr<uchar>(j);
		if (srcData != NULL)
			memcpy(imagedata + j * lockedrect.Pitch, srcData, dst_width * 4);
	}
	
	return true;
}

int Decoder::InitRemux(int index) {
	if (decode_status_->decode_core == 0)
		return InitRemux_ffmpeg(index);
	if (decode_status_->decode_core == 1)
		return InitRemux_cv(index);
	return false;
}

int Decoder::InitRemux_ffmpeg(int index) {

	std::vector<std::string> output_address = decode_status_->output_address;

	//output
	AVFormatContext *ofmt_ctx = NULL;
	avformat_alloc_output_context2(&ofmt_ctx, NULL, NULL, output_address[index].c_str());
	if (!ofmt_ctx)
	{
		printf("Could not create output context\n");
		return -1;
	}
	//push the ofmt_ctx to the vector write_FormatCtx_
	out_FormatCtx_.push_back(ofmt_ctx);

	int stream_mapping_size = 0;
	int * stream_mapping = NULL;
	int stream_index = 0;
	stream_mapping_size = FormatCtx_[index]->nb_streams;
	stream_mapping = (int *)av_mallocz_array(stream_mapping_size, sizeof(*stream_mapping));
	if (!stream_mapping) {
		return false;
	}
	stream_mapping_size_.push_back(stream_mapping_size);
	stream_mapping_.push_back(stream_mapping);

	AVOutputFormat * ofmt = NULL;
	ofmt = ofmt_ctx->oformat;
	for (int i = 0; i < FormatCtx_[index]->nb_streams; i++)
	{
		AVStream *out_stream;
		AVStream *in_stream = FormatCtx_[index]->streams[i];
		AVCodecParameters *in_codecpar = in_stream->codecpar;

		if (in_codecpar->codec_type != AVMEDIA_TYPE_AUDIO &&
			in_codecpar->codec_type != AVMEDIA_TYPE_VIDEO &&
			in_codecpar->codec_type != AVMEDIA_TYPE_SUBTITLE) {
			stream_mapping[i] = -1;
			continue;
		}

		stream_mapping_[index][i] = stream_index++;

		out_stream = avformat_new_stream(ofmt_ctx, NULL);
		if (!out_stream)
		{
			printf("Failed allocating output stream\n");
			return -1;
		}
		//copy the setting of avcodecco
		if (avcodec_parameters_copy(out_stream->codecpar, in_codecpar))
		{
			printf("Failed to copy codec parameters.\n");
			return -1;
		}

		out_stream->codecpar->codec_tag = 0;
	}

	//av_dump_format(ofmt_ctx, 0, output_address[index].c_str(), 1);

	//open output file
	if (!(ofmt->flags & AVFMT_NOFILE))
	{
		if (avio_open(&ofmt_ctx->pb, output_address[index].c_str(), AVIO_FLAG_WRITE))
		{
			printf("Could not open output URL '%s'", output_address[index]);
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

int Decoder::InitRemux_cv(int index) {
	return true;
}

int Decoder::Remux(int index) {

	if (decode_status_->decode_core == 0)
		return Remux_ffmpeg(index);
	if (decode_status_->decode_core == 1)
		return Remux_cv(index);
	
	return false;
}

int Decoder::Remux_ffmpeg(int index) {
	AVStream * in_stream, *out_stream;
	AVPacket pkt = *packet_[index];
	in_stream = FormatCtx_[index]->streams[pkt.stream_index];
	if (pkt.stream_index >= stream_mapping_size_[index] ||
		stream_mapping_[index][pkt.stream_index] < 0) {
		av_packet_unref(&pkt);
		return true;
	}

	pkt.stream_index = stream_mapping_[index][pkt.stream_index];
	out_stream = out_FormatCtx_[index]->streams[pkt.stream_index];

	pkt.pts = av_rescale_q_rnd(pkt.pts, in_stream->time_base, out_stream->time_base, (AVRounding)(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
	pkt.dts = av_rescale_q_rnd(pkt.dts, in_stream->time_base, out_stream->time_base, (AVRounding)(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
	pkt.duration = av_rescale_q(pkt.duration, in_stream->time_base, out_stream->time_base);
	pkt.pos = -1;

	if (av_interleaved_write_frame(out_FormatCtx_[index], &pkt) < 0)
	{
		printf("Error muxing packet\n");
		return -1;
	}
	av_packet_unref(&pkt);
	return true;
}

int Decoder::Remux_cv(int index) {
	return true;
}

int Decoder::stopRemux() {
	if (decode_status_->decode_core == 0)
		return stopRemux_ffmpeg();
	if (decode_status_->decode_core == 1)
		return stopRemux_cv();
	return false;
}

int Decoder::stopRemux_ffmpeg() {
	if (decode_status_->output_address.size() == 0)
		return true;
	for (int i = 0; i < decode_status_->output_address.size(); i++)
	{
		av_write_trailer(out_FormatCtx_[i]);

		/* close output */
		if (out_FormatCtx_[i] && !(out_Format_[i]->flags & AVFMT_NOFILE))
			avio_close(out_FormatCtx_[i]->pb);
		avformat_free_context(out_FormatCtx_[i]);

		av_freep(&stream_mapping_[i]);
	}
	decode_status_->output_address.clear();
	return true;
}

int Decoder::stopRemux_cv() {
	return true;
}