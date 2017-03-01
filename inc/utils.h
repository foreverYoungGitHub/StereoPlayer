#pragma once
#include <string>
#include <vector>
#include <d3d9.h>
#include <d3dx9.h>

struct WindowSize {
	LPCSTR name;
	int width;
	int height;
	bool fullscreen;
	
	WindowSize() : name("Player"),width(1920), height(1080), fullscreen(false){}
	WindowSize(int w, int h) : name("Player"), width(w), height(h), fullscreen(false) {}
};

struct DecodeStatus {

	//the status for display mode, 0 means homepage, 1 means one channel video, 2 means stereo video
	int visual_status; 
	int video_mode;
	int decode_core;
	bool write_file;
	bool multi_thread;
	std::string input_address;
	DecodeStatus() :visual_status(0), write_file(0), multi_thread(0), input_address("") {}
};

struct VideoInfo {

};



extern WindowSize g_window_size;
extern DecodeStatus g_decode_status;
extern VideoInfo g_video_info;