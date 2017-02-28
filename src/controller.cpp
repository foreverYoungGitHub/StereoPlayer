#include "../inc/controller.h"

#include "../IMGUI/imgui.h"
#include "../IMGUI/imgui_impl_dx9.h"

Controller::Controller(){

}

Controller::~Controller() {

}

int Controller::Init(void* hwnd, IDirect3DDevice9* device) {
	return ImGui_ImplDX9_Init(hwnd, device);
}

int Controller::Update() {
	ImGui_ImplDX9_NewFrame();
	MainUI();
	return true;
}

int Controller::Render()
{
	ImGui::Render();
	return true;
}

int Controller::Shutdown()
{
	ImGui_ImplDX9_Shutdown();
	return true;
}

int Controller::MainUI() {
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
	ImGui::SetNextWindowPos(ImVec2(650, 20), ImGuiSetCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(100, 180), ImGuiSetCond_FirstUseEver);

	if (!(ImGui::Begin("Home Page"), true, window_flags))
	{
		ImGui::End();
		return false;
	}



	ImGui::End();

	return true;
}