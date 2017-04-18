#include <d3d9.h>
#include <string>
#include <vector>

struct DecodeStatus;
class Controller;
class UI{
public:
	UI();
	UI(Controller *);
	~UI();

	int Init(void* hwnd, IDirect3DDevice9* device);

	int Update();

	int Render();

	int Shutdown();

	int BuildConnection(DecodeStatus * );
	int Disconnection();

	int AbortUI(char *);

private:
	int HomeUI();
	int MainUI();
	int StopUI();
	int SettingUI();
	int AbortUI();

	int ReadCSV();
	int WriteCSV();
	std::string get_current_time();
	
	Controller * controller_;

	struct Playlist {
		bool exist;
		int input_mode;
		int select_item;
		int visual_status;
		std::vector<std::string> playlist_items;
		//char** playlist_items;
		std::string save_file_address;
		std::vector<std::string> address_end = {
			"live_stream.txt",
			"save_files.txt"
		};
		Playlist() :exist(0), input_mode(0), select_item(-1), visual_status(2), save_file_address("C:\\3DPlayerSavedVideo\\") {}
	};

	Playlist * playlist_status_;
	std::vector<bool> windows_opened;
	char * abort_msg_;
};