#ifndef INCLUDE_PMAN_SCORE
#define INCLUDE_PMAN_SCORE

/* pman_score.h

   Functions and data for the scoreboard.
   Atul Varma - 7/24/2003

   The scoreboard keeps track of the player's score and lives left.  Also manages adding to
   and removing from the player's score, life count, and so forth.
*/

#include "SDL.h"

#include "state.h"
#include "pman_board.h"

/* Height and width of the scoreboard, in pixels. */
#define SCORE_PIXEL_HEIGHT 20
#define SCORE_PIXEL_WIDTH BOARD_PIXEL_WIDTH

/* Number of starting and maximum lives, respectively. */
#define SCORE_STARTING_LIVES 2
#define SCORE_MAX_LIVES 5

/* Point value for each nibblet */
#define SCORE_NIBBLET_SCORE  10
/* Point value for each nibbloon */
#define SCORE_NIBBLOON_SCORE 100
/* Base point value for killing a ghost */
#define SCORE_BASE_GHOST_SCORE 200
/* Base point value for killing a fruit */
#define SCORE_BASE_FRUIT_SCORE 100
/* Whenever these many points have been attained, an extra
   life is earned. */
#define SCORE_NEW_LIFE_EARNED       10000

typedef struct Score {
	/* Player's current score */
	int score;
	/* Used internally.  Tells whether the scoreboard has changed, and therefore whether
	   to redraw it or not. */
	int score_changed;
	/* Player's score at the last time they earned enough points to gain a level. */
	int score_last_life_earned;
	/* Number of lives the player has left. */
	int lives_left;
	/* Whether the scoreboard is visible or not. */
	int is_visible;
	/* The drawing rectangle of the scoreboard on the screen, in absolute pixel coordinates. */
	SDL_Rect draw_rect;
	/* How many ghosts the player has killed since the last nibbloon was eaten. */
	int curr_nibbloon_kills;
} Score;

void score_restart(Score *s);
void score_init(Score *s, int x_ofs, int y_ofs);
void score_destroy(Score *s);
void score_draw(Score *s, SDL_Surface *surface, int game_view_flags);
void score_toggle_visible(Score *s);
int score_lives_decrement(Score *s);
int score_add_agent_kill(Score *s, GameAgent *ga);
void score_add_nibbloon(Score *s);
void score_add(Score *s, int amount);

#endif
