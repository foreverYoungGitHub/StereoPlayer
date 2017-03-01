#pragma once
#include <string>
#include <vector>
#include <d3d9.h>

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