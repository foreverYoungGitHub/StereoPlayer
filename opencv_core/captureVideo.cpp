#include "captureVideo.hpp"

captureVideo::captureVideo(std::string urlL, Mode mode, bool stereo)
{
	vector<string> stream_source;
	stream_source.push_back(urlL);

	captureVideo::captureVideo(stream_source);
}

captureVideo::captureVideo(string urlL, string urlR)
{
	vector<string> stream_source;
	stream_source.push_back(urlL);
	stream_source.push_back(urlR);

	captureVideo::captureVideo(stream_source);
}

captureVideo::captureVideo(string urlL, string urlR, int screenWidth, int screenHeight)
{
	vector<string> stream_source;
	stream_source.push_back(urlL);
	stream_source.push_back(urlR);
	width_ = screenWidth;
	height_ = screenHeight;

	captureVideo::captureVideo(stream_source);
}

captureVideo::captureVideo(vector<string> stream_source, Mode mode, bool stereo)
{
	camera_source = stream_source;
	camera_count = camera_source.size();
	isUSBCamera = false;

	startCapture();
}

captureVideo::captureVideo(vector<int> capture_index)
{
	camera_index = capture_index;
	camera_count = capture_index.size();
	isUSBCamera = true;
	realtime = realtime;
	startCapture();
}

captureVideo::~captureVideo()
{
	stopCapture();
}



void captureVideo::captureStereoFrame()
{
	VideoCapture *capture = camera_capture[0];
	Mat frame;
	//frames_.push_back(frame);
	//frames_.push_back(frame);
	mode_ = UPDOWN;
	while (true)
	{		
		//Grab frame from camera capture
		(*capture) >> frame;
		if (!frame.empty())
		{
			cv::cvtColor(frame, frame, cv::COLOR_BGR2BGRA);
			//check the stereo way and resize it to suitable size
			switch (mode_)
			{
			case LEFTRIGHT:	//the url is a left-right stereo video
			{
				if (!(frame.rows == height_ && frame.cols == width_ * 2))
					resize(frame, frame, Size(width_ * 2, height_));
				//Put frame to the queue
				if (realtime)
				{
					writeState_[0] = true;
					frames_[0] = frame(Rect(0, 0, width_, height_));
					writeState_[1] = false;
					writeState_[1] = true;
					frames_[1] = frame(Rect(width_, 0, width_, height_));
					writeState_[1] = false;
				}
				//Put frame to the member variabale vector
				else
				{
					frame_queue[0]->push(frame(Rect(0, 0, width_, height_)));
					frame_queue[0]->push(frame(Rect(width_, 0, width_, height_)));
				}

			}
			case UPDOWN: //the url is a up-down stereo video
			{
				if (!(frame.rows == height_ * 2 && frame.cols == width_))
					resize(frame, frame, Size(width_, height_ * 2));
				//Put frame to the queue
				if (realtime)
				{
					writeState_[0] = true;
					frames_[0] = frame(Rect(0, 0, width_, height_));
					writeState_[1] = false;
					writeState_[1] = true;
					frames_[1] = frame(Rect(0, height_, width_, height_));
					writeState_[1] = false;
				}
				//Put frame to the member variabale vector
				else
				{
					frame_queue[0]->push(frame(Rect(0, 0, width_, height_)));
					frame_queue[0]->push(frame(Rect(0, height_, width_, height_)));
				}

			}
			default:
				break;
			}
		}				
	}
	//relase frame resource
	frame.release();
}

void captureVideo::captureFrame(int index)
{
	VideoCapture *capture = camera_capture[index];
	Mat frame;
	frames_.push_back(frame);
	while (true)
	{
		//Grab frame from camera capture
		(*capture) >> frame;
		if(!frame.empty())
		{ 			
			cv::cvtColor(frame, frame, cv::COLOR_BGR2BGRA);
			//resize it to suitable size
			if (!(frame.rows == height_ && frame.cols == width_))
				resize(frame, frame, Size(width_, height_));
			//Put frame to the queue
			if (realtime)
			{
				writeState_[index] = true;
				frames_[index] = frame;
				writeState_[index] = false;
			}				
			//Put frame to the member variabale vector
			else
				frame_queue[index]->push(frame);
		}
		while (frame_queue[index]->unsafe_size() > 5)
		{
			frame_queue[index]->try_pop(frame);
		}
	}
	//relase frame resource
	frame.release();
}

void captureVideo::convert2render(int index)
{
	Mat frame;
	while (true)
	{
		
		if (frame_queue[index]->unsafe_size() == 0)
			continue;
		
		frame_queue[index]->try_pop(frame);

		unsigned char * imagedata = new unsigned char;

		if (!(frame.empty() || frame.at<uchar>(0, 0) == NULL)) //make sure it works well
		{
			//cv::cvtColor(img_, img_, cv::COLOR_BGR2BGRA);

			for (int j = 0; j < frame.rows; j++)
			{
				uchar* srcData = frame.ptr<uchar>(j);
				if (srcData != NULL)
					memcpy(imagedata + j * frame.cols * 4, srcData, frame.cols * 4);
			}
		}
	}
	
	
}

void captureVideo::startCapture()
{
	VideoCapture *capture;
	thread *t;
	concurrent_queue<Mat> *q;
	bool writeState = true;

	for (int i = 0; i < camera_count; i++)
	{
		//Make VideoCapture instance
		if (!isUSBCamera) {
			string url = camera_source[i];
			capture = new VideoCapture(url);
			cout << "Camera Setup: " << url << endl;
		}
		else {
			int idx = camera_index[i];
			capture = new VideoCapture(idx);
			cout << "Camera Setup: " << to_string(idx) << endl;
		}

		//Put VideoCapture to the vector
		camera_capture.push_back(capture);

		//Put writing state
		writeState_.push_back(writeState);

		if (!realtime)
		{
			//Make a queue instance
			q = new concurrent_queue<Mat>;

			//Put queue to the vector
			frame_queue.push_back(q);
		}

		//Make thread instance
		if(camera_count == 1)
			t = new thread(&captureVideo::captureStereoFrame, this);
		else
			t = new thread(&captureVideo::captureFrame, this, i);

		//Put thread to the vector
		camera_thread.push_back(t);

	}
}

void captureVideo::stopCapture()
{
	VideoCapture *cap;
	for (int i = 0; i < camera_count; i++)
	{
		cap = camera_capture[i];
		if (cap->isOpened()) {
			//Relase VideoCapture resource
			cap->release();
			cout << "Capture " << i << " released" << endl;
		}
	}
}


void captureVideo::setStereo(bool stereo)
{
	stereo_ = stereo;
}