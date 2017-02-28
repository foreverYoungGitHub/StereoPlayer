#pragma once


#include <d3d9.h>

class Controller {
public:
	Controller();
	~Controller();

	int Init(void* hwnd, IDirect3DDevice9* device);

	int Render();

	int Update();

	int MainUI();

	int Shutdown();
};