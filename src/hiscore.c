#include "globals.h"

#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <ctype.h>

#include "SDL.h"

#include "game.h"
#include "font.h"
#include "debug.h"
#include "menu.h"
#include "hiscore.h"

const GameState hiscore_game_state = { hiscore_model, hiscore_view, hiscore_controller, hiscore_init, hiscore_shutdown };

/* List of hi scores. */
static HiScoreList g_hiscores;

/* If we need to test the hi score list against a score after the player
   lost the game, this is the score to test against. */
static int g_hiscore_test_score = 0;

/* If there is a new hiscore, this is its position + 1.  If there is no
   new hiscore, this is 0. */
static int g_new_hiscore_position;

/* If the hiscore list is only supposed to display for a given amount of
   time and then quit to the main menu, this records the number of milliseconds
   left to display for.  If the hiscore list doesn't need to countdown, this is
   -1. */
static int g_hiscore_countdown = -1;

/* Removes the trailing underscore ("_") from a new hiscore entry. */
void hiscore_new_entry_finalize()
{
	if (g_new_hiscore_position) {
		char *str = g_hiscores.entries[g_new_hiscore_position-1].name;

		int str_length = (int) strlen(str);

		str[str_length-1] = '\0';

		g_new_hiscore_position = 0;
		SDL_EnableUNICODE(0);
		game_set_draw_flags(GAME_DRAW_FLAG_REDRAW);
	}
}

/* Removes a character from the latest hiscore entry. */
void hiscore_new_entry_backspace()
{
	char *str = g_hiscores.entries[g_new_hiscore_position-1].name;

	/* If we're not a null string (excluding the trailing underscore) */
	if (str[1] != '\0') {
		/* Remove a character */
		int str_length = (int) strlen(str);

		str[str_length-1] = '\0';
		str[str_length-2] = '_';
		game_set_draw_flags(GAME_DRAW_FLAG_REDRAW);
	}
}

/* Adds a new character to the latest hiscore entry. */
void hiscore_new_entry_add_char(char c)
{
	char *str = g_hiscores.entries[g_new_hiscore_position-1].name;

	int str_length = (int) strlen(str);
	
	/* If we're at our maximum string length, do nothing. */
	if (str_length == HISCORE_NAME_MAXLENGTH) return;

	/* Otherwise, add the character. */
	str[str_length+1] = '\0';
	str[str_length] = '_';
	str[str_length-1] = c;
	game_set_draw_flags(GAME_DRAW_FLAG_REDRAW);
}

/* Attempts to insert the given score into the high score list.  If the
   score doesn't make it into the high score list, returns 0.  Otherwise,
   returns the position in the high score list + 1. */
int hiscore_insert_entry(HiScoreList *h, int score)
{
	int i;

	for (i = 0; i < HISCORE_NUM_ENTRIES; i++) {
		if (h->entries[i].score <= score) {
			int j;
			for (j = HISCORE_NUM_ENTRIES-1; j > i; j--) {
				h->entries[j].score = h->entries[j-1].score;
				strcpy(h->entries[j].name, h->entries[j-1].name);
			}
			h->entries[i].score = score;
			/* Add the leading underscore to the string. */
			h->entries[i].name[0] = '_';
			h->entries[i].name[1] = '\0';
			return i + 1;
		}
	}
	return 0;
}

/* Sets the "test" score that should be compared to the hi score list.  This
   function is used by game.c when the player loses the game, in effect
   telling the hi score module that we're not just passively looking at the
   hi score list--we want to see if the player scored a new hi score, too. */
void hiscore_set_test_score(int test_score)
{
	g_hiscore_test_score = test_score;
}

/* Compares the "test" score (submitted via hiscore_set_test_score()) to
   the hi score list to see if the score is worthy of inclusion in the
   hi score list.  If so, the user is prompted to enter their name.  Otherwise,
   the hi score list displays for a specified period of time and then goes
   back to the main menu.  If no "test" score was submitted, then this
   function does nothing. */
void hiscore_test_score()
{
	if (g_hiscore_test_score) {
		/* See if we've got a new entry in the hiscore list... */
		g_new_hiscore_position = hiscore_insert_entry(&g_hiscores, g_hiscore_test_score);
		if (!g_new_hiscore_position) {
			/* If not, set the countdown timer. */
			g_hiscore_countdown = HISCORE_COUNTDOWN_DELAY;
		} else {
			SDL_EnableUNICODE(1);
		}
		g_hiscore_test_score = 0;
	}
}

/* Returns true if the hiscore list was loaded from the file. */
int hiscore_load_from_file(HiScoreList *h, const char *filename)
{
	FILE *f;

	f = fopen(filename, "rb");

	if (f == NULL) return 0;

	fread(h, sizeof(HiScoreList), 1, f);
	return 1;
}

