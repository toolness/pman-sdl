#ifdef WIN32
#pragma comment(lib, "SDL.lib")
#pragma comment(lib, "SDLmain.lib")
#endif

#include "globals.h"

#include <stdlib.h>

#include "SDL.h"
#include "SDL_endian.h"

#include "game.h"

#include "menu.h"

int main(int argc, char **argv)
{
	game_init();
	//game_set_state(&pman_game_state);
	game_set_state(&menu_game_state);
	game_run();
	game_shutdown();

	return 0;
}
