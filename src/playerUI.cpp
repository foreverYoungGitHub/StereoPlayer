#include "playerUI.h"



playerUI::playerUI()
{
}


playerUI::~playerUI()
{
}


bool playerUI::Init(void* hwnd, IDirect3DDevice9* device)
{
	return ImGui_ImplDX9_Init(hwnd, device);
}

void playerUI::Shutdown() 
{
	ImGui_ImplDX9_Shutdown();
}

void playerUI::NewFrame(bool* p_open) {
	readCSV();
	//use the NewFrame function
	ImGui_ImplDX9_NewFrame();
	MainUI(p_open);
}

void playerUI::MainUI(bool* p_open)
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
	ImGui::SetNextWindowPos(ImVec2(650, 20), ImGuiSetCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(100, 180), ImGuiSetCond_FirstUseEver);

	if (!(ImGui::Begin("Home Page"), p_open, window_flags))
	{
		ImGui::End();
		return;
	}

	//ImGui::Text("Home Page");
	if (ImGui::Button("Home Page"))
		HomePage = true;
	ImGui::SameLine(150);
	ImGui::Checkbox("Write File", &isWriteFile);

	//select video by live stream or url
	if (ImGui::CollapsingHeader("Video"))
	{
		if (ImGui::TreeNode("Live Video"))
		{
			ImGui::BeginChild("##header", ImVec2(0, ImGui::GetTextLineHeightWithSpacing() + ImGui::GetStyle().ItemSpacing.y));
			ImGui::Columns(3);
			ImGui::Text("ID"); ImGui::NextColumn();
			ImGui::Text("Name"); ImGui::NextColumn();
			ImGui::Text("Location"); ImGui::NextColumn();
			ImGui::Columns(1);
			ImGui::Separator();
			ImGui::EndChild();

			ImGui::BeginChild("##scrollingregion", ImVec2(0, 60));
			ImGui::Columns(3);
			for (int i = 0; i < url_.size(); i++)
			{
				char label[32];
				sprintf(label, "%04d", i);
				if (ImGui::Selectable(label, selectedURL_ == i, ImGuiSelectableFlags_SpanAllColumns))
				{
					selectedURL_ = i;
				}
				ImGui::NextColumn();
				ImGui::Text(url_[i][2].c_str()); ImGui::NextColumn();
				ImGui::Text(url_[i][3].c_str()); ImGui::NextColumn();
			}
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
			ImGui::Columns(1);
			ImGui::Separator();
			ImGui::EndChild();

			ImGui::BeginChild("##scrollingregion", ImVec2(0, 60));
			ImGui::Columns(3);
			for (int i = 0; i < file_.size(); i++)
			{
				char label[32];
				sprintf(label, "%02d", i);
				if (ImGui::Selectable(label, selectedFile_ == i, ImGuiSelectableFlags_SpanAllColumns))
					selectedFile_ = i;
				ImGui::NextColumn();
				ImGui::Text(file_[i][2].c_str()); ImGui::NextColumn();
				ImGui::Text(file_[i][3].c_str()); ImGui::NextColumn();
			}
			ImGui::Columns(1);
			ImGui::Separator();
			ImGui::EndChild();
			ImGui::TreePop();
		}
	}

	//mode
	{ 
		ImGui::RadioButton("Left Video", &videoMode_, 0); ImGui::SameLine(150);
		ImGui::RadioButton("Right Video", &videoMode_, 1); ImGui::SameLine(300);
		ImGui::RadioButton("Stereo Video", &videoMode_, 2);
	}

	ImGui::End();
}

void playerUI::Render()
{
	ImGui::Render();
}

bool playerUI::readCSV()
{
	std::string fileName = "C:\\3DPlayerSavedVideo\\index.csv";

	std::ifstream file(fileName, std::ios::in);
	
	if (!file)
		return -1;

	file_.clear();

	while (!file.eof()) 
	{
		std::string file_info_str;
		std::vector<std::string> file_info;


		std::getline(file, file_info_str, ',');
		file_info.push_back(file_info_str);

		std::getline(file, file_info_str, ',');
		file_info.push_back(file_info_str);

		std::getline(file, file_info_str, ',');
		file_info.push_back(file_info_str);

		std::getline(file, file_info_str);
		file_info.push_back(file_info_str);

		file_.push_back(file_info);
	}

	file.close();

	return true;

}