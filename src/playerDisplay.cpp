#include "playerDisplay.h"



playerDisplay::playerDisplay()
{	
	wc = { sizeof(WNDCLASSEX), CS_CLASSDC, (WNDPROC)WndProc, 0L, 0L,
		GetModuleHandle(NULL), NULL, LoadCursor(NULL, IDC_ARROW), NULL, NULL,
		WINDOW_CLASS, NULL };
	RegisterClassEx(&wc);
	hwnd = CreateWindow(WINDOW_CLASS, WINDOW_NAME, WS_OVERLAPPEDWINDOW,
		0, 0, width, height, NULL, NULL,//GetDesktopWindow(), NULL,
		wc.hInstance, NULL);
	if (hwnd == NULL) {
		return;
	}

	ui_ = new playerUI();
}


playerDisplay::~playerDisplay()
{
}


int playerDisplay::InitD3D(HWND hwnd, bool fullscreen)
{
	//HRESULT lRet;
	InitializeCriticalSection(&m_critial_);
	Cleanup();

	D3DDISPLAYMODE displayMode;

	// Create the D3D object.
	g_D3D_ = Direct3DCreate9(D3D_SDK_VERSION);
	if (g_D3D_ == NULL)
		return -1;

	// Get the desktop display mode.
	if (FAILED(g_D3D_->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &displayMode)))
		return false;

	D3DPRESENT_PARAMETERS d3dpp;
	ZeroMemory(&d3dpp, sizeof(d3dpp));

	if (fullscreen)
	{
		d3dpp.Windowed = FALSE;
		d3dpp.BackBufferWidth = width;
		d3dpp.BackBufferHeight = height;
	}
	else
		d3dpp.Windowed = TRUE;

	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	d3dpp.BackBufferFormat = D3DFMT_A8R8G8B8;// displayMode.Format;
	d3dpp.EnableAutoDepthStencil = TRUE;
	d3dpp.AutoDepthStencilFormat = D3DFMT_D16;
	d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_ONE;
	d3dpp.BackBufferCount = 1;
	d3dpp.Flags = D3DPRESENTFLAG_LOCKABLE_BACKBUFFER;

	g_D3DDevice_ = NULL;
	if (FAILED(g_D3D_->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hwnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &d3dpp, &g_D3DDevice_)))
	{
		return false;
	}

	g_D3DDevice_->CreateOffscreenPlainSurface(width, height, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &gImageSrcLeft_, NULL);
	g_D3DDevice_->CreateOffscreenPlainSurface(width, height, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &gImageSrcRight_, NULL);
	g_D3DDevice_->CreateOffscreenPlainSurface(width * 2, height + 1, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &gImageSrcStereo_, NULL);

	return true;
}

int playerDisplay::InitD3D(bool fullscreen)
{
	//HRESULT lRet;
	InitializeCriticalSection(&m_critial_);
	Cleanup();

	D3DDISPLAYMODE displayMode;

	// Create the D3D object.
	g_D3D_ = Direct3DCreate9(D3D_SDK_VERSION);
	if (g_D3D_ == NULL)
		return -1;

	// Get the desktop display mode.
	if (FAILED(g_D3D_->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &displayMode)))
		return false;

	ZeroMemory(&d3dpp, sizeof(d3dpp));

	if (fullscreen)
	{
		d3dpp.Windowed = FALSE;
		d3dpp.BackBufferWidth = width;
		d3dpp.BackBufferHeight = height;
	}
	else
		d3dpp.Windowed = TRUE;

	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	d3dpp.BackBufferFormat = D3DFMT_A8R8G8B8;// displayMode.Format;
	d3dpp.EnableAutoDepthStencil = TRUE;
	d3dpp.AutoDepthStencilFormat = D3DFMT_D16;
	d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;
	//d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_ONE;
	d3dpp.BackBufferCount = 1;
	d3dpp.Flags = D3DPRESENTFLAG_LOCKABLE_BACKBUFFER;

	if (FAILED(g_D3D_->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hwnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &d3dpp, &g_D3DDevice_)))
	{
		return false;
	}

	g_D3DDevice_->CreateOffscreenPlainSurface(width, height, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &gImageSrcHome_, NULL);
	g_D3DDevice_->CreateOffscreenPlainSurface(width, height, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &gImageSrcSingle_, NULL);
	g_D3DDevice_->CreateOffscreenPlainSurface(width, height, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &gImageSrcLeft_, NULL);
	g_D3DDevice_->CreateOffscreenPlainSurface(width, height, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &gImageSrcRight_, NULL);
	g_D3DDevice_->CreateOffscreenPlainSurface(width * 2, height + 1, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &gImageSrcStereo_, NULL);

	return true;
}

