#include "globals.h"

#include <assert.h>
#include <stdlib.h>

#include "SDL.h"

#include "fixed.h"
#include "state.h"
#include "drawing.h"
#include "pman.h"
#include "pman_board.h"
#include "pman_agent.h"
#include "pman_agent_ghost.h"
#include "pman_agent_fruit.h"

/* Restarts the fruit at the beginning/continuing of each level. */
void agent_fruit_restart(GameAgent *ga)
{
	ga->fruit_id++;
	ga->is_visible = 0;
	state_construct(&ga->state, STATE_ID_AGENT_FRUIT, STATE_ID_AGENT_FRUIT, ga, TIMER_ID_GAME_AGENT);
}

/* Initializes fruit and its graphics. */
void agent_fruit_init(GameAgent *ga)
{
	ga->agent_type = GAME_AGENT_FRUIT;
	ga->frames = game_load_bmp(FRUIT_BMP);
	fixed_vector_set(&ga->loc, FRUIT_PIXEL_X, FRUIT_PIXEL_Y);
	ga->last_loc = ga->loc;
	fixed_vector_set(&ga->graphical_dim, BLOCK_SIZE+8, BLOCK_SIZE+8);
	fixed_vector_set(&ga->graphical_offset, -4, -4);
	fixed_vector_set(&ga->physical_dim, BLOCK_SIZE, BLOCK_SIZE);
	ga->fruit_id = 0;
}

/* Draws the fruit. */
void agent_fruit_draw(GameAgent *ga, SDL_Surface *surface, int x_ofs, int y_ofs)
{
	SDL_Rect r_src, r_dst;
	int curr_level;

	/* Curr_level takes the current level into account to determine which
	   fruit to draw, i.e. which tile to grab off the fruit image.  If the
	   current level is more than the available number of fruits, then "wrap
	   around" to the first fruit by using the modulus operator. */
	curr_level = pman_get_level() % FRUIT_NUM_FRUITS;

	r_src.w = (Uint16) FIXED_GET_INT(ga->graphical_dim.x);
	r_src.h = (Uint16) FIXED_GET_INT(ga->graphical_dim.y);

	r_src.x = (Sint16) 0;
	r_src.y = (Sint16) curr_level * r_src.h;

	agent_get_draw_bounding_rect(ga, &r_dst, x_ofs, y_ofs);

	/* If we've been eaten and we need to show the score the player got
	   from eating the fruit, display the score instead of the fruit
	   sprite. */
	if (ga->ghost_score_amount > 0) {
		agent_draw_score_amount(ga, surface, &r_dst);
		return;
	}

	/* Otherwise, draw the fruit sprite. */
	game_update_rect_add(&r_dst);

	SDL_BlitSurface(ga->frames, &r_src, surface, &r_dst);	
}

/* Free all memory dynamically allocated by agent_fruit_init(). */
void agent_fruit_destroy(GameAgent *ga)
{
	SDL_FreeSurface(ga->frames);
}

/* Creates temporary int pool storage for a new fruit ID, increments
   the game agent's fruit ID, and assigns the new fruit ID to the
   temporary int storage variable and returns it.  Used in the fruit
   FSM to make sure invalid (old) "toggle visibility" messages aren't
   processed. */
int *agent_fruit_id_new(GameAgent *ga)
{
	int *data;

	data = temp_int_pool_get_int();

	ga->fruit_id++;
	data = temp_int_pool_get_int();
	*data = ga->fruit_id;

	return data;
}

/* The fruit game agent state machine. */
BEGIN_STATE_MACHINE(agent_fruit_state_machine)
	GameAgent *fruit = (GameAgent *) s->parent;
	STATE_MACHINE_HEADER
	ON_ENTER
		int *data;

		data = agent_fruit_id_new(fruit);

		state_send_message(FRUIT_MSG_DISPLAY_TOGGLE, 0, STATE_ID_AGENT_FRUIT, rand_int(FRUIT_INITIAL_RAND_TIME)+FRUIT_INITIAL_BASE_TIME, data);
	ON_MSG(AGENT_MSG_HIT_PMAN)
		if (fruit->is_visible) {
			state_send_message(PLAY_STATE_MSG_AGENT_KILLED, s->state_id, STATE_ID_PLAY_STATE, 0, 0);
		}
	ON_MSG(AGENT_MSG_CONTINUE)
		int *data;

		fruit->ghost_score_amount = 0;
		agent_toggle_visible(fruit);

		data = agent_fruit_id_new(fruit);

		state_send_message(FRUIT_MSG_DISPLAY_TOGGLE, 0, STATE_ID_AGENT_FRUIT, rand_int(FRUIT_EATEN_RAND_TIME)+FRUIT_EATEN_BASE_TIME, data);	
	ON_MSG(FRUIT_MSG_DISPLAY_TOGGLE)
		int *data;

		data = (int *) sm->data;

		if (*data != fruit->fruit_id) return 1;
		agent_toggle_visible(fruit);

		fruit->fruit_id++;
		*data = fruit->fruit_id;

		/* Based on whether we just appeared or disappeared, use the appropriate
		   *_BASE_TIME and *_RAND_TIME constants to tell the game how much time
		   needs to pass for us to disappear or reappear, respectively. */
		if (fruit->is_visible) {
			state_send_message(FRUIT_MSG_DISPLAY_TOGGLE, 0, STATE_ID_AGENT_FRUIT, rand_int(FRUIT_APPEARED_RAND_TIME)+FRUIT_APPEARED_BASE_TIME, data);
		} else {
			state_send_message(FRUIT_MSG_DISPLAY_TOGGLE, 0, STATE_ID_AGENT_FRUIT, rand_int(FRUIT_DISAPPEARED_RAND_TIME)+FRUIT_DISAPPEARED_BASE_TIME, data);
		}
END_STATE_MACHINE