/* Writes the hiscore list to the given file. */
void hiscore_write_to_file(HiScoreList *h, const char *filename)
{
	FILE *f;

	f = fopen(filename, "wb");

	fwrite(h, sizeof(HiScoreList), 1, f);
}

void hiscore_construct(HiScoreList *h)
{
	int i;

	if (hiscore_load_from_file(h, HISCORE_FILENAME)) return;
	for (i = 0; i < HISCORE_NUM_ENTRIES; i++) {
		//h->entries[i].name[0] = '\0';
		strcpy(h->entries[i].name, "SMITH");
		h->entries[i].score = 000;
	}
}

void hiscore_destroy(HiScoreList *h)
{
	hiscore_write_to_file(h, HISCORE_FILENAME);
}

void hiscore_init()
{
	g_new_hiscore_position = 0;
	/* Load the hiscore from disk. */
	hiscore_construct(&g_hiscores);
	/* Check to see if we're being asked to check a score for inclusion
	   on the hi score list. */
	hiscore_test_score();
}

void hiscore_shutdown()
{
	/* Just in case we've been ordered to quit before the user finished
	   typing their name, save what they've typed so far as the new hi
	   score entry. */
	hiscore_new_entry_finalize();
	g_hiscore_countdown	= -1;
	/* Save the hiscore list to disk. */
	hiscore_destroy(&g_hiscores);
}

void hiscore_view(SDL_Surface *surface, int game_view_flags)
{
	int i;
	int curr_y;
	Font *f = game_get_font_big();

	/* If we're not being specifically told to redraw the screen, leave,
	   since the hiscore list's display doesn't really change much. */
	if (!(game_view_flags & GAME_DRAW_FLAG_REDRAW)) return;

	curr_y = HISCORE_START_Y;
	font_draw_string_centered(f, surface, (SCREEN_WIDTH / 2), curr_y, "Hi Scores");
	curr_y += HISCORE_LIST_SPACING * 2;

	for (i = 0; i < HISCORE_NUM_ENTRIES; i++) {
		char str_entry[50];
		
		sprintf(str_entry, "%05d   %s", g_hiscores.entries[i].score, g_hiscores.entries[i].name);
		font_draw_string(f, surface, HISCORE_LIST_X, curr_y, str_entry);
		curr_y += HISCORE_LIST_SPACING;
	}
}

void hiscore_model(Uint32 frame_time)
{
	/* If we need to countdown and then quit to the main menu, decrement
	   the countdown timer now. */
	if (g_hiscore_countdown > 0) {
		g_hiscore_countdown -= frame_time;
		if (g_hiscore_countdown <= 0) {
			game_set_state(&menu_game_state);			
		}
	}
}

/* Given the keyboard event, returns an ASCII character representing
   the key pressed.  If the key can't be represented using ASCII
   characters, returns 0. */
char get_key_char(SDL_KeyboardEvent *key)
{
	/* This statement was ripped out of the SDL documentation at
	   \lib_sdl\docs\html\guideinputkeyboard.html . */
	if( key->keysym.unicode < 0x80 && key->keysym.unicode > 0 ) {
        return (char)key->keysym.unicode;
	} else
		return 0;
}

/* Specialized controller if the player is being prompted to enter
   their name b/c they just made it on the hi score list. */
int hiscore_entry_controller(SDL_Event *e)
{
	if (e->type == SDL_KEYDOWN) {
		char c = (char) toupper(get_key_char(&e->key));
		
		if (isalpha(c)) {
			hiscore_new_entry_add_char(c);
			return 1;
		}
		switch (e->key.keysym.sym) {
			case SDLK_BACKSPACE:
				hiscore_new_entry_backspace();
				return 1;
				break;
			case SDLK_RETURN:
				hiscore_new_entry_finalize();
				/* Use the following line if we want a delay after the
				   player finishes entering their name. */
				g_hiscore_countdown = HISCORE_COUNTDOWN_DELAY_ENTRY_FINISHED;
				//game_set_state(&menu_game_state);
				return 1;
				break;
		}
	}
	return 0;
}

/* Generic controller for the hi score list.  Unless the player has been
   prompted to enter their name on the list, just quit if the user presses
   any keys. */
int hiscore_controller(SDL_Event *e)
{
	/* If the player is currently being prompted to enter a hi score, delegate
	   the controller to the new hiscore entry controller. */
	if (g_new_hiscore_position)
		return hiscore_entry_controller(e);
	if (e->type == SDL_KEYDOWN) {
		/* Otherwise, if the player presses any key, go back to the
		   main menu. */
		game_set_state(&menu_game_state);
		return 1;
	}
	return 0;
}
