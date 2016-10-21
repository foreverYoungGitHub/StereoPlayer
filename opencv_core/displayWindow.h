#pragma once

#include <d3d9.h>
#include <d3dx9.h>
#include <tchar.h>
#include <opencv2\opencv.hpp>
#include <thread>

#define WINDOW_CLASS _T("UGPDX")
#define WINDOW_NAME _T("3D")

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

class displayWindow {

public:

	displayWindow();

	//initialize the window
	int InitD3D(HWND hwnd, bool fullscreen);

	//message process
	static LRESULT CALLBACK WINAPI MyWndProc(HWND hwnd, UINT msg, WPARAM wparma, LPARAM lparam);
	

	//
	void Cleanup();

	//the main loop for rendering process
	bool Render();
	bool Render(cv::Mat left, cv::Mat right);

	//transform cv::mat to IDirect3DSurface9
	void transformLeft(cv::Mat left);
	void transformRight(cv::Mat right);


	//void transformLeft();
	//void transformRight();
	void render(cv::Mat img, D3DLOCKED_RECT lockedrect);
	void renderLoop(cv::Mat left, int height_start, int height_end, D3DLOCKED_RECT lockedrect);

	int width = 1920, height = 1080;
	IDirect3DSurface9 *gImageSrcLeft_ = NULL, *gImageSrcRight_ = NULL, *gImageSrcStereo_ = NULL, *gBackBuffer_, *gImageSrcTemp_ = NULL;


	LPDIRECT3D9 g_D3D_ = NULL;
	LPDIRECT3DDEVICE9 g_D3DDevice_ = NULL;

	CRITICAL_SECTION  m_critial_;


	RECT srcRect = { 0,0,width,height };
	RECT dstRect = { 0,0,width,height };
	RECT dstRect2 = { width,0,width * 2,height };
	D3DLOCKED_RECT lr;
	RECT bufferRect = { 0,0,width,height };
	RECT stereoRect = { 0,0,width * 2,height + 1 };
	WNDCLASSEX wc;
	int renderLoopTimes = 8;
};







