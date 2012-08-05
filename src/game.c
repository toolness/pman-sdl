#include "globals.h"

#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <time.h>

#include "SDL.h"
#include "SDL_endian.h"

#include "game.h"
#include "font.h"
#include "state.h"
#include "debug.h"
#include "audio.h"

/* SDL surface pointing to the game screen.  This is the primary surface
   that everything is ultimately displayed to. */
static SDL_Surface *g_game_screen;

/* Structure containing time-tracking info to record framerate, passage
   of time, etc. */
static Time g_game_time;

/* Pointer to the current pluggable game state. */
static const GameState *g_game_state;

/* A bunch of bitwise flags that tell us what conditions to draw the
   current frame under.  See the GAME_DRAW_FLAG_* constants. */
static int g_draw_flags;

/* Flag tells us whether we should quit after processing the
   current frame. */
static int g_quit_flag;

/* Flag tells us whether to display framerate information. */
static int g_show_stats;

/* Pointer to the game's "big" font. */
static Font g_game_font_big;

/* Pointer to the game's "small" font. */
static Font g_game_font_small;

/* Records whether the game state needs to be changed or not. */
static int g_state_change_flag;

/* Whether the game is currently in fullscreen or windowed mode. */
static int g_is_fullscreen;

/* Pointer to the game state that we need to change to, if
   g_state_change_flag is true. */
static GameState *g_next_game_state;

/* List of all the rectangles to update on the screen.  This is 
   processed through if the game doesn't have to redraw the entire
   frame.  (Part of the "dirty rectangle" animation method.) */
static RectList g_update_rects;

/* Resets the given rectangle list. */
void rect_list_reset(RectList *u)
{
	u->numrects = 0;
}

/* Adds a rectangle to the given rectangle list. */
void rect_list_add(RectList *u, SDL_Rect *r)
{
	assert(u->numrects < GAME_MAX_UPDATE_RECTS);
	u->rects[u->numrects] = *r;
	u->numrects++;
}

/* Add a rectangle to the game's list of drawing update rectangles.  This is used
   for dirty rectangle animation; whenever a drawable game token has changed its
   appearance/position/etc and needs to be redrawn, it calls this function with its
   bounding drawing rectangle and the game will update that part of the display
   during the next frame redraw. */
void game_update_rect_add(SDL_Rect *r)
{
	rect_list_add(&g_update_rects, r);
}

/* Sets the game's video mode.  Takes into account whether the game is fullscreen or not. */
void game_set_video_mode()
{
	int flags = SDL_SWSURFACE;
	int depth = SDL_VideoModeOK(SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_DEPTH, flags);

	if (g_is_fullscreen) {
		flags = flags | SDL_FULLSCREEN;
		if (g_game_screen) depth = g_game_screen->format->BitsPerPixel;
	}
	g_game_screen = SDL_SetVideoMode(SCREEN_WIDTH, SCREEN_HEIGHT, depth, flags);
	if ( g_game_screen == NULL ) {
		err("couldn't init video.\n", 1);
	}
	game_set_draw_flags(GAME_DRAW_FLAG_REDRAW);
}

/* Toggles the game between fullscreen to windowed mode. */
void game_toggle_fullscreen()
{
	if (g_is_fullscreen) {
		g_is_fullscreen = 0;
	} else {
		g_is_fullscreen = 1;
	}
	game_set_video_mode();
}

/* Restart info about passage of time and framerate statistics. */
void time_restart(Time *t)
{
	t->ticks_start = SDL_GetTicks();;
	t->ticks_end = 0;
	t->frame_time = 0;
	t->seconds_time = 0;
	t->frames_this_second = 0;
	t->fps = 0;
}

/* Update passage of time and framerate statistics. */
void time_update(Time *t)
{
	t->ticks_end = SDL_GetTicks();
	t->frame_time = t->ticks_end - t->ticks_start;
	t->seconds_time += t->frame_time;
	if (t->seconds_time > 1000) {
		t->fps = t->frames_this_second;
		t->frames_this_second = 0;
		t->seconds_time = 0;
	} else {
		t->frames_this_second++;
	}
	t->ticks_start = t->ticks_end;
	/* Now that we've made the FPS calculation, retroactively truncate
	   the frame time to its max if needed. */
	if (t->frame_time > GAME_MAX_FRAME_TIME) t->frame_time = GAME_MAX_FRAME_TIME;
	state_timer_update(TIMER_ID_GAME, t->frame_time);
}

