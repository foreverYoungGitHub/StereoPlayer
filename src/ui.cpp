#include "../inc/controller.hpp"
#include "../inc/ui.hpp"
#include "../inc/viewer.hpp"
#include "../IMGUI/imgui.h"
#include "../IMGUI/imgui_impl_dx9.h"
#include <fstream>
#include <ctime>
#include <sstream>

#define IM_ARRAYSIZE(_ARR)  ((int)(sizeof(_ARR)/sizeof(*_ARR)))

UI::UI() {
	playlist_status_ = new Playlist();
	windows_opened.resize(4);
	windows_opened[0] = true;
}
UI::UI(Controller * _controller) :UI() {
	controller_ = _controller;
}

UI::~UI(){
	//Shutdown();
}

int UI::Init(void* hwnd, IDirect3DDevice9* device) {
	return ImGui_ImplDX9_Init(hwnd, device);
}

int UI::Update() {
	ImGui_ImplDX9_NewFrame();
	HomeUI();
	if (windows_opened[0]) {
		MainUI();
	}
	if (windows_opened[1]) {
		StopUI();
	}
	if (windows_opened[2]) {
		SettingUI();
	}
	if (windows_opened[3]) {
		AbortUI();
	}
	return true;
}

int UI::Render() {
	ImGui::Render();
	return true;
}

int UI::Shutdown() {
	ImGui_ImplDX9_Shutdown();
	return true;
}

/*
Add the input address and output address to the decode_status 
*/
int UI::BuildConnection(DecodeStatus * decode_status) {
	decode_status->input_address.clear();

	//something just for temp to avoid problems
	if (decode_status->visual_status == -1) {
		decode_status->visual_status = 2;
	}
	if (playlist_status_->input_mode == 1) {
		decode_status->multi_thread = false;
	}

	if (playlist_status_->input_mode == 0)
	{
		std::string address;
		if (decode_status->visual_status == 2 || decode_status->visual_status == 0) {
			address = "rtsp://" + playlist_status_->playlist_items[playlist_status_->select_item] + ":8554/video1/unicast";
			decode_status->input_address.push_back(address);
		}
		if (decode_status->visual_status == 2 || decode_status->visual_status == 1) {
			address = "rtsp://" + playlist_status_->playlist_items[playlist_status_->select_item] + ":8554/video3/unicast";
			decode_status->input_address.push_back(address);
		}
	}

	else if (playlist_status_->input_mode == 1)
	{
		std::string address;
		if (decode_status->visual_status == 2 || decode_status->visual_status == 0) {
			address = playlist_status_->save_file_address + playlist_status_->playlist_items[playlist_status_->select_item] + "_left.mp4";
			decode_status->input_address.push_back(address);
		}
		if (decode_status->visual_status == 2 || decode_status->visual_status == 1) {
			address = playlist_status_->save_file_address + playlist_status_->playlist_items[playlist_status_->select_item] + "_right.mp4";
			decode_status->input_address.push_back(address);
		}
	}

	if (decode_status->write_file == true)
	{
		decode_status->output_address.clear();

		std::string address;
		if (decode_status->visual_status == 2 || decode_status->visual_status == 0) {
			address = playlist_status_->save_file_address + get_current_time() + "_left.mp4";
			decode_status->output_address.push_back(address);
		}
		if (decode_status->visual_status == 2 || decode_status->visual_status == 1) {
			address = playlist_status_->save_file_address + get_current_time() + "_right.mp4";
			decode_status->output_address.push_back(address);			
		}

		//write csv for recorded video to the list
		playlist_status_->input_mode = 1;
		ReadCSV();
		playlist_status_->playlist_items.push_back(get_current_time());
		WriteCSV();
	}

	return true;
}

int UI::Disconnection() {
	return true;
}

int UI::ReadCSV() {
	std::string address = playlist_status_->save_file_address + playlist_status_->address_end[playlist_status_->input_mode];

	std::ifstream file(address, std::ios::in);

	if (!file)
		return -1;

	std::vector<std::string> playlist_items_temp;

	while (!file.eof())
	{
		std::string items;
		std::getline(file, items);
		if (items == "")
			continue;
		playlist_items_temp.push_back(items);
	}

	file.close();

	playlist_status_->playlist_items = playlist_items_temp;

	return true;
}

int UI::WriteCSV() {
	std::string address = playlist_status_->save_file_address + playlist_status_->address_end[playlist_status_->input_mode];
	std::ofstream file(address, std::ios::out);

	for (int i = 0; i < playlist_status_->playlist_items.size(); i++) {
		file << playlist_status_->playlist_items[i] << std::endl;
	}

	return true;
}

std::string UI::get_current_time() {
	time_t t = time(0);
	struct tm * now = localtime(&t);
	std::stringstream current_time;
	current_time << (now->tm_year + 1900) << '_'
		<< (now->tm_mon + 1) << '_'
		<< now->tm_mday << '_'
		<< now->tm_hour << '_'
		<< now->tm_min;

	return current_time.str();
}

int UI::HomeUI() {
	// the signal for window flags 
	static bool no_titlebar = true;
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
	ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiSetCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(0, 0), ImGuiSetCond_FirstUseEver);
	
	if (!(ImGui::Begin("Home"), true, window_flags))
	{
		ImGui::End();
		return false;
	}
	ImGui::End();
}

int UI::MainUI() {

	ImGui::SetNextWindowPos(ImVec2(650, 20), ImGuiSetCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(100, 180), ImGuiSetCond_FirstUseEver);

	if (!(ImGui::Begin("Main"), true))
	{
		ImGui::End();
		return false;
	}
	for (int i = 0; i < 4; i++) {
		ImGui::NewLine();
	}
	ImGui::SameLine(150);
	if (ImGui::Button("Play Live Video", ImVec2(400,150)))
	{
		playlist_status_->input_mode = 0;
		ReadCSV();
		playlist_status_->select_item = playlist_status_->playlist_items.size() - 1;

		controller_->decode_status_->visual_status = 2;
		controller_->decode_status_->write_file = false;

		controller_->BuildConnection();
		windows_opened[0] = false;
		windows_opened[1] = true;
	}

	for (int i = 0; i < 4; i++) {
		ImGui::NewLine();
	}
	ImGui::SameLine(150);
	if (ImGui::Button("Play and record", ImVec2(400, 150)))
	{
		playlist_status_->input_mode = 0;
		ReadCSV();
		playlist_status_->select_item = playlist_status_->playlist_items.size() - 1;
		
		controller_->decode_status_->visual_status = 2;
		controller_->decode_status_->write_file = true;
		//controller_->decode_status_->multi_thread = false;

		controller_->BuildConnection();
		windows_opened[0] = false;
		windows_opened[1] = true;
	}

	for (int i = 0; i < 4; i++) {
		ImGui::NewLine();
	}
	ImGui::SameLine(150);
	if (ImGui::Button("Play Record Video", ImVec2(400, 150)))
	{
		playlist_status_->input_mode = 1;
		ReadCSV();
		playlist_status_->select_item = playlist_status_->playlist_items.size() - 1;
		
		controller_->decode_status_->visual_status = 2;
		controller_->decode_status_->write_file = false;

		controller_->BuildConnection();
		windows_opened[0] = false;
		windows_opened[1] = true;
	}

	for (int i = 0; i < 4; i++) {
		ImGui::NewLine();
	}
	ImGui::SameLine(150);
	if (ImGui::Button("Setting", ImVec2(400, 150)))
	{
		windows_opened[0] = false;
		windows_opened[2] = true;
	}

	for (int i = 0; i < 4; i++) {
		ImGui::NewLine();
	}
	ImGui::SameLine(150);
	if (ImGui::Button("Exit", ImVec2(400, 150)))
	{
		controller_->~Controller();
	}

	ImGui::End();
}

int UI::StopUI() {
	ImGui::SetNextWindowPos(ImVec2(650, 20), ImGuiSetCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(100, 180), ImGuiSetCond_FirstUseEver);

	if (!(ImGui::Begin("Stop"), true))
	{
		ImGui::End();
		return false;
	}
	for (int i = 0; i < 4; i++) {
		ImGui::NewLine();
	}
	ImGui::SameLine(150);
	if (ImGui::Button("Stop Play", ImVec2(400, 150)))
	{
		controller_->Disconnection();
		windows_opened[0] = true;
		windows_opened[1] = false;
	}

	ImGui::End();
}

