#ifndef INCLUDE_GAME
#define INCLUDE_GAME

/* game.h

   Game logic for main gameplay state.

   This module functions like a Singleton class that stores references to game
   objects such as pac man, ghosts, the game board, etc, and coordinates between
   those game objects.
*/

#include "SDL.h"

#include "font.h"
#include "state.h"

#define SCREEN_WIDTH  640
#define SCREEN_HEIGHT 480
#define SCREEN_DEPTH  16

/* Timer ID for the global game timer.  This timer keeps track of the
   total ticks passed in the game since it started.  the value of this variable
   is similar to the SDL_GetTicks() function, except that if a frame took longer than
   GAME_MAX_FRAME_TIME to generate, GAME_MAX_FRAME_TIME is tacked on to this
   variable (so it's not necessarily the actual number of ms passed since the game
   started). */
#define TIMER_ID_GAME 0

/* Maximum time (in ms) between each frame.  Note that
   the actual time per frame can be more than this, but the
   pman game module "pretends" that only this much time has
   passed. */
#define GAME_MAX_FRAME_TIME 200

/* GAME_DRAW_FLAG_* constants are passed to any game state's view() function and
   give it information about how to draw the view. */

/* This flag tells the game state's view() function to completely redraw the
   game's view. */
#define GAME_DRAW_FLAG_REDRAW 1

/* Information about the game's default "big size" font. */
#define GAME_FONT_BIG_FILENAME "pman_font02.bmp"
#define GAME_FONT_BIG_CHAR_WIDTH (6*2)
#define GAME_FONT_BIG_CHAR_HEIGHT (8*2)
#define GAME_FONT_BIG_CHARS_PER_LINE 16

/* Information about the game's default "small size" font. */
#define GAME_FONT_SMALL_FILENAME "pman_font01.bmp"
#define GAME_FONT_SMALL_CHAR_WIDTH (6)
#define GAME_FONT_SMALL_CHAR_HEIGHT (8)
#define GAME_FONT_SMALL_CHARS_PER_LINE 16

/* Maximum number of update rectangles for dirty rectangle
   animation. */
#define GAME_MAX_UPDATE_RECTS 100

/* The Time structure contains time-tracking statistics for the game,
   including frames per second and the amount of game "ticks" that have
   passed so far.  These variables and the code associated with this structure
   rely on the SDL_GetTicks() function to determine the passage of time. */
typedef struct Time {
	/* used internally.  ticks_start and ticks_end are the ticks at the
	   beginning of a frame, respectively. */
	Uint32 ticks_start, ticks_end;
	/* the amount of time it's taken to render the current frame.  the maximum amount of
	   time this can be is GAME_MAX_FRAME_TIME, even if the game actually took longer
	   to draw the frame. */
	Uint32 frame_time;
	/* used internally.  this tracks the number of milliseconds elapsed so far this second. */
	Uint32 seconds_time;
	/* used internally.  number of frames that have been rendered so far this second. */
	Uint32 frames_this_second;
	/* number of frames rendered in the last second (frames per second). */
	Uint32 fps;
} Time;

typedef void (*GameStateModel)(Uint32);
typedef void (*GameStateView)(SDL_Surface *, int);
typedef int (*GameStateController)(SDL_Event *);
typedef void (*GameStateEnter)();
typedef void (*GameStateExit)();

/* Encapsulates the "state" of a game at a high level of abstraction.  This is similar to
   the State pattern as described in the GoF text.  It also implements the
   Model-View-Controller architectural pattern.
   
   For instance, one State object could be used to interface to a game's main menu,
   anther State could be used to interface to a game's credits, another to the game's
   main play mode, etc... */
typedef struct GameState {
	/* Tells the game state to do its logic stuff. */
	GameStateModel model;
	/* Tells the game state to draw itself. */
	GameStateView  view;
	/* Events are sent to the controller. */
	GameStateController controller;
	/* Called whenever a state is entered. */
	GameStateEnter on_enter;
	/* Called whenever a state is exited. */
	GameStateExit  on_exit;
} GameState;

/* Structure to store a list of rectangles.  This is used to
   store the list of rectangles to update for dirty rectangle
   animation. */
typedef struct RectList {
	/* The list of rectangles, stored sequentially in an array. */
	SDL_Rect rects[GAME_MAX_UPDATE_RECTS];
	/* The number of rectangles currently in the list (i.e., the
	   current length of the list). */
	int numrects;
} RectList;

void game_set_state(const GameState *gs);
SDL_Surface *game_create_bitmap(Uint32 flags, int w, int h);
Uint32 game_map_rgb(Uint8 r, Uint8 g, Uint8 b);

void game_init();
void game_run();
void game_shutdown();

int is_game_quit();
void game_quit();
void game_set_draw_flags(int flags);
Uint32 game_get_ticks();

Font *game_get_font_big();
Font *game_get_font_small();

void game_update_rect_add(SDL_Rect *r);

SDL_Surface *game_load_bmp(const char *filename);

#endif
