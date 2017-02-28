#pragma once
#include <string>

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
	std::string input_address;
	DecodeStatus() :visual_status(0), input_address("") {}
};

struct VideoInfo {

};

WindowSize g_window_size;
DecodeStatus g_decode_status;
VideoInfo g_video_info;