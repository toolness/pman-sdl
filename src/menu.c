#include "globals.h"

#include <stdlib.h>
#include <assert.h>

#include "SDL.h"

#include "game.h"
#include "font.h"
#include "drawing.h"
#include "debug.h"
#include "pman_agent_pman.h"
#include "pman.h"
#include "menu.h"
#include "hiscore.h"

const GameState menu_game_state = { menu_model, menu_view, menu_controller, menu_init, menu_shutdown };

static int g_demo_timeout_countdown;

void menu_reset_demo_timeout_countdown()
{
	g_demo_timeout_countdown = MENU_DEMO_TIMEOUT;
}

void menu_construct(Menu *m)
{
	m->title = game_load_bmp(m->title_bmp);
	assert(m->title != NULL);
}

void menu_destroy(Menu *m)
{
	SDL_FreeSurface(m->title);
}

void menu_start_game()
{
	game_set_state(&pman_game_state);
}

void menu_hi_scores()
{
	game_set_state(&hiscore_game_state);
}

void menu_quit_game()
{
	game_quit();
}

static MenuOption g_menu_main_options[] = {
	{ "Start Game", menu_start_game },
	{ "Hi Scores", menu_hi_scores },
	{ "Quit Game", menu_quit_game }
};

static Menu g_menu_main = {
	MENU_MAIN_TITLE_BMP,
	NULL,
	MENU_MAIN_START_Y,
	MENU_MAIN_TITLE_SPACING,
	MENU_MAIN_NUM_OPTIONS,
	MENU_MAIN_OPTION_X, 
	MENU_MAIN_OPTION_SPACING,
	0,
	g_menu_main_options
};

static Menu *g_curr_menu;

void menu_init()
{
	g_curr_menu = &g_menu_main;
	menu_construct(g_curr_menu);
	menu_reset_demo_timeout_countdown();
}

void menu_shutdown()
{
	menu_destroy(g_curr_menu);
}

void menu_view(SDL_Surface *surface, int game_view_flags)
{
	int curr_y = g_curr_menu->start_y;

	Font *f = game_get_font_big();
	int i;
	SDL_Rect cursor_rect;
	int cursor_radius = f->char_height / 2;
	int cursor_mouth = cursor_radius / 2;
	SDL_Rect title_rect;
	int redraw = game_view_flags & GAME_DRAW_FLAG_REDRAW;

	//font_draw_string_centered(f, surface, x, curr_y, g_curr_menu->title);
	title_rect.x = (Sint16) ((SCREEN_WIDTH / 2) - (g_curr_menu->title->w / 2));
	title_rect.y = (Sint16) curr_y;
	title_rect.w = (Uint16) g_curr_menu->title->w;
	title_rect.h = (Uint16) g_curr_menu->title->h;

	if (redraw) SDL_BlitSurface(g_curr_menu->title, NULL, surface, &title_rect);

	curr_y += g_curr_menu->title_spacing;

	cursor_rect.w = cursor_rect.h = (Uint16) (cursor_radius * 2 + 1);
	cursor_rect.x = (Sint16) g_curr_menu->option_x - cursor_rect.w - 5;

	for (i = 0; i < g_curr_menu->num_options; i++) {
		cursor_rect.y = (Sint16) curr_y;
		if (g_curr_menu->curr_option == i) {			
			pman_draw(surface, cursor_rect.x + cursor_radius, cursor_rect.y + cursor_radius, cursor_radius, cursor_mouth, 0, DIRECTION_RIGHT, game_map_rgb(255, 255, 0));
		} else {
			SDL_FillRect(surface, &cursor_rect, 0);
		}

		game_update_rect_add(&cursor_rect);

		if (redraw) font_draw_string(f, surface, g_curr_menu->option_x, curr_y, g_curr_menu->options[i].name);
		curr_y += g_curr_menu->option_spacing;
	}

	if (redraw) font_draw_string_centered(f, surface, SCREEN_WIDTH / 2, SCREEN_HEIGHT - f->char_height-2, "(c) 2003 by Atul Varma");
}

void menu_model(Uint32 frame_time)
{
	/* Decrement the demo timeout counter, and if it reaches 0, then start the game
	   demo/attract mode. */
	g_demo_timeout_countdown -= frame_time;
	if (g_demo_timeout_countdown <= 0) {
		game_set_state(&pman_demo_game_state);
	}
}

void menu_move_curr_option_up(Menu *m)
{
	m->curr_option--;
	if (m->curr_option < 0) m->curr_option = m->num_options-1;
}

void menu_move_curr_option_down(Menu *m)
{
	m->curr_option++;
	if (m->curr_option >= m->num_options) m->curr_option = 0;
}

void menu_execute_curr_option(Menu *m)
{
	m->options[m->curr_option].action();
}

int menu_controller(SDL_Event *e)
{
	if (e->type == SDL_KEYDOWN) {
		menu_reset_demo_timeout_countdown();
		switch (e->key.keysym.sym) {
			case SDLK_UP:
				menu_move_curr_option_up(g_curr_menu);
				return 1;
				break;

			case SDLK_DOWN:
				menu_move_curr_option_down(g_curr_menu);
				return 1;
				break;

			case SDLK_RETURN:
				menu_execute_curr_option(g_curr_menu);
				return 1;
				break;
		}
	}
	return 0;
}
