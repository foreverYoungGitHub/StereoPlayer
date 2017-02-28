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

class Viewer {
public:
	Viewer();
	~Viewer();
	
	int Display();


protected:
	//initialze the D3D environment
	int Init();
	//Release and initialze the D3D and D3D devices
	void CleanUp();

	int CheckStatus();
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
	IDirect3DSurface9 *single_src_surface = NULL, *stereo_src_surface = NULL;
	//surfaces use to display the result to the screen
	IDirect3DSurface9 *dst_surface_ = NULL, *BackBuffer_;

	//locked rect use to 
	D3DLOCKED_RECT lr;

	
};