bool playerDisplay::RenderHome()
{
	g_D3DDevice_->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_ARGB(1, 0, 0, 0), 1.0f, 0);

	g_D3DDevice_->BeginScene();

	if (!no_background)
	{
		if (!FAILED(D3DXLoadSurfaceFromFile(gImageSrcHome_, NULL, NULL, "pic.jpg", NULL, D3DX_FILTER_NONE, 0, NULL)))
			no_background = true;
	}

	g_D3DDevice_->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &gBackBuffer_);
	g_D3DDevice_->StretchRect(gImageSrcHome_, NULL, gBackBuffer_, NULL, D3DTEXF_LINEAR);

	//for ui rendering
	ui_->Render();

	g_D3DDevice_->EndScene();
	g_D3DDevice_->Present(NULL, NULL, NULL, NULL);


	return true;
}

bool playerDisplay::RenderSingleView(int index)
{
	//for decoding and transforming the data
	if (!Transform(index))
		return -1;


	g_D3DDevice_->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_ARGB(1, 0, 0, 0), 1.0f, 0);

	g_D3DDevice_->BeginScene();
	g_D3DDevice_->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &gBackBuffer_);
	g_D3DDevice_->StretchRect(gImageSrcSingle_, NULL, gBackBuffer_, NULL, D3DTEXF_LINEAR);

	//for ui rendering
	ui_->Render();

	g_D3DDevice_->EndScene();
	g_D3DDevice_->Present(NULL, NULL, NULL, NULL);

	

	return true;
}

bool playerDisplay::RenderStereoView()
{
	if (!TransformLeft())
		return -1;
	g_D3DDevice_->StretchRect(gImageSrcLeft_, &srcRect, gImageSrcStereo_, &dstRect, D3DTEXF_LINEAR);

	if (!TransformRight())
		return -1;
	g_D3DDevice_->StretchRect(gImageSrcRight_, &srcRect, gImageSrcStereo_, &dstRect2, D3DTEXF_LINEAR);

	gImageSrcStereo_->LockRect(&lr, NULL, 0);

	LPNVSTEREOIMAGEHEADER pSIH =
		(LPNVSTEREOIMAGEHEADER)(((unsigned char *)lr.pBits) + (lr.Pitch * (height)));

	pSIH->dwSignature = NVSTEREO_IMAGE_SIGNATURE;
	pSIH->dwBPP = 32;
	pSIH->dwFlags = SIH_SWAP_EYES;
	pSIH->dwWidth = width * 2;
	pSIH->dwHeight = height;

	gImageSrcStereo_->UnlockRect();

	g_D3DDevice_->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_ARGB(1, 0, 0, 0), 1.0f, 0);

	g_D3DDevice_->BeginScene();

	g_D3DDevice_->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &gBackBuffer_);
	g_D3DDevice_->StretchRect(gImageSrcStereo_, NULL, gBackBuffer_, NULL, D3DTEXF_LINEAR);

	//for ui rendering
	ui_->Render();

	g_D3DDevice_->EndScene();
	g_D3DDevice_->Present(NULL, NULL, NULL, NULL);


	return true;
}

