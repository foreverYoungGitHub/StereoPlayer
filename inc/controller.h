#pragma once
#include <string>
#include <vector>
#include <d3d9.h>

struct WindowSize {
	LPCSTR name;
	int width;
	int height;
	bool fullscreen;

	WindowSize() : name("Player"), width(1920), height(1080), fullscreen(false) {}
	WindowSize(int w, int h) : name("Player"), width(w), height(h), fullscreen(false) {}
};

struct DecodeStatus {

	//the status for display mode, 0 means homepage, 1 means capture video
	//one channel video, 2 means stereo video
	int visual_status;
	int video_mode;
	int decode_core;
	bool write_file;
	bool multi_thread;
	std::vector<std::string> input_address;
	std::vector<std::string> output_address;
	DecodeStatus() :visual_status(0), write_file(0), multi_thread(0) {}
};

struct VideoInfo {

};

class Controller {
public:
	Controller();
	~Controller();

	int Init(void* hwnd, IDirect3DDevice9* device);

	int Render();

	int Update();

	int MainUI();

	int PlaylistUI();

	int Shutdown();

	int BuildConnection();

	int Disconnection();

	std::string get_current_time();

	int ReadCSV();
	int WriteCSV();
	int ReadCSV(std::string address);
	int WriteCSV(std::string address);

private:

	struct Playlist {
		bool exist;
		int input_mode;
		int select_item;
		std::vector<std::string> playlist_items;
		//char** playlist_items;
		std::string save_file_address;
		std::vector<std::string> address_end = {
			"live_stream.txt",
			"save_files.txt"
		};
		Playlist():exist(0),input_mode(0), select_item(-1), save_file_address("C:\\3DPlayerSavedVideo\\"){}
	};

	Playlist playlist_status_;
	
};

extern Controller * g_controller;
extern WindowSize g_window_size;
extern DecodeStatus g_decode_status;
extern VideoInfo g_video_info;