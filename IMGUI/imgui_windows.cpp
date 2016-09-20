#if defined(_MSC_VER) && !defined(_CRT_SECURE_NO_WARNINGS)
#define _CRT_SECURE_NO_WARNINGS
#endif

#include "imgui.h"
#include <ctype.h>          // toupper, isprint
#include <math.h>           // sqrtf, powf, cosf, sinf, floorf, ceilf
#include <stdio.h>          // vsnprintf, sscanf, printf
#include <stdlib.h>         // NULL, malloc, free, qsort, atoi
#if defined(_MSC_VER) && _MSC_VER <= 1500 // MSVC 2008 or earlier
#include <stddef.h>         // intptr_t
#else
#include <stdint.h>         // intptr_t
#endif

#ifdef _MSC_VER
#pragma warning (disable: 4996) // 'This function or variable may be unsafe': strcpy, strdup, sprintf, vsnprintf, sscanf, fopen
#define snprintf _snprintf
#endif
#ifdef __clang__
#pragma clang diagnostic ignored "-Wold-style-cast"             // warning : use of old-style cast                              // yes, they are more terse.
#pragma clang diagnostic ignored "-Wdeprecated-declarations"    // warning : 'xx' is deprecated: The POSIX name for this item.. // for strdup used in demo code (so user can copy & paste the code)
#pragma clang diagnostic ignored "-Wint-to-void-pointer-cast"   // warning : cast to 'void *' from smaller integer type 'int'
#pragma clang diagnostic ignored "-Wformat-security"            // warning : warning: format string is not a string literal
#pragma clang diagnostic ignored "-Wexit-time-destructors"      // warning : declaration requires an exit-time destructor       // exit-time destruction order is undefined. if MemFree() leads to users code that has been disabled before exit it might cause problems. ImGui coding style welcomes static/globals.
#pragma clang diagnostic ignored "-Wreserved-id-macro"          // warning : macro name is a reserved identifier                // 
#elif defined(__GNUC__)
#pragma GCC diagnostic ignored "-Wint-to-pointer-cast"          // warning: cast to pointer from integer of different size
#pragma GCC diagnostic ignored "-Wformat-security"              // warning : format string is not a string literal (potentially insecure)
#pragma GCC diagnostic ignored "-Wdouble-promotion"             // warning: implicit conversion from 'float' to 'double' when passing argument to function
#pragma GCC diagnostic ignored "-Wconversion"                   // warning: conversion to 'xxxx' from 'xxxx' may alter its value
#endif

// Play it nice with Windows users. Notepad in 2015 still doesn't display text data with Unix-style \n.
#ifdef _WIN32
#define IM_NEWLINE "\r\n"
#else
#define IM_NEWLINE "\n"
#endif

#define IM_ARRAYSIZE(_ARR)  ((int)(sizeof(_ARR)/sizeof(*_ARR)))
#define IM_MAX(_A,_B)       (((_A) >= (_B)) ? (_A) : (_B))