/* Returns the game's default "big" font.  Currently, this is a 12x16 font. */
Font *game_get_font_big()
{
	return &g_game_font_big;
}

/* Returns the game's default "small" font.  Currently, this is a 6x8 font. */
Font *game_get_font_small()
{
	return &g_game_font_small;
}

/* Sets the game draw flags to the given flags.  Should be used, for instance, by a game state
   whenever it wants the game to redraw its screen entirely, instead of using dirty rectangle
   animation.  See the GAME_DRAW_FLAG_* constants. */
void game_set_draw_flags(int flags)
{
	g_draw_flags = flags;
}

/* Queues a change in the game state.  Game state is changed
   in a similar way as the current state of an FSM, in state.c. */
void game_set_state(const GameState *gs)
{
	g_state_change_flag = 1;
	/* Typecast the gs so it's not a const anymore */
	g_next_game_state = (GameState *) gs;
}

/* Executes a change in the game state. */
void game_change_state()
{
	if (g_state_change_flag) {
		assert(g_next_game_state != NULL);
		if (g_game_state) {
			g_game_state->on_exit();
		}

		/* Reinitialize the FSM subsystem and the state message routing
		   subsystem. */
		state_shutdown();
		state_init();

		g_game_state = g_next_game_state;
		g_game_state->on_enter();
		game_set_draw_flags(GAME_DRAW_FLAG_REDRAW);
		g_state_change_flag = 0;
		g_next_game_state = NULL;
	}
}

/* Returns true if the game quit flag has been activated. */
int is_game_quit()
{
	return g_quit_flag;
}

/* Quit the game.  This should be called by a game state when it wants the game
   to quit, so that everything is properly shut down upon exit.  This is done by
   setting the game quit flag to TRUE; the game loop checks the value of this flag
   each time around, and if it has been activated, the game deinitializes. */
void game_quit()
{
	g_quit_flag = 1;
}

/* Initialize the game. */
void game_init()
{
	SDL_Init(SDL_INIT_VIDEO);

	g_game_screen = NULL;
	g_game_state = NULL;
	g_quit_flag = 0;
	g_show_stats = 0;
	g_state_change_flag = 0;
	g_next_game_state = NULL;
	g_is_fullscreen = 0;

	state_init();
	if ( SDL_Init( SDL_INIT_VIDEO ) < 0) {
		err("couldn't init SDL.\n", 1);
	}

	game_set_video_mode();

	font_init(&g_game_font_big, GAME_FONT_BIG_FILENAME, GAME_FONT_BIG_CHAR_WIDTH,
		GAME_FONT_BIG_CHAR_HEIGHT, GAME_FONT_BIG_CHARS_PER_LINE);
	font_init(&g_game_font_small, GAME_FONT_SMALL_FILENAME, GAME_FONT_SMALL_CHAR_WIDTH,
		GAME_FONT_SMALL_CHAR_HEIGHT, GAME_FONT_SMALL_CHARS_PER_LINE);

	SDL_EnableKeyRepeat(500,500);
	srand( (unsigned) time(NULL) );

	audio_init();
}

/* Display framerate statistics to the upper-left hand corner of the given surface. */
void game_print_stats(SDL_Surface *surface)
{
	Font *f = game_get_font_small();
	char buffer[200];
	int x = 1;
	int y = 1;

	font_draw_string_opaque(f, surface, x, y, PACKAGE_STRING);

	y += f->char_height + 2;

	sprintf(buffer, "fps: %6d", g_game_time.fps);
	font_draw_string_opaque(f, surface, x, y, buffer);
}

