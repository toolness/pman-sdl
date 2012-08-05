#include "globals.h"

#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "SDL.h"

#include "game.h"
#include "state.h"
#include "font.h"
#include "debug.h"
#include "fixed.h"
#include "drawing.h"
#include "pman.h"
#include "pman_score.h"
#include "pman_agent_pman.h"

/* Restarts the game board.  Should be called whenever a new level is started. */
void score_restart(Score *s)
{
	s->is_visible = 1;
	s->score_changed = 0;
	s->curr_nibbloon_kills = 0;
}

/* Initializes the game board by dynamically allocating memory, etc.  Should be
   used only once per game session, and always countered with board_destroy(). */
void score_init(Score *s, int x_ofs, int y_ofs)
{
	if (pman_in_demo_mode()) {
		s->lives_left = 0;
	} else {
		s->lives_left = SCORE_STARTING_LIVES;
	}
	s->score = 0;
	s->score_last_life_earned = 0;

	s->draw_rect.h = SCORE_PIXEL_HEIGHT;
	s->draw_rect.w = SCORE_PIXEL_WIDTH;
	s->draw_rect.x = (Sint16) x_ofs;
	s->draw_rect.y = (Sint16) y_ofs;
}

/* Deallocates memory gathered in board_init(). */
void score_destroy(Score *s)
{
	/* Do nothing for now... Someday we might put something in here. */
}

/* Draws the player's score to the scoreboard. */
void score_draw_score(Score *s, SDL_Surface *surface)
{
	Font *f;
	char buffer[50];
	SDL_Rect r;

	f = game_get_font_big();

	sprintf(buffer, "Score: %05d  L%02d", s->score, pman_get_level()+1);
	r.x = s->draw_rect.x;
	r.y = s->draw_rect.y + (Uint16) 4;
	r.w = (Uint16) (f->char_width * strlen(buffer));
	r.h = (Uint16) f->char_height;
	SDL_FillRect(surface, &r, 0);

	font_draw_string(f, surface, r.x, r.y, buffer);
}

/* Draws the number of player lives left onto the scoreboard by
   drawing one pac-man icon for each life remaining. */
void score_draw_lives(Score *s, SDL_Surface *surface)
{
	int radius = 10;
	int mouth = 5;
	int pman_icon_size = 2*radius+3;
	int start_x = s->draw_rect.x + SCORE_PIXEL_WIDTH - radius;
	int i;

	for (i = 0; i < s->lives_left; i++) {
		pman_draw(surface, start_x, s->draw_rect.y+radius, radius, mouth, 0, DIRECTION_RIGHT, game_map_rgb(255, 255, 0));
		start_x -= pman_icon_size;
	}
}

/* Redraws the entire scoreboard.  Different from score_draw(), which
   decides which parts of the scoreboard should be redrawn, calling
   this function if necessary. */
void score_redraw(Score *s, SDL_Surface *surface)
{
	score_draw_score(s, surface);
	score_draw_lives(s, surface);
}

/* Draws the scoreboard. */
void score_draw(Score *s, SDL_Surface *surface, int game_view_flags)
{
	if (s->is_visible) {
		if (game_view_flags & GAME_DRAW_FLAG_REDRAW || s->score_changed) {
			score_redraw(s, surface);
			if (s->score_changed) {
				game_update_rect_add(&s->draw_rect);
			}
			s->score_changed = 0;
		} else { }
	} else {
		if (game_view_flags & GAME_DRAW_FLAG_REDRAW)
			SDL_FillRect(surface, &s->draw_rect, 0);
	}
}

/* Toggles whether the score is visible or not.

   Note: I don't believe this function is actually used anywhere.  It was
   included b/c all the other visible game tokens implemented it, so it
   would be a great candidate for inheritance in an OO framework. */
void score_toggle_visible(Score *s)
{
	if (s->is_visible)
		s->is_visible = 0;
	else
		s->is_visible = 1;

	game_set_draw_flags(GAME_DRAW_FLAG_REDRAW);
}

/* Removes a life from the scoreboard.  If there are no more lives left to lose,
   this function returns 0; otherwise it returns 1. */
int score_lives_decrement(Score *s)
{
	if (s->lives_left == 0) return 0;
	s->lives_left--;
	s->score_changed = 1;
	return 1;
}

/* Give the player an extra life. */
void score_lives_increment(Score *s)
{
	if (s->lives_left < SCORE_MAX_LIVES) s->lives_left++;
}

/* Resets the "ghost killed since last nibbloon eaten" counter used
   for determining how many points are to be awarded for a ghost kill.
   This function is called whenever a nibbloon is eaten. */
void score_nibbloon_kills_reset(Score *s)
{
	s->curr_nibbloon_kills = 0;
}

/* Adds the required # of points for a ghost kill to the player's score.
   This is determined by looking at how many ghosts have been killed since
   the last nibbloon was eaten. */
int score_add_agent_kill(Score *s, GameAgent *ga)
{
	int i;
	int amount;

	if (ga->agent_type == GAME_AGENT_FRUIT) {
		amount = SCORE_BASE_FRUIT_SCORE * (pman_get_level() + 1);
		score_add(s, amount);
		return amount;
	}
	if (ga->agent_type == GAME_AGENT_GHOST) {
		amount = SCORE_BASE_GHOST_SCORE;

		for (i = 0; i < s->curr_nibbloon_kills; i++) {
			/* For each ghost that's died so far, we double the point value. */
			amount *= 2;
		}
		s->curr_nibbloon_kills++;
		score_add(s, amount);
		return amount;
	}
	return 0;
}

/* Adds the number of required points to the player's score when
   they hit a nibbloon.  Also resets the nibbloon kill counter, which
   helps us determine how many points to give for every ghost killed.
   (Since the points awarded for a ghost kill depend on how many ghosts
   have been killed since the last nibbloon was eaten.) */
void score_add_nibbloon(Score *s)
{
	score_add(s, SCORE_NIBBLOON_SCORE);
	score_nibbloon_kills_reset(s);
}

/* Adds the given amount to the player's score, giving him an extra
   life if necessary. */
void score_add(Score *s, int amount)
{
	/* Add to the score */
	s->score += amount;
	/* Test to see if player's gotten enough points for a new life */
	if (s->score - s->score_last_life_earned >= SCORE_NEW_LIFE_EARNED) {
		s->score_last_life_earned = s->score;
		score_lives_increment(s);
	}
	/* Set the score changed flag so the scoreboard gets redrawn. */
	s->score_changed = 1;
}
