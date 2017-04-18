#pragma once

#include <d3d9.h>
#include <d3dx9.h>

#define WINDOW_CLASS "UGPDX"
//#define WINDOW_NAME "Player"

#define SIH_SWAP_EYES 0x00000001
#define SIH_SCALE_TO_FIT 0x00000002

// Stereo Blit defines
#define NVSTEREO_IMAGE_SIGNATURE 0x4433564e //NV3D
typedef struct _Nv_Stereo_Image_Header
{
	unsigned int dwSignature;
	unsigned int dwWidth;
	unsigned int dwHeight;
	unsigned int dwBPP;
	unsigned int dwFlags;
} NVSTEREOIMAGEHEADER, *LPNVSTEREOIMAGEHEADER;

static LPDIRECT3DDEVICE9 g_D3DDevice = NULL;
static D3DPRESENT_PARAMETERS g_d3dpp;

//message process
extern LRESULT ImGui_ImplDX9_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

class Controller;

class Viewer {
public:
	Viewer(Controller * _controller);
	~Viewer();

	int Display();

	

protected:
	Viewer();

	//initialze the D3D environment
	int Init();
	//Release and initialze the D3D and D3D devices
	void CleanUp();

	//Read the data from decoder
	int Reader();

	int Render();
	

	//message process
	static LRESULT CALLBACK WINAPI WndProc(HWND hwnd, UINT msg, WPARAM wparma, LPARAM lparam);

private:
	//the varables for the windows
	WNDCLASSEX wc_;
	HWND hwnd_;
	CRITICAL_SECTION  m_critial_;
	MSG msg_;
	LPDIRECT3D9 D3D_ = NULL;

	//surfaces use to reader the data from decoder
	IDirect3DSurface9 *single_src_surface_ = NULL, *single_src_surface_extra_ = NULL, *stereo_src_surface_ = NULL;
	//surfaces use to display the result to the screen
	IDirect3DSurface9 *dst_surface_ = NULL, *BackBuffer_;
	//locked rect use to copy the data from decoder
	D3DLOCKED_RECT lr_;

	//controller
	Controller * controller_;

	struct WindowSize {
		LPCSTR name;
		int width;
		int height;
		bool fullscreen;

		WindowSize() : name("Player"), width(1920), height(1080), fullscreen(false) {}
		WindowSize(int w, int h) : name("Player"), width(w), height(h), fullscreen(true) {}
	};

	WindowSize window_size_;
};
