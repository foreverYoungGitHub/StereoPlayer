#pragma once
#include <string>
#include <vector>

struct DecodeStatus {
	//the status for display mode, -1 means homepage, 0,1 means capture video
	//one channel video, 2 means stereo video
	int visual_status;
	//int video_mode;
	int decode_core;
	int width;
	int height;
	bool write_file;
	bool multi_thread;
	bool audio;
	std::vector<std::string> input_address;
	std::vector<std::string> output_address;
	std::string audio_address;
	DecodeStatus() :visual_status(-1), decode_core(0), write_file(0), multi_thread(1), audio(false){}
};

class Viewer;
class UI;
class Decoder;
class Controller {
public:
	Controller();
	~Controller();
	
	int Display();

	int BuildConnection();
	int Disconnection();

	Viewer * viewer_;
	UI * ui_;
	Decoder * decoder_;
	DecodeStatus * decode_status_;
};