bool playerDisplay::Transform(int index)
{
	if (!decoder_->Capture(index))
		return -1;

	gImageSrcSingle_->LockRect(&lr, NULL, 0);

	BYTE* imagedata = (BYTE *)lr.pBits;
	avpicture_fill((AVPicture *)decoder_->FrameRGB_current_[index], imagedata, AV_PIX_FMT_RGB32, width, height);
	sws_scale(decoder_->img_convert_ctx_[index], decoder_->Frame_[index]->data, decoder_->Frame_[index]->linesize, 0, decoder_->CodecCtx_[index]->height, decoder_->FrameRGB_current_[index]->data, decoder_->FrameRGB_current_[index]->linesize);

	//this code is replaced by the above code to reduce time 
	//for (int y = 0; y < height; y++)
	//	memcpy(imagedata + y*lr.Pitch, decoder_->FrameRGB_current_[index]->data[0] + y*decoder_->FrameRGB_current_[index]->linesize[0], width * 4);

	gImageSrcSingle_->UnlockRect();

	//for remuxer
	if (activeRemutexer())
	{
		decoder_->Remuxer(index);
	}

	return true;
}

bool playerDisplay::TransformLeft()
{
	int index = 0;
	if (!decoder_->Capture(index))
		return -1;

	gImageSrcLeft_->LockRect(&lr, NULL, 0);

	BYTE* imagedata = (BYTE *)lr.pBits;
	avpicture_fill((AVPicture *)decoder_->FrameRGB_current_[index], imagedata, AV_PIX_FMT_RGB32, width, height);
	sws_scale(decoder_->img_convert_ctx_[index], decoder_->Frame_[index]->data, decoder_->Frame_[index]->linesize, 0, decoder_->CodecCtx_[index]->height, decoder_->FrameRGB_current_[index]->data, decoder_->FrameRGB_current_[index]->linesize);

	//this code is replaced by the above code to reduce time 
	//for (int y = 0; y < height; y++)
	//	memcpy(imagedata + y*lr.Pitch, decoder_->FrameRGB_current_[index]->data[0] + y*decoder_->FrameRGB_current_[0]->linesize[index], width * 4);

	gImageSrcLeft_->UnlockRect();

	//for remuxer
	if (activeRemutexer())
	{
		decoder_->Remuxer(index);
	}

	return true;
}

bool playerDisplay::TransformRight()
{
	int index = 1;

	if (!decoder_->Capture(index))
		return -1;

	gImageSrcRight_->LockRect(&lr, NULL, 0);

	BYTE* imagedata = (BYTE *)lr.pBits;
	avpicture_fill((AVPicture *)decoder_->FrameRGB_current_[index], imagedata, AV_PIX_FMT_RGB32, width, height);
	sws_scale(decoder_->img_convert_ctx_[index], decoder_->Frame_[index]->data, decoder_->Frame_[index]->linesize, 0, decoder_->CodecCtx_[index]->height, decoder_->FrameRGB_current_[index]->data, decoder_->FrameRGB_current_[index]->linesize);

	//this code is replaced by the above code to reduce time 
	//for (int y = 0; y < height; y++)
	//	memcpy(imagedata + y*lr.Pitch, decoder_->FrameRGB_current_[index]->data[0] + y*decoder_->FrameRGB_current_[index]->linesize[0], width * 4);

	gImageSrcRight_->UnlockRect();

	//for remuxer
	if (activeRemutexer())
	{
		decoder_->Remuxer(index);
	}

	return true;
}

