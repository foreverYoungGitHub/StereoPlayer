#pragma once
//
#include <d3d9.h>
#include <d3dx9.h>

#include "playerUI.h"
#include "IMGUI\imgui.h"
#include "IMGUI\imgui_impl_dx9.h"

#include "playerDecoder.h"

#define WINDOW_CLASS "UGPDX"
#define WINDOW_NAME "Player"

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

static LPDIRECT3DDEVICE9 g_D3DDevice_ = NULL;
static D3DPRESENT_PARAMETERS d3dpp;

//message process
extern LRESULT ImGui_ImplDX9_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

class playerDisplay
{
public:
	playerDisplay();
	~playerDisplay();

	//initialize the window
	int InitD3D(bool fullscreen);
	int InitD3D(HWND hwnd, bool fullscreen);

	//message process
	
	static LRESULT CALLBACK WINAPI WndProc(HWND hwnd, UINT msg, WPARAM wparma, LPARAM lparam);

	//
	void Cleanup();

	//the render functions for rendering process
	bool RenderHome();
	bool RenderSingleView(int index);
	bool RenderStereoView();
	bool Transform(int index);
	bool TransformLeft();
	bool TransformRight();
	//the display function contains all the components including the main rendering loop 
	void Display();

	void Shutdown();

	bool activeDecoding();
	bool activeRemutexer();

	//variables
	playerUI * ui_;
	playerDecoder * decoder_;

	//
	int selectURLTemp_, selectFileTemp_;

	//the varables for the windows
	WNDCLASSEX wc;
	HWND hwnd;
	CRITICAL_SECTION  m_critial_;
	MSG msg;
	LPDIRECT3D9 g_D3D_ = NULL;
	//LPDIRECT3DDEVICE9 g_D3DDevice_ = NULL;
	//D3DPRESENT_PARAMETERS d3dpp;


	int width = 1920, height = 1080;
	IDirect3DSurface9 *gImageSrcHome_ = NULL, *gImageSrcSingle_ = NULL, *gImageSrcLeft_ = NULL, *gImageSrcRight_ = NULL, *gImageSrcStereo_ = NULL, *gBackBuffer_, *gImageSrcTemp_ = NULL;
	bool no_background = false;
	bool isDecoding = false;
	bool isWriteFile = false;

	RECT srcRect = { 0,0,width,height };
	RECT dstRect = { 0,0,width,height };
	RECT dstRect2 = { width,0,width * 2,height };
	D3DLOCKED_RECT lr;
	RECT bufferRect = { 0,0,width,height };
	RECT stereoRect = { 0,0,width * 2,height + 1 };
	
};

