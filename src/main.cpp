#include"playerDisplay.h"

int main(int, char**)
{
	playerDisplay * player = new playerDisplay();

	player->Display();

	player->Shutdown();
}