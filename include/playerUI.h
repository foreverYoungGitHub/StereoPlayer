#pragma once

#include "IMGUI\imgui.h"
#include "IMGUI\imgui_impl_dx9.h"
#include <d3d9.h>
#include <stdio.h>
#include <vector>
#include <string>
#include <fstream>

class playerUI
{
public:
	playerUI();
	~playerUI();

	bool Init(void* hwnd, IDirect3DDevice9* device);
	void Shutdown();
	void NewFrame(bool* p_open);
	void MainUI(bool* p_open);
	void Render();

	bool readCSV();

	std::vector<std::string> names_, locations_, time_;
	//
	std::vector<std::vector<std::string>> url_; //the vector to store the information for the live stream, [?][0] is left view url, [?][1] is right view url, [?][2] is the name. [?][3] is the location
	std::vector<std::vector<std::string>> file_;//the vector to store the information for the files, [?][0] is left view url, [?][1] is right view url, [?][2] is the name. [?][3] is the time

	int selectedURL_ = -1;
	int selectedFile_ = -1;
	int videoMode_ = 0;
	bool HomePage = true;
	bool isWriteFile = false;
};

