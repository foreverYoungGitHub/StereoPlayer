#include"displayWindow.h"

displayWindow:: displayWindow()
{

}


int displayWindow::InitD3D(HWND hwnd, bool fullscreen)
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
	//d3dpp.BackBufferFormat = D3DFMT_A8B8G8R8;
	d3dpp.BackBufferFormat = D3DFMT_A8R8G8B8;// displayMode.Format;
	d3dpp.EnableAutoDepthStencil = TRUE;
	//d3dpp.AutoDepthStencilFormat = D3DFMT_D24FS8;
	//d3dpp.hDeviceWindow = hwnd;
	d3dpp.AutoDepthStencilFormat = D3DFMT_D16;
	d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_ONE;
	d3dpp.BackBufferCount = 1;
	d3dpp.Flags = D3DPRESENTFLAG_LOCKABLE_BACKBUFFER;


	if (FAILED(g_D3D_->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hwnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &d3dpp, &g_D3DDevice_)))
	{
		return false;
	}
	//g_D3DDevice_->CreateOffscreenPlainSurface(width, height, D3DFMT_A8B8G8R8, D3DPOOL_DEFAULT, &gImageSrcLeft_, NULL);
	//g_D3DDevice_->CreateOffscreenPlainSurface(width, height, D3DFMT_A8B8G8R8, D3DPOOL_DEFAULT, &gImageSrcRight_, NULL);
	//g_D3DDevice_->CreateOffscreenPlainSurface(width * 2, height + 1, D3DFMT_A8B8G8R8, D3DPOOL_DEFAULT, &gImageSrcStereo_, NULL);
	g_D3DDevice_->CreateOffscreenPlainSurface(width, height, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &gImageSrcLeft_, NULL);
	g_D3DDevice_->CreateOffscreenPlainSurface(width, height, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &gImageSrcRight_, NULL);
	g_D3DDevice_->CreateOffscreenPlainSurface(width * 2, height + 1, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &gImageSrcStereo_, NULL);

	return true;
}

bool displayWindow::Render()
{
	g_D3DDevice_->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_ARGB(1, 0, 0, 0), 1.0f, 0);

	g_D3DDevice_->BeginScene();

	
	D3DXLoadSurfaceFromFile(gImageSrcLeft_, NULL, NULL, "left.jpg", NULL, D3DX_FILTER_NONE, 0, NULL);
	D3DXLoadSurfaceFromFile(gImageSrcRight_, NULL, NULL, "right.jpg", NULL, D3DX_FILTER_NONE, 0, NULL);

	g_D3DDevice_->StretchRect(gImageSrcLeft_, &srcRect, gImageSrcStereo_, &dstRect, D3DTEXF_LINEAR);
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

	g_D3DDevice_->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &gBackBuffer_);
	g_D3DDevice_->StretchRect(gImageSrcStereo_, NULL, gBackBuffer_, NULL, D3DTEXF_LINEAR);
	g_D3DDevice_->EndScene();
	g_D3DDevice_->Present(NULL, NULL, NULL, NULL);


	return true;
}

bool displayWindow::Render(cv::Mat left, cv::Mat right)
{
	g_D3DDevice_->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_ARGB(1, 0, 0, 0), 1.0f, 0);

	g_D3DDevice_->BeginScene();

	transformLeft(left);
	g_D3DDevice_->StretchRect(gImageSrcLeft_, &srcRect, gImageSrcStereo_, &dstRect, D3DTEXF_LINEAR);


	transformRight(right);	
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

	g_D3DDevice_->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &gBackBuffer_);
	g_D3DDevice_->StretchRect(gImageSrcStereo_, NULL, gBackBuffer_, NULL, D3DTEXF_LINEAR);
	g_D3DDevice_->EndScene();
	g_D3DDevice_->Present(NULL, NULL, NULL, NULL);


	return true;
}

LRESULT WINAPI displayWindow:: MyWndProc(HWND hwnd, UINT msg, WPARAM wparma, LPARAM lparam)
{
	switch (msg) {
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hwnd, msg, wparma, lparam);
}


void displayWindow::Cleanup()
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


