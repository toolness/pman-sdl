#ifndef INCLUDE_MENU
#define INCLUDE_MENU

/* menu.h

   Functions and data for the game's main menu.
   Atul Varma - 7/30/2003

   Some generic architecture has been built in the case that the game
   might want more than one menu (e.g., an options menu), although this
   currently isn't the case (there's only one menu).
*/

#include "game.h"

/* Maximum length, in characters, of a menu's title and any menu options. */
#define MENU_NAME_MAXLENGTH 50

/* The title of the main menu. */
#define MENU_MAIN_TITLE_BMP "pman_logo02.bmp"

/* The y-coordinate where the main menu starts. */
#define MENU_MAIN_START_Y 100

/* y-distance in pixels between the main menu title and the menu options. */
#define MENU_MAIN_TITLE_SPACING 150

/* Number of options in the main menu. */
#define MENU_MAIN_NUM_OPTIONS 3

/* y-distance between each menu option. */
#define MENU_MAIN_OPTION_SPACING 30

/* x-coordinate of each menu option. */
#define MENU_MAIN_OPTION_X ( (SCREEN_WIDTH / 2) - 50 )

/* Delay (in ms) until the game goes into "demo" (aka attract) mode, if the
   user doesn't press any keys. */
#define MENU_DEMO_TIMEOUT 10000

/* The MenuOption structure describes each option in a menu. */
typedef struct MenuOption {
	/* The displayed name of the menu option. */
	char name[MENU_NAME_MAXLENGTH];
	/* Pointer to a function that is called when the menu option
	   is pressed. */
	void (*action)();
} MenuOption;

/* The Menu structure describes a menu. */
typedef struct Menu {
	/* The displayed title of the menu. */
	char title_bmp[MENU_NAME_MAXLENGTH];
	/* Surface pointing to title bitmap. */
	SDL_Surface *title;
	/* Starting y coordinate of the menu (where the title is displayed. */
	int start_y;
	/* y-distance between the title and the menu options. */
	int title_spacing;
	/* Number of menu options. */
	int num_options;
	/* x-coordinate of each menu option. */
	int option_x;
	/* y-distance between each menu option. */
	int option_spacing;
	/* The currently selected option in the menu. */
	int curr_option;
	/* An array of all the options in the menu.  The array has
	   num_options elements. */
	MenuOption *options;
} Menu;

extern const GameState menu_game_state;

void menu_init();
void menu_shutdown();
void menu_view(SDL_Surface *surface, int game_view_flags);
void menu_model(Uint32 frame_time);
int menu_controller(SDL_Event *e);

#endif
