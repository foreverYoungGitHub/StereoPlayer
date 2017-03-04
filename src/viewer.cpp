#include "../inc/viewer.h"
//#include "../inc/utils.h"
#include "../inc/controller.h"
#include "../IMGUI/imgui.h"
#include "../IMGUI/imgui_impl_dx9.h"

//Controller * g_controller;

Viewer::Viewer()
{
	wc_ = { sizeof(WNDCLASSEX), CS_CLASSDC, (WNDPROC)WndProc, 0L, 0L,
		GetModuleHandle(NULL), NULL, LoadCursor(NULL, IDC_ARROW), NULL, NULL,
		WINDOW_CLASS, NULL };
	RegisterClassEx(&wc_);
	hwnd_ = CreateWindow(WINDOW_CLASS, g_window_size.name, WS_OVERLAPPEDWINDOW,
		0, 0, g_window_size.width, g_window_size.height, NULL, NULL,//GetDesktopWindow(), NULL,
		wc_.hInstance, NULL);
	
	if (hwnd_ == NULL) {
		return;
	}

	g_controller = new Controller();
}

Viewer::~Viewer() {

}

int Viewer::Display(){
	if (Init()){

		g_controller->Init(hwnd_, g_D3DDevice);

		ShowWindow(hwnd_, SW_SHOWDEFAULT);
		UpdateWindow(hwnd_);

		while (msg_.message != WM_QUIT){
			if (PeekMessage(&msg_, NULL, 0U, 0U, PM_REMOVE))
			{
				TranslateMessage(&msg_);
				DispatchMessage(&msg_);
				continue;
			}
			g_controller->Update();
			Reader();
			Render();
		}
	}
	return true;
}

int Viewer::Init() {
	//HRESULT lRet;
	InitializeCriticalSection(&m_critial_);
	CleanUp();

	D3DDISPLAYMODE displayMode;

	// Create the D3D object.
	D3D_ = Direct3DCreate9(D3D_SDK_VERSION);
	if (D3D_ == NULL)
		return -1;

	// Get the desktop display mode.
	if (FAILED(D3D_->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &displayMode)))
		return false;

	ZeroMemory(&g_d3dpp, sizeof(g_d3dpp));

	if (g_window_size.fullscreen)
	{
		g_d3dpp.Windowed = FALSE;
		g_d3dpp.BackBufferWidth = g_window_size.width;
		g_d3dpp.BackBufferHeight = g_window_size.height;
	}
	else
		g_d3dpp.Windowed = TRUE;

	g_d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	g_d3dpp.BackBufferFormat = D3DFMT_A8R8G8B8;// displayMode.Format;
	g_d3dpp.EnableAutoDepthStencil = TRUE;
	g_d3dpp.AutoDepthStencilFormat = D3DFMT_D16;
	g_d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;
	//d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_ONE;
	g_d3dpp.BackBufferCount = 1;
	g_d3dpp.Flags = D3DPRESENTFLAG_LOCKABLE_BACKBUFFER;

	if (FAILED(D3D_->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hwnd_, D3DCREATE_HARDWARE_VERTEXPROCESSING, &g_d3dpp, &g_D3DDevice)))
	{
		return false;
	}

	g_D3DDevice->CreateOffscreenPlainSurface(g_window_size.width, g_window_size.height, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &single_src_surface, NULL);

	return true;
}

void Viewer::CleanUp()
{
	EnterCriticalSection(&m_critial_);

	if (g_D3DDevice != NULL)
		g_D3DDevice->Release();
	if (D3D_ != NULL)
		D3D_->Release();

	g_D3DDevice = NULL;
	D3D_ = NULL;

	LeaveCriticalSection(&m_critial_);
}

int Viewer::Reader() {

	if (g_decode_status.visual_status == 0)
	{
		//if (FAILED(D3DXLoadSurfaceFromFile(single_src_surface, NULL, NULL, ".//resources//background.jpg", NULL, D3DX_FILTER_NONE, 0, NULL)))
		//	return false;
		dst_surface_ = single_src_surface;
	}
	else if (g_decode_status.visual_status == 1)
	{
		single_src_surface->LockRect(&lr, NULL, 0);
		
		single_src_surface->UnlockRect();

		dst_surface_ = single_src_surface;
	}
	else if (g_decode_status.visual_status == 2)
	{
		stereo_src_surface->LockRect(&lr, NULL, 0);

		LPNVSTEREOIMAGEHEADER pSIH =
			(LPNVSTEREOIMAGEHEADER)(((unsigned char *)lr.pBits) + (lr.Pitch * (g_window_size.height)));

		pSIH->dwSignature = NVSTEREO_IMAGE_SIGNATURE;
		pSIH->dwBPP = 32;
		pSIH->dwFlags = SIH_SWAP_EYES;
		pSIH->dwWidth = g_window_size.width * 2;
		pSIH->dwHeight = g_window_size.height;

		stereo_src_surface->UnlockRect();

		dst_surface_ = stereo_src_surface;
	}

	return true;
}

int Viewer::Render() {
	g_D3DDevice->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_ARGB(1, 0, 0, 0), 1.0f, 0);

	g_D3DDevice->BeginScene();

	g_D3DDevice->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &BackBuffer_);
	g_D3DDevice->StretchRect(dst_surface_, NULL, BackBuffer_, NULL, D3DTEXF_LINEAR);

	g_controller->Render();
	g_D3DDevice->EndScene();

	g_D3DDevice->Present(NULL, NULL, NULL, NULL);

	return true;
}

LRESULT WINAPI Viewer::WndProc(HWND hwnd, UINT msg, WPARAM wparma, LPARAM lparam)
{
	if (ImGui_ImplDX9_WndProcHandler(hwnd, msg, wparma, lparam))
		return true;

	switch (msg)
	{
	case WM_SIZE:
		//if (wparma != SIZE_MINIMIZED)
		if (g_D3DDevice != NULL && wparma != SIZE_MINIMIZED)
		{
			ImGui_ImplDX9_InvalidateDeviceObjects();
			//d3dpp.BackBufferWidth = LOWORD(lparam);
			//d3dpp.BackBufferHeight = HIWORD(lparam);
			//HRESULT hr = g_D3DDevice_->Reset(&d3dpp);
			//if (hr == D3DERR_INVALIDCALL)
			//	IM_ASSERT(0);
			ImGui_ImplDX9_CreateDeviceObjects();
		}
		return 0;
	case WM_SYSCOMMAND:
		if ((wparma & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
			return 0;
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hwnd, msg, wparma, lparam);
}