#include <stdio.h>
#include <tchar.h>
#include <d3d9.h>
#include <d3dx9.h>
#include <Windows.h>
#include <thread>

#include "displayWindow.h"
#include "captureVideo.hpp"


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	std::string URL = "D:\\YangLiu\\Project\\bbb_sunflower_1080p_60fps_stereo_abl.mp4";

	std::string URLLeft = "rtsp://192.168.0.100:8554/video1/unicast";
	std::string URLRight = "rtsp://192.168.0.100:8554/video3/unicast";

	//vector<string> capture_source = {
	//	"rtsp://192.168.0.100:8554/video1/unicast",
	//	"rtsp://192.168.0.100:8554/video3/unicast"
	//};

	//vector<string> capture_source = {
	//	"D:\\YangLiu\\Project\\bbb_sunflower_1080p_60fps_stereo_abl.mp4"
	//	//"rtsp://192.168.0.100:8554/unicast",
	//	//"rtsp://192.168.0.100:8554/unicast"
	//};

	vector<int> capture_source = {
		1,
		0
	};

	displayWindow displayWindow_;
	WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_CLASSDC, (WNDPROC)displayWindow_.MyWndProc, 0L, 0L,
		GetModuleHandle(NULL), NULL, NULL, NULL, NULL,
		WINDOW_CLASS, NULL };

	RegisterClassEx(&wc);

	HWND hwnd = CreateWindow(WINDOW_CLASS, WINDOW_NAME, WS_OVERLAPPEDWINDOW,
		0, 0, displayWindow_.width, displayWindow_.height, GetDesktopWindow(), NULL,
		wc.hInstance, NULL);
	if (hwnd == NULL) {
		return -1;
	}

	

	captureVideo captureVideo_(capture_source);
	
	//captureVideo_.realtime = false;

	if (displayWindow_.InitD3D(hwnd, false))
	{
		ShowWindow(hwnd, SW_SHOWDEFAULT);
		UpdateWindow(hwnd);

		MSG msg;
		ZeroMemory(&msg, sizeof(msg));

		while (msg.message != WM_QUIT)
		{
			if (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			else
			{
				if ((captureVideo_.frame_ptr_[0]->unsafe_size() == 0) ||
					(captureVideo_.frame_ptr_[1]->unsafe_size() == 0) )					)
				{
					unsigned char * leftImg, rightImg;
					captureVideo_.frame_ptr_[0]->try_pop(leftImg);
					captureVideo_.frame_ptr_[1]->try_pop(rightImg);
					displayWindow_.Render(leftImg, rightImg);
				}
				else
					continue;
				// if (captureVideo_.realtime)
				// {
				// 	if(captureVideo_.writeState_[0] == false && captureVideo_.writeState_[1] == false) //make sure the mat is finishing writing
				// 	{
				// 		displayWindow_.Render(captureVideo_.frames_[0], captureVideo_.frames_[1]);
				// 	}					
				// }					
				// else
				// {
				// 	Mat leftImg, rightImg;
				// 	if (captureVideo_.camera_count == 1)
				// 	{
				// 		captureVideo_.frame_queue[0]->try_pop(leftImg);
				// 		captureVideo_.frame_queue[0]->try_pop(rightImg);
				// 	}
				// 	else
				// 	{
				// 		if ((captureVideo_.frame_queue[0]->unsafe_size() != 0) && (captureVideo_.frame_queue[1]->unsafe_size() != 0))
				// 		{						
				// 			captureVideo_.frame_queue[0]->try_pop(leftImg);
				// 			captureVideo_.frame_queue[1]->try_pop(rightImg);
				// 		}
				// 		else
				// 			continue;
				// 	}

				// 	displayWindow_.Render(leftImg, rightImg);
				// }
			}
				
		}
	}

	displayWindow_.Cleanup();
	UnregisterClass("D3D", hInstance);
	return 0;
}