LRESULT WINAPI playerDisplay::WndProc(HWND hwnd, UINT msg, WPARAM wparma, LPARAM lparam)
{
	if (ImGui_ImplDX9_WndProcHandler(hwnd, msg, wparma, lparam))
		return true;

	switch (msg)
	{
	case WM_SIZE:
		//if (wparma != SIZE_MINIMIZED)
		if (g_D3DDevice_ != NULL && wparma != SIZE_MINIMIZED)
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

void playerDisplay::Cleanup()
{
	EnterCriticalSection(&m_critial_);

	if (g_D3DDevice_ != NULL)
		g_D3DDevice_->Release();
	if (g_D3D_ != NULL)
		g_D3D_->Release();

	g_D3DDevice_ = NULL;
	g_D3D_ = NULL;

	LeaveCriticalSection(&m_critial_);
}

void playerDisplay::Display()
{
	//Init d3d
	if (InitD3D(false))
	{
		//Init ui
		ui_->Init(hwnd, g_D3DDevice_);

		bool show_test_window = true;
		bool show_another_window = false;
		bool show_home_page = true;
		ImVec4 clear_col = ImColor(114, 144, 154);

		//ui_->names_ = { "One", "Two", "Three", "One", "Two", "Three" };
		//ui_->locations_ = { "Ottawa", "Ottawa", "Ottawa", "Ottawa", "Ottawa", "Ottawa" };
		//ui_->time_ = { "Ottawa", "Ottawa", "Ottawa", "Ottawa", "Ottawa", "Ottawa" };

		//ui_->url_.push_back(ui_->names_);
		//ui_->url_.push_back(ui_->locations_);

		//ui_->file_.push_back(ui_->names_);
		//ui_->file_.push_back(ui_->locations_);

		//ui_->file_.push_back({ "bigbuckbunny_480x272.h265", "Dracula 480p.wmv", "big buck bunny", "Ottawa" });
		ui_->url_.push_back({ "rtsp://192.168.0.100:8554/video1/unicast", "rtsp://192.168.0.100:8554/video3/unicast", "Yang Liu", "10/08/2016" });

		ShowWindow(hwnd, SW_SHOWDEFAULT);
		UpdateWindow(hwnd);

		selectURLTemp_ = ui_->selectedURL_;
		selectFileTemp_ = ui_->selectedFile_;

		while (msg.message != WM_QUIT)
		{
			if (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
				continue;
			}
			ui_->NewFrame(&show_home_page);
			
			if (!activeDecoding())
			{
				ui_->HomePage = true;
			}
			else 
			{
				if (ui_->HomePage == true)
				{
					RenderHome();
					isDecoding = false;
				}
				else if (ui_->videoMode_ == 0)
				{
					RenderSingleView(0);
				}
				else if (ui_->videoMode_ == 1)
				{
					RenderSingleView(1);
				}
				else if (ui_->videoMode_ == 2)
				{
					RenderStereoView();
				}
			}						
		}
	}
	
}

void playerDisplay::Shutdown()
{
	ui_->Shutdown();
	Cleanup();
	UnregisterClass("Player", wc.hInstance);
}

bool playerDisplay::activeDecoding()
{
	if (selectURLTemp_ != ui_->selectedURL_)
	{
		selectURLTemp_ = ui_->selectedURL_;
		selectFileTemp_ = ui_->selectedFile_ = -1;
		ui_->HomePage = false;

		if (isDecoding == true)
		{
			if (ui_->isWriteFile == true)
				decoder_->stopRemuxer();
			decoder_->stopCapture();		
		}
		isDecoding = true;

		//std::vector<std::string> url = { ui_->url_[selectURLTemp_][0], ui_->url_[selectURLTemp_][1] };
		//playerDecoder * decoder = new playerDecoder(url, width, height);
		decoder_ = new playerDecoder(ui_->url_[selectURLTemp_], width, height);
		
		return decoder_->initialSuccess_;
	}
	else if (selectFileTemp_ != ui_->selectedFile_)
	{
		selectFileTemp_ = ui_->selectedFile_;
		selectURLTemp_ = ui_->selectedURL_ = -1;
		ui_->HomePage = false;

		if (isDecoding == true)
		{
			if (ui_->isWriteFile == true)
				decoder_->stopRemuxer();
			decoder_->stopCapture();			
		}

		//std::vector<std::string> file = { ui_->file_[selectFileTemp_][0], ui_->file_[selectFileTemp_][1] };
		//playerDecoder * decoder = new playerDecoder(file, width, height);
		decoder_ = new playerDecoder(ui_->file_[selectFileTemp_], width, height);
		return decoder_->initialSuccess_;
	}
	return true;
}

bool playerDisplay::activeRemutexer()
{
	if (ui_->isWriteFile == false)//when select unwrite, it will return false
	{
		if (isWriteFile == true) //when select unwrite, but the remuxer is writing, it should stop first
		{
			decoder_->stopRemuxer();
			isWriteFile = ui_->isWriteFile;
		}			
		return false;
	}
	else //when select write, it will return true
	{
		if (isWriteFile == false) //when select write, but the remuxer is unwrite, just start the remuxer
		{
			decoder_->InitRemuxer();
			isWriteFile = ui_->isWriteFile; 
		}
		return true;
	}	
}