/* Draw the current frame. */
void game_draw_frame(int game_view_flags)
{
		/* Lock the game screen surface for modification by pixel-level drawing
		   routines. */
		if ( SDL_MUSTLOCK(g_game_screen) ) {
			if ( SDL_LockSurface(g_game_screen) < 0) {
				err("Couldn't lock surface screen\n", 1);
			}
		}

		/* If we have to redraw the whole screen, first clear it so
		   no artifacts from the last frame are left over. */
		if (game_view_flags & GAME_DRAW_FLAG_REDRAW)
			SDL_FillRect(g_game_screen, NULL, 0);

		/* Tell the current game state to draw itself. */
		g_game_state->view(g_game_screen, game_view_flags);

		/* Display framerate statistics if we need to. */
		if (g_show_stats) {
			game_print_stats(g_game_screen);
		}

		/* Unlock the game screen surface. */
		if ( SDL_MUSTLOCK( g_game_screen ) ) {
			SDL_UnlockSurface(g_game_screen);
		}

		/* If we're redrawing the screen from scratch, blit the whole screen to the
		   surface by calling SDL_Flip().  Otherwise, do dirty rectangle animation by
		   only updating our list of updated rectangles. */
		if (game_view_flags & GAME_DRAW_FLAG_REDRAW) {
			SDL_Flip(g_game_screen);
		} else {
			SDL_UpdateRects(g_game_screen, g_update_rects.numrects, g_update_rects.rects);
		}
}

/* Toggle the display of framerate statistics. */
void game_toggle_show_stats()
{
	if (g_show_stats) {
		g_show_stats = 0;
		game_set_draw_flags(GAME_DRAW_FLAG_REDRAW);
	} else
		g_show_stats = 1;
}

/* Run the game.  This is the main "game loop". */
void game_run()
{
	time_restart(&g_game_time);

	game_set_draw_flags(GAME_DRAW_FLAG_REDRAW);

	while ( !is_game_quit() ) {
		SDL_Event event;

		/* Clear the game update rect list. */
		rect_list_reset(&g_update_rects);

		/* If the game state has changed, switch it now. */
		game_change_state();

		/* Process input using a basic "chain of command" pattern. */
		if (SDL_PollEvent(&event)) {
			/* If the game state's controller doesn't handle the input,
			   we'll deal with it ourselves. */
			if (!g_game_state->controller(&event)) {
				switch (event.type) {
					/* If the user presses "f", show framerate info. */
					case SDL_KEYDOWN:
						switch (event.key.keysym.sym) {
							case SDLK_f:
								/* If the user pressed CTRL-F, toggle fullscreen mode,
								   otherwise just toggle display of the game's
								   framerate statistics. */
								if (SDL_GetModState() & KMOD_CTRL) {
									game_toggle_fullscreen();
								} else {
									game_toggle_show_stats();
								}
								break;
						}
						break;
					/* If the user uses the OS or GUI to stop the program,
					   e.g. by clicking the game window's close box, then quit. */
					case SDL_QUIT:
						game_quit();
				}
			}
		}

		/* Process the model of the current game state. */
		g_game_state->model( g_game_time.frame_time );

		/* Have the message routing subsystem process any messages
		   to FSM's. */
		state_process_messages();

		/* Draw the current frame. */
		game_draw_frame(g_draw_flags);
		game_set_draw_flags(0);

		/* Update framerate and other "passage of time"-related information. */
		time_update(&g_game_time);
	}
}

/* Shut down the game. */
void game_shutdown()
{
	audio_shutdown();

	game_set_state(NULL);

	state_shutdown();

	font_destroy(&g_game_font_big);
	font_destroy(&g_game_font_small);

	SDL_FreeSurface(g_game_screen);

	SDL_Quit();
}

/* Create and return a bitmap with the given width, height, SDL flags, and
   the game's current pixel format. */
SDL_Surface *game_create_bitmap(Uint32 flags, int w, int h)
{
	SDL_PixelFormat *pf;

	pf = g_game_screen->format;
	return SDL_CreateRGBSurface(flags, w, h, SCREEN_DEPTH,
		pf->Rmask, pf->Gmask, pf->Bmask, pf->Amask);
}

/* Return the given RGB color in the game's current pixel format. */
Uint32 game_map_rgb(Uint8 r, Uint8 g, Uint8 b)
{
	return SDL_MapRGB(g_game_screen->format, r, g, b);
}

/* Load the given Windows BMP file and return a pointer to its surface. */
SDL_Surface *game_load_bmp(const char *filename)
{
  SDL_Surface *result;

  result = SDL_LoadBMP(filename);
  if (!result) {
	err("Couldn't load BMP.\n", 1);
  }
  return result;
}