void displayWindow::transformLeft(cv::Mat left)
{
	cv::Mat left_ = left;

	if (!(left_.at<uchar>(0, 0) == NULL))
	{

		D3DLOCKED_RECT lockedrect;

		RECT rc = { 0, 0, width, height };

		gImageSrcLeft_->LockRect(&lockedrect, &rc, 0);

		render(left_, lockedrect);

		//std::vector<std::thread *> renderThread;

		//for (int i = 0; i < renderLoopTimes; i++)
		//{
		//	std::thread * t;
		//	t = new std::thread(&displayWindow::renderLoop, this, left, (i/ renderLoopTimes) * height, ((i + 1)/ renderLoopTimes) * height, lockedrect);
		//	renderThread.push_back(t);
		//}

		//for (int i = 0; i < renderLoopTimes; i++)
		//{
		//	if (renderThread[i]->joinable())
		//		renderThread[i]->join();
		//}

		gImageSrcLeft_->UnlockRect();
	}
}


void displayWindow::transformRight(cv::Mat right)
{
	cv::Mat right_ = right;

	if (!(right_.empty() || right_.at<uchar>(0, 0) == NULL))
	{
		D3DLOCKED_RECT lockedrect;

		RECT rc = { 0, 0, width, height };

		gImageSrcRight_->LockRect(&lockedrect, &rc, 0);

		render(right_, lockedrect);

		//std::vector<std::thread *> renderThread;

		//for (int i = 0; i < renderLoopTimes; i++)
		//{
		//	std::thread * t;
		//	t = new std::thread(&displayWindow::renderLoop, this, right, (i / renderLoopTimes) * height, ((i + 1) / renderLoopTimes) * height, lockedrect);
		//	renderThread.push_back(t);
		//}

		//for (int i = 0; i < renderLoopTimes; i++)
		//{
		//	if (renderThread[i]->joinable())
		//		renderThread[i]->join();
		//}

		gImageSrcRight_->UnlockRect();
	}
	
}

void displayWindow::renderLoop(cv::Mat img, int height_start, int height_end, D3DLOCKED_RECT lockedrect)
{
	DWORD * imagedata = (DWORD *)lockedrect.pBits;
	cv::Mat img_ = img;

	if (!(img_.empty() || img_.at<uchar>(0, 0) == NULL)) //make sure it works well
	{
		for (int j = height_start; j < height_end; j++)
		{
			uchar* srcData = img_.ptr<uchar>(j);
			for (int i = 0; i < width; i++)
			{
				// the initial color is completely black
				// since the IDirect3DSurface9 is initialized as A8R8G8B8 it is 32 bit
				unsigned int color = 0x00000000;

				//get color from opencv
				color += *srcData++;			//blue
				color += *srcData++ * 256;		//green
				color += *srcData++ * 65536;	//red

				// put the value into image
				// index into texture, note we use the pitch and divide by
				// four since the pitch is given in bytes and there are
				// 4 bytes per dword.
				int index = j * lockedrect.Pitch / 4 + i;
				imagedata[index] = color;
			}
		}
	}		
}

void displayWindow::render(cv::Mat img, D3DLOCKED_RECT lockedrect)
{
	BYTE * imagedata = (BYTE *)lockedrect.pBits;

	cv::Mat img_ = img;

	if (!(img_.empty() || img_.at<uchar>(0, 0) == NULL)) //make sure it works well
	{
		//cv::cvtColor(img_, img_, cv::COLOR_BGR2BGRA);
		
		for (int j = 0; j < height; j++)
		{
			uchar* srcData = img_.ptr<uchar>(j);
			if (srcData != NULL)
				memcpy(imagedata + j * lockedrect.Pitch, srcData, width * 4);
		}
	}
}

//void displayWindow::transformLeft(unsigned char * frame_ptr)
//{
//	D3DLOCKED_RECT lockedrect;
//
//	RECT rc = { 0, 0, width, height };
//
//	gImageSrcLeft_->LockRect(&lockedrect, &rc, 0);
//
//	BYTE * imagedata = (BYTE *)lockedrect.pBits;
//	imagedata = frame_ptr;
//
//	gImageSrcLeft_->UnlockRect();
//}