void ImGui::ShowMainUIWindow(bool* p_open)
{
	// the signal for window flags 
	static bool no_titlebar = false;
	static bool no_border = true;
	static bool no_resize = true;
	static bool no_move = true;
	static bool no_scrollbar = false;
	static bool no_collapse = false;
	static bool no_menu = true;



	// Demonstrate the various window flags. Typically you would just use the default.
	ImGuiWindowFlags window_flags = 0;
	if (no_titlebar)  window_flags |= ImGuiWindowFlags_NoTitleBar;
	if (!no_border)   window_flags |= ImGuiWindowFlags_ShowBorders;
	if (no_resize)    window_flags |= ImGuiWindowFlags_NoResize;
	if (no_move)      window_flags |= ImGuiWindowFlags_NoMove;
	if (no_scrollbar) window_flags |= ImGuiWindowFlags_NoScrollbar;
	if (no_collapse)  window_flags |= ImGuiWindowFlags_NoCollapse;
	if (!no_menu)     window_flags |= ImGuiWindowFlags_MenuBar;
	ImGui::SetNextWindowSize(ImVec2(100, 180), ImGuiSetCond_FirstUseEver);

	if (!(ImGui::Begin("Home Page"), p_open, window_flags))
	{
		ImGui::End();
		return;
	}

	ImGui::Text("Home Page");

	if(ImGui::CollapsingHeader("Video"))
	{
		if (ImGui::TreeNode("Live Video"))
		{
			ImGui::BeginChild("##header", ImVec2(0, ImGui::GetTextLineHeightWithSpacing() + ImGui::GetStyle().ItemSpacing.y));
			ImGui::Columns(3);
			ImGui::Text("ID"); ImGui::NextColumn();
			ImGui::Text("Name"); ImGui::NextColumn();
			ImGui::Text("Location"); ImGui::NextColumn();
			//ImGui::Text("Status"); ImGui::NextColumn();
			ImGui::Columns(1);
			ImGui::Separator();
			ImGui::EndChild();

			ImGui::BeginChild("##scrollingregion", ImVec2(0, 60));
			ImGui::Columns(3);
			const char* names[6] = { "One", "Two", "Three", "One", "Two", "Three" };
			const char* locations[6] = { "Ottawa", "Ottawa", "Ottawa", "Ottawa", "Ottawa", "Ottawa" };
			//const char* status[6] = { "offline", "online", "offline", "offline", "online", "offline" };
			static int selected = -1;
			for (int i = 0; i < 6; i++)
			{
				char label[32];
				sprintf(label, "%04d", i);
				if (ImGui::Selectable(label, selected == i, ImGuiSelectableFlags_SpanAllColumns))
				{
					selected = i;
				}					
				ImGui::NextColumn();
				ImGui::Text(names[i]); ImGui::NextColumn();
				ImGui::Text(locations[i]); ImGui::NextColumn();
				//ImGui::Text(status[i]); ImGui::NextColumn();
			}
			//if (selected = 1)
			//	int c = 1;
			ImGui::Columns(1);
			ImGui::Separator();
			ImGui::EndChild();
			ImGui::TreePop();
		}

		if (ImGui::TreeNode("Saved Video"))
		{
			ImGui::BeginChild("##header", ImVec2(0, ImGui::GetTextLineHeightWithSpacing() + ImGui::GetStyle().ItemSpacing.y));
			ImGui::Columns(3);
			ImGui::Text("ID"); ImGui::NextColumn();
			ImGui::Text("Name"); ImGui::NextColumn();
			ImGui::Text("Time"); ImGui::NextColumn();
			//ImGui::Text("Status"); ImGui::NextColumn();
			ImGui::Columns(1);
			ImGui::Separator();
			ImGui::EndChild();

			ImGui::BeginChild("##scrollingregion", ImVec2(0, 60));
			ImGui::Columns(3);
			const char* names[6] = { "One", "Two", "Three", "One", "Two", "Three" };
			const char* Time[6] = { "Ottawa", "Ottawa", "Ottawa", "Ottawa", "Ottawa", "Ottawa" };
			//const char* status[6] = { "offline", "online", "offline", "offline", "online", "offline" };
			static int selected = -1;
			for (int i = 0; i < 6; i++)
			{
				char label[32];
				sprintf(label, "%04d", i);
				if (ImGui::Selectable(label, selected == i, ImGuiSelectableFlags_SpanAllColumns))
					selected = i;
				ImGui::NextColumn();
				ImGui::Text(names[i]); ImGui::NextColumn();
				ImGui::Text(Time[i]); ImGui::NextColumn();
			}
			ImGui::Columns(1);
			ImGui::Separator();
			ImGui::EndChild();
			ImGui::TreePop();
		}
	}


	//mode
	{
		static int videoMode_ = 0;

		ImGui::RadioButton("Left Video", &videoMode_, 0); ImGui::SameLine(150);
		ImGui::RadioButton("Right Video", &videoMode_, 1); ImGui::SameLine(300);
		ImGui::RadioButton("Stereo Video", &videoMode_, 2);

	}
	ImGui::End();
}