int UI::SettingUI() {
	ImGui::SetNextWindowPos(ImVec2(650, 20), ImGuiSetCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(100, 180), ImGuiSetCond_FirstUseEver);

	if (!(ImGui::Begin("Setting"), true))
	{
		ImGui::End();
		return false;
	}

	//ImGui::SameLine(150);
	ImGui::Text("Play Item:");
	ImGui::SameLine();
	ImGui::Text(playlist_status_->select_item == -1 ? "<None>" : playlist_status_->playlist_items[playlist_status_->select_item].c_str());

	if (playlist_status_->select_item == -1||
		playlist_status_->input_mode == 2)
	{
		ImGui::NewLine();
		ImGui::SameLine(150);
		if (ImGui::Button("Network Address", ImVec2(400, 150)))
		{
			playlist_status_->input_mode = 0;
			ReadCSV();
			ImGui::OpenPopup("network");
		}
		
		if (ImGui::BeginPopup("network"))
		{
			if (ImGui::Button("Add"))
			{
				playlist_status_->input_mode = 2;
			}
			ImGui::Separator();
			for (int i = 0; i < playlist_status_->playlist_items.size(); i++)
				if (ImGui::Selectable(playlist_status_->playlist_items[i].c_str()))
					playlist_status_->select_item = i;
			ImGui::EndPopup();
		}

		ImGui::NewLine();
		ImGui::SameLine(150);
		if (ImGui::Button("Recoded files", ImVec2(400, 150)))
		{
			playlist_status_->input_mode = 1;
			ReadCSV();
ImGui::OpenPopup("file");
		}

		if (ImGui::BeginPopup("file"))
		{
			for (int i = 0; i < playlist_status_->playlist_items.size(); i++)
				if (ImGui::Selectable(playlist_status_->playlist_items[i].c_str()))
					playlist_status_->select_item = i;
			ImGui::EndPopup();
		}

		ImGui::NewLine();
		ImGui::SameLine(150);
		if (ImGui::Button("Back to Main Menu", ImVec2(400, 150)))
		{
			playlist_status_->input_mode = 0;
			playlist_status_->select_item = -1;

			windows_opened[2] = false;
			windows_opened[0] = true;
		}
	}


	{
		if (playlist_status_->input_mode == 2)
		{
			static char str0[128] = "New Network Address";
			ImGui::InputText("", str0, IM_ARRAYSIZE(str0));
			ImGui::SameLine();
			if (ImGui::Button("Add"))
			{
				playlist_status_->playlist_items.push_back(str0);
				WriteCSV();
				playlist_status_->input_mode = 0;
			}
		}
	}

	if (playlist_status_->select_item != -1)
	{
		if (ImGui::Button("Connect", ImVec2(100, 70)))
		{
			controller_->decode_status_->visual_status = playlist_status_->visual_status;
			controller_->BuildConnection();
			windows_opened[2] = false;
			windows_opened[1] = true;
		}

		ImGui::SameLine(150);

		if (ImGui::Button("Delete", ImVec2(100, 70)))
		{
			playlist_status_->playlist_items.erase(playlist_status_->playlist_items.begin() + playlist_status_->select_item);
			WriteCSV();
			playlist_status_->input_mode = 0;
			playlist_status_->select_item = -1;
		}

		ImGui::SameLine(300);

		if (ImGui::Button("Back", ImVec2(100, 70)))
		{
			playlist_status_->input_mode = 0;
			playlist_status_->select_item = -1;
		}

		ImGui::SameLine(450);

		if (ImGui::Button("Back to Main Menu", ImVec2(130, 70)))
		{
			playlist_status_->input_mode = 0;
			playlist_status_->select_item = -1;

			windows_opened[2] = false;
			windows_opened[0] = true;
		}

		ImGui::Separator();
		ImGui::Text("Play Mode");
		{
			ImGui::Checkbox("Write File", &controller_->decode_status_->write_file);
			ImGui::SameLine(150);
			ImGui::Checkbox("Multi-Thread", &controller_->decode_status_->multi_thread);
		}

		ImGui::Text("Video Mode");
		{
			ImGui::RadioButton("Left Video", &playlist_status_->visual_status, 0); ImGui::SameLine(150);
			ImGui::RadioButton("Right Video", &playlist_status_->visual_status, 1); ImGui::SameLine(300);
			ImGui::RadioButton("Stereo Video", &playlist_status_->visual_status, 2);
		}

		//ImGui::Text("Decode Core");
		//{
		//	ImGui::RadioButton("ffmpeg core", &controller_->decode_status_->decode_core, 0); ImGui::SameLine(150);
		//	ImGui::RadioButton("opencv core", &controller_->decode_status_->decode_core, 1);
		//}
	}

	{
		if (controller_->decode_status_->write_file == true) {
			if (playlist_status_->visual_status != 2 || controller_->decode_status_->decode_core != 0) {
				controller_->decode_status_->write_file = false;
				AbortUI("Only can write file in FFMpeg with stereo mode");
			}
		}
	}

	ImGui::End();
}

int UI::AbortUI() {

	if (!(ImGui::Begin("Abort"), true))
	{
		ImGui::End();
		return false;
	}

	controller_->decode_status_->visual_status = -1;

	ImGui::Text(abort_msg_);

	if (ImGui::Button("Close", ImVec2(100, 50))) {
		
		windows_opened[3] = false;
		windows_opened[0] = true;
	}

	ImGui::End();
}

int UI::AbortUI(char * abort_msg) {

	abort_msg_ = abort_msg;

	windows_opened[3] = true;

	if (windows_opened[1] == true) {
		controller_->Disconnection();
		windows_opened[1] = false;
	}
	windows_opened[2] = false;

	return true;
}