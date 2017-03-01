#include "../inc/controller.h"
#include "../inc/utils.h"
#include "../IMGUI/imgui.h"
#include "../IMGUI/imgui_impl_dx9.h"
#include <fstream>

#define IM_ARRAYSIZE(_ARR)  ((int)(sizeof(_ARR)/sizeof(*_ARR)))

WindowSize g_window_size;
DecodeStatus g_decode_status;
VideoInfo g_video_info;

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
	if (playlist_status_.exist) {
		PlaylistUI();
	}
	return true;
}

int Controller::Render(){
	ImGui::Render();
	return true;
}

int Controller::Shutdown(){
	ImGui_ImplDX9_Shutdown();
	return true;
}


int Controller::ReadCSV() {
	std::string address = playlist_status_.save_file_address + playlist_status_.address_end[playlist_status_.input_mode];

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

	playlist_status_.playlist_items = playlist_items_temp;

	return true;
}

int Controller::ReadCSV(std::string address) {

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

	playlist_status_.playlist_items = playlist_items_temp;

	return true;
}

int Controller::WriteCSV() {
	std::string address = playlist_status_.save_file_address + playlist_status_.address_end[playlist_status_.input_mode];
	std::ofstream file(address, std::ios::out);

	for (int i = 0; i < playlist_status_.playlist_items.size(); i++) {
		file << playlist_status_.playlist_items[i] << std::endl;
	}

	return true;
}

int Controller::WriteCSV(std::string address) {
	std::ofstream file(address, std::ios::out);
	
	for (int i = 0; i < playlist_status_.playlist_items.size(); i++){
		file << playlist_status_.playlist_items[i] << std::endl;
	}

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

	if (ImGui::Button("Home Page"))
	{
		g_decode_status.visual_status = 0;
	}
	
	if (playlist_status_.exist == 0)
	{
		if (ImGui::Button("Start Play"))
		{
			playlist_status_.exist = 1;
		}
	}
	else
	{
		if (ImGui::Button("Stop Play"))
		{
			playlist_status_.exist = 0;
		}
	}

	

	ImGui::End();

	return true;
}

int Controller::PlaylistUI() {
	ImGui::SetNextWindowSize(ImVec2(200, 100), ImGuiSetCond_FirstUseEver);
	ImGui::Begin("Playlist", &playlist_status_.exist);

	ImGui::Text("Play Item:");
	ImGui::SameLine();
	ImGui::Text(playlist_status_.select_item == -1 ? "<None>" : playlist_status_.playlist_items[playlist_status_.select_item].c_str());

	if (playlist_status_.select_item == -1||
		playlist_status_.input_mode == 2)
	{
		//if(playlist_status_.playlist_items == NULL)
		//	ReadCSV(playlist_status_.save_file_address + playlist_status_.address_end[playlist_status_.input_mode]);


		if (ImGui::Button("Network Address"))
		{
			playlist_status_.input_mode = 0;
			ReadCSV();
			ImGui::OpenPopup("network");
		}
		
		if (ImGui::BeginPopup("network"))
		{
			if (ImGui::Button("Add"))
			{
				playlist_status_.input_mode = 2;
			}
			ImGui::Separator();
			for (int i = 0; i < playlist_status_.playlist_items.size(); i++)
				if (ImGui::Selectable(playlist_status_.playlist_items[i].c_str()))
					playlist_status_.select_item = i;
			ImGui::EndPopup();
		}


		if (ImGui::Button("Save Files"))
		{
			playlist_status_.input_mode = 1;
			ReadCSV();
			ImGui::OpenPopup("file");
		}
		
		if (ImGui::BeginPopup("file"))
		{
			for (int i = 0; i < playlist_status_.playlist_items.size(); i++)
				if (ImGui::Selectable(playlist_status_.playlist_items[i].c_str()))
					playlist_status_.select_item = i;
			ImGui::EndPopup();
		}

	}

	{
		//if (playlist_status_.input_mode == 0)
		//{
		//	const char* items[] = { "AAAA", "BBBB", "CCCC", "DDDD", "EEEE", "FFFF", "GGGG", "HHHH", "IIII", "JJJJ", "KKKK" };
		//	ImGui::Combo("Network Address", &playlist_status_.select_item, playlist_status_.playlist_items, IM_ARRAYSIZE(playlist_status_.playlist_items));   // Combo using proper array. You can also pass a callback to retrieve array value, no need to create/copy an array just for that.
		//}

		//if (playlist_status_.input_mode == 1)
		//{
		//	const char* items[] = { "AAAA", "BBBB", "CCCC", "DDDD", "EEEE", "FFFF", "GGGG", "HHHH", "IIII", "JJJJ", "KKKK" };
		//	ImGui::Combo("Save Files", &playlist_status_.select_item, playlist_status_.playlist_items, IM_ARRAYSIZE(playlist_status_.playlist_items));   // Combo using proper array. You can also pass a callback to retrieve array value, no need to create/copy an array just for that.
		//}

		if (playlist_status_.input_mode == 2)
		{
			static char str0[128] = "New Network Address";
			ImGui::InputText("", str0, IM_ARRAYSIZE(str0));
			ImGui::SameLine();
			if (ImGui::Button("Add"))
			{
				playlist_status_.playlist_items.push_back(str0);
				WriteCSV();
				playlist_status_.input_mode = 0;
			}
		}
	}

	if (playlist_status_.select_item != -1)
	{
		if (ImGui::Button("Connect"))
		{
		}

		ImGui::SameLine(150);

		if (ImGui::Button("Delete"))
		{
			playlist_status_.playlist_items.erase(playlist_status_.playlist_items.begin() + playlist_status_.select_item);
			WriteCSV();
			playlist_status_.input_mode = 0;
			playlist_status_.select_item = -1;
		}

		ImGui::SameLine(300);

		if (ImGui::Button("Back"))
		{
			playlist_status_.input_mode = 0;
			playlist_status_.select_item = -1;
		}

		ImGui::Separator();
		ImGui::Text("Play Mode");
		{
			ImGui::Checkbox("Write File", &g_decode_status.write_file);
			ImGui::SameLine(150);
			ImGui::Checkbox("Multi-Thread", &g_decode_status.multi_thread);
		}

		ImGui::Text("Video Mode");
		//if (ImGui::CollapsingHeader("Video Mode"))
		{
			ImGui::RadioButton("Left Video", &g_decode_status.video_mode, 0); ImGui::SameLine(150);
			ImGui::RadioButton("Right Video", &g_decode_status.video_mode, 1); ImGui::SameLine(300);
			ImGui::RadioButton("Stereo Video", &g_decode_status.video_mode, 2);
		}

		ImGui::Text("Decode Core");
		//if (ImGui::CollapsingHeader("Decode Core"))
		{
			ImGui::RadioButton("ffmpeg core", &g_decode_status.decode_core, 0); ImGui::SameLine(150);
			ImGui::RadioButton("opencv core", &g_decode_status.decode_core, 1);
		}
	}
	
	ImGui::End();

	return true;
}