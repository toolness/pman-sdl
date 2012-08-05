#include "globals.h"

#include <assert.h>
#include <stdlib.h>

#include "SDL.h"

#include "fixed.h"
#include "drawing.h"
#include "pman.h"
#include "pman_board.h"
#include "pman_agent.h"
#include "pman_agent_ghost.h"
#include "pman_agent_pman.h"

/* Array of colors/sprite-states used by the ghost game agent. */
static Uint32 g_ghost_colors[GHOST_MAX_COLORS] = {0};

/* Draws the eyes (whites of eyes and pupils) of a ghost at the given location.  The
   eyes are looking in the given direction. */
void ghost_draw_eyes(SDL_Surface *surface, SDL_Rect *r, int direction)
{
	int radius = 3;
	int p1_x, p1_y, p2_x, p2_y;

	p1_x = r->x + 5 + radius;
	p1_y = r->y + 5 + radius;
	p2_x = r->x + r->w - 5 - radius;
	p2_y = p1_y;

	/* Draw the whites of their eyes */
	drawCircle(surface, p1_x, p1_y, radius, FILL_FULL, g_ghost_colors[GHOST_COLOR_EYES]);
	drawCircle(surface, p2_x, p2_y, radius, FILL_FULL, g_ghost_colors[GHOST_COLOR_EYES]);	

	/* Depending on the movement direction, determine the location of the
	   ghost's pupils. */
	switch (direction) {
		case DIRECTION_UP:
			p1_y -= GHOST_SPRITE_EYEBALL_OFFSET;
			p2_y -= GHOST_SPRITE_EYEBALL_OFFSET;
			break;
		case DIRECTION_DOWN:
			p1_y += GHOST_SPRITE_EYEBALL_OFFSET;
			p2_y += GHOST_SPRITE_EYEBALL_OFFSET;
			break;
		case DIRECTION_LEFT:
			p1_x -= GHOST_SPRITE_EYEBALL_OFFSET;
			p2_x -= GHOST_SPRITE_EYEBALL_OFFSET;
			break;
		case DIRECTION_RIGHT:
			p1_x += GHOST_SPRITE_EYEBALL_OFFSET;
			p2_x += GHOST_SPRITE_EYEBALL_OFFSET;
			break;
	}

	radius = 2;

	/* Draw the ghost's pupils. */
	drawCircle(surface, p1_x, p1_y, radius, FILL_FULL, g_ghost_colors[GHOST_COLOR_PUPILS]);
	drawCircle(surface, p2_x, p2_y, radius, FILL_FULL, g_ghost_colors[GHOST_COLOR_PUPILS]);
}

/* Draws the body ("bedsheet") of the ghost using the given color at the given location. */
void ghost_draw_body(SDL_Surface *surface, SDL_Rect *r, Uint32 color)
{
	SDL_Rect r2;
	int radius = (r->w / 2) - 1;
	int i;
	int p1_x, p1_y, p2_x, p2_y, p3_x, p3_y;

	/* Draw the top half-circle */
	drawCircle(surface, r->x + radius, r->y + radius, radius, FILL_TOP_HALF_ONLY, color);

	/* Draw the middle rectangle */
	r2.x = r->x;
	r2.y = (Sint16) (r->y + radius);
	r2.h = (Sint16) (radius - GHOST_SPRITE_TRIANGLE_HEIGHT);
	r2.w = r->w - 1;
	SDL_FillRect(surface, &r2, color);

	p1_x = r2.x;
	p1_y = r2.y + r2.h;
	p2_x = p1_x + GHOST_SPRITE_TRIANGLE_WIDTH / 2;
	p2_y = p1_y + GHOST_SPRITE_TRIANGLE_HEIGHT;
	p3_x = p1_x + GHOST_SPRITE_TRIANGLE_WIDTH;
	p3_y = p1_y;

	/* Draw the lower "tattered" part */
	for (i = 0; i < (r2.w / GHOST_SPRITE_TRIANGLE_WIDTH); i++) {
		drawFlatTopTriangle(surface, p1_x, p1_y, p2_x, p2_y, p3_x, p3_y, color);
		p1_x += GHOST_SPRITE_TRIANGLE_WIDTH;
		p2_x += GHOST_SPRITE_TRIANGLE_WIDTH;
		p3_x += GHOST_SPRITE_TRIANGLE_WIDTH;
		if (p3_x > r2.x+r2.w-2) p3_x = r2.x+r2.w-2;
	}
}

/* Draws the "scared" sprite of the ghost at the given location. */
void ghost_draw_scared_features(SDL_Surface *surface, SDL_Rect *r, Uint32 color)
{
	int radius = 2;
	int p1_x, p1_y, p2_x, p2_y, p3_x, p3_y;
	int i;

	p1_x = r->x + 5 + radius;
	p1_y = r->y + 5 + radius;
	p2_x = r->x + r->w - 5 - radius;
	p2_y = p1_y;

	/* Draw the freaked-out eyes */
	drawCircle(surface, p1_x, p1_y, radius, FILL_FULL, color);
	drawCircle(surface, p2_x, p2_y, radius, FILL_FULL, color);	

	p1_x = r->x + 5;
	p1_y = r->y + r->h - 10;
	p2_x = p1_x + 1;
	p2_y = p1_y + 2;
	p3_x = p1_x + 2;
	p3_y = p1_y;

	/* Draw the freaked-out, zigzag mouth */
	for (i = 0; i < ( (r->w - 10) / 2); i++) {
		drawLine(surface, p1_x, p1_y, p2_x, p2_y, color);
		drawLine(surface, p2_x, p2_y, p3_x, p3_y, color);
		p1_x += 2;
		p2_x += 2;
		p3_x += 2;
	}
}

/* Initializes the ghost global color array. */
void ghost_colors_init()
{
	g_ghost_colors[GHOST_COLOR_SCARED] = game_map_rgb(0, 0, 150);
	g_ghost_colors[GHOST_COLOR_SPIRIT]= game_map_rgb(50, 50, 50);
	g_ghost_colors[GHOST_COLOR_EYES] = game_map_rgb(255, 255, 255);
	g_ghost_colors[GHOST_COLOR_PUPILS] = game_map_rgb(0, 0, 0);
	g_ghost_colors[GHOST_COLOR_SCARED_FEATURES] = game_map_rgb(255, 0, 0);
}

/* Draws the ghost agent at the given location, depending on its color state. */
void agent_ghost_draw(GameAgent *ga, SDL_Surface *surface, int x_ofs, int y_ofs)
{
	SDL_Rect r;

	agent_get_draw_bounding_rect(ga, &r, x_ofs, y_ofs);

	if (ga->ghost_score_amount > 0) {
		agent_draw_score_amount(ga, surface, &r);
		return;
	}

	if (ga->color == g_ghost_colors[GHOST_COLOR_SCARED]) {
		/* If they're scared, draw their body and then their scared features */
		ghost_draw_body(surface, &r, ga->color);
		ghost_draw_scared_features(surface, &r, g_ghost_colors[GHOST_COLOR_SCARED_FEATURES]);
	} else if (ga->color == g_ghost_colors[GHOST_COLOR_SPIRIT]) {
		/* If they're in spirit form and running back to the asylum to get respawned,
		   draw only their eyes. */
		ghost_draw_eyes(surface, &r, map_fixed_vector_to_direction(&ga->curr_move));
	} else {
		/* Otherwise, draw their body and then their eyes. */
		ghost_draw_body(surface, &r, ga->color);
		ghost_draw_eyes(surface, &r, map_fixed_vector_to_direction(&ga->curr_move));
	}

	game_update_rect_add(&r);
}

/* Restarts the ghost game agent.  Should be called whenever a
   new level starts. */
void agent_ghost_restart(GameAgent *ga, int block_x, int block_y, int state_id, int state_machine_id, int initial_state, int resting_hit_times)
{
	ga->is_visible = 1;
	ga->ghost_score_amount = 0;
	ga->ghost_resting_hit_times = resting_hit_times;
	ga->can_open_asylum_door = 0;
	//ga->original_speed = FIXED_SET_INT(PMAN_BASE_SPEED + pman_get_level());
	ga->original_speed = fixed_from_float( CONVERT_PPDS_TO_PPMS(PMAN_BASE_SPEED + GHOST_ADDED_SPEED_PER_LEVEL*pman_get_level()) );
	ga->speed = ga->original_speed;
	ga->color = ga->original_color;
	ga->ghost_flee_times = 0;
	ga->curr_move = fixed_vector_zero;
	ga->next_move = fixed_vector_zero;
	fixed_vector_set(&ga->loc, block_x*BLOCK_SIZE, block_y*BLOCK_SIZE);
	ga->last_loc = ga->loc;
	state_construct(&ga->state, state_id, state_machine_id, ga, TIMER_ID_GAME_AGENT);
	ga->state.state = initial_state;
}

/* Initializes the ghost game agent.  This should take care of any
   dynamic memory allocation that needs to be done, and should only
   really be called once per gameplay session. */
void agent_ghost_init(GameAgent *ga, Uint32 color)
{
	ga->agent_type = GAME_AGENT_GHOST;
	fixed_vector_set(&ga->graphical_offset, -4, -4);
	fixed_vector_set(&ga->graphical_dim, BLOCK_SIZE+8, BLOCK_SIZE+8);
	fixed_vector_set(&ga->physical_dim, BLOCK_SIZE-1, BLOCK_SIZE-1);
	ga->original_color = color;
	ghost_colors_init();
}

/* Main agent move routine.  Currently, ghosts just move in random directions if they
   can't see pman.  When
   they hit an intersection, they look forward, left and right.  If they see pac man,
   they go in that direction. */
void agent_ghost_determine_next_move(GameAgent *ga)
{
	FixedVector agent_dirs[3];
	FixedVector *the_dir;
	int i;

	/* Direction in front of the agent. */
	agent_dirs[0] = ga->curr_move;
	
	/* Direction to the right of the agent. */
	agent_dirs[1] = fixed_vector_rotate_right(&ga->curr_move);

	/* Direction to the left of the agent. */
	agent_dirs[2] = fixed_vector_rotate_left(&ga->curr_move);

	the_dir = NULL;

	for (i = 0; i < 3; i++) {
		if (agent_can_see_agent(ga, pman_get_game_agent(STATE_ID_AGENT_PMAN), &agent_dirs[i], 20))
			the_dir = &agent_dirs[i];
	}

	if (the_dir) {
		agent_set_move(ga, the_dir);		
	}
	else {
		agent_determine_next_random_move(ga);
	}
}

/* When the agent is scared, this function determines its next move.  If
   reverse_ok is true, then it's ok for the ghost to go in the direction opposite
   from the one it's going in. */
void agent_ghost_scared_determine_next_move(GameAgent *ga, int reverse_ok)
{
	FixedVector agent_dirs[4];
	FixedVector viable_dirs[4];
	int i;
	int num_agent_dirs;
	int num_viable_dirs;

	/* If we're in a wrap-around tunnel, don't do anything. */
	if (agent_in_tunnel(ga)) return;

	/* Direction in front of the agent. */
	agent_dirs[0] = ga->curr_move;
	
	/* Direction to the right of the agent. */
	agent_dirs[1] = fixed_vector_rotate_right(&ga->curr_move);

	/* Direction to the left of the agent. */
	agent_dirs[2] = fixed_vector_rotate_left(&ga->curr_move);

	if (reverse_ok) {
		agent_dirs[3] = fixed_vector_reverse(&ga->curr_move);
		num_agent_dirs = 4;
	} else
		num_agent_dirs = 3;

	num_viable_dirs = 0;
	for (i = 0; i < num_agent_dirs; i++) {
		if (!agent_can_see_agent(ga, pman_get_game_agent(STATE_ID_AGENT_PMAN), &agent_dirs[i], 20)) {
			FixedVector temp_dir_scaled;
			FixedVector new_position;

			temp_dir_scaled = fixed_vector_scale(&agent_dirs[i], FIXED_SET_INT(BLOCK_SIZE));
			
			new_position = fixed_vector_add(&ga->loc, &temp_dir_scaled);
			if ( agent_is_position_viable(ga, &new_position) ) {
				viable_dirs[num_viable_dirs] = agent_dirs[i];
				num_viable_dirs++;
			}
		}
	}

	if (num_viable_dirs == 0) {
		agent_determine_next_random_move(ga);
		return;
	}
	agent_set_move(ga, fixed_vector_choose_random(viable_dirs, num_viable_dirs));
}

/* Tests to see if the ghost is on or has passed the center of the asylum
   (which lies between two blocks), and if so, sets them on it and sets the
   current move to 0 and returns 1.  Otherwise, returns 0. */
int agent_ghost_adjust_asylum_center_pos(GameAgent *ga)
{
	// is the agent on the block left of center?
	if (GET_BLOCK_FIXED(ga->loc.x) == BLOCK_ASYLUM_CENTER_X) {
		// if so, are they left or right of center?
		// if they're left of center and moving left, set them directly on center and stop movement.
		// if they're right of center and moving right, set them on center and stop movement.
		int pixel_loc_x = FIXED_GET_INT(ga->loc.x);
		int on_center = 0;
		if (pixel_loc_x == BLOCK_ASYLUM_CENTER_PIXEL_X)
			on_center = 1;
		else if (pixel_loc_x < BLOCK_ASYLUM_CENTER_PIXEL_X &&
			ga->curr_move.x < 0)
				on_center = 1;
		else if (pixel_loc_x > BLOCK_ASYLUM_CENTER_PIXEL_X &&
			ga->curr_move.x > 0)
				on_center = 1;
		if (on_center == 1) {
			ga->loc.x = FIXED_SET_INT(BLOCK_ASYLUM_CENTER_PIXEL_X);
			return 1;
		}
	}
	return 0;
}

int agent_ghost_go_to_asylum(GameAgent *ga)
{
	Board *b = pman_get_board();
	FixedVector new_move;

	new_move = board_get_asylum_directions_at_block(b, GET_BLOCK_FIXED(ga->loc.x), GET_BLOCK_FIXED(ga->loc.y) );

	if (!fixed_vector_equals(&new_move, &fixed_vector_zero)) {
		agent_set_move(ga, &new_move);
		return 1;
	} else return 0;
}

/* Moronic ghost state machine. */
BEGIN_STATE_MACHINE(agent_ghost1_state_machine)
	GameAgent *ghost = (GameAgent *) s->parent;
	STATE_MACHINE_HEADER
	ON_ENTER
		SET_STATE(GHOST_STATE_SEEKING);
	ON_UPDATE
		int move_result;
		Uint32 time = *(Uint32 *) sm->data;
		move_result = agent_move(ghost, time);
	STATE(GHOST_STATE_SEEKING)
		ON_ENTER
			agent_ghost_determine_next_move(ghost);
		ON_MSG(AGENT_MSG_HIT_PMAN)
			state_send_message(PLAY_STATE_MSG_PMAN_KILLED, 0, STATE_ID_PLAY_STATE, 0, 0);
		ON_MSG(GAME_AGENT_MSG_BLOCK_CHANGE)
			agent_ghost_determine_next_move(ghost);
		ON_MSG(GHOST_MSG_START_FLEEING)
			SET_STATE(GHOST_STATE_FLEEING);
	STATE(GHOST_STATE_FLEEING)
		ON_ENTER
			int *data;

			ghost->color = g_ghost_colors[GHOST_COLOR_SCARED];
			ghost->speed = FIXED_MULT(ghost->speed, fixed_from_float(GHOST_FLEE_SPEED_MULTIPLIER));
			/* Change the "fleeing" id so that old fleeing messages that are still queued are
			   now ignored.  All state messages dealing with fleeing behavior will have the
			   current value of this variable passed as the message data, so as to "tag" which
			   particular fleeing state the message belongs to. */
			ghost->ghost_flee_times++;
			ghost->ghost_flee_flash_times = GHOST_FLEE_FLASH_TIMES;
			agent_ghost_scared_determine_next_move(ghost, 1);

			data = temp_int_pool_get_int();
			*data = ghost->ghost_flee_times;
			state_send_message(GHOST_MSG_FLEE_FLASH, 0, s->state_id, GHOST_FLEE_INITIAL_TIME - (GHOST_FLEE_LESS_TIME_PER_LEVEL*pman_get_level()), data );
		ON_EXIT
			ghost->color = ghost->original_color;
			ghost->speed = ghost->original_speed;
			ghost->is_visible = 1;
		ON_MSG(GAME_AGENT_MSG_BLOCK_CHANGE)
			agent_ghost_scared_determine_next_move(ghost, 0);
		ON_MSG(AGENT_MSG_HIT_PMAN)
			state_send_message(PLAY_STATE_MSG_AGENT_KILLED, s->state_id, STATE_ID_PLAY_STATE, 0, 0);
		ON_MSG(AGENT_MSG_FREEZE_AND_DIE)
			SET_STATE(GHOST_STATE_FREEZE_KILLED);
		ON_MSG(GHOST_MSG_FLEE_FLASH)
			if ( *(int *) sm->data != ghost->ghost_flee_times) return 1;
			if (ghost->ghost_flee_flash_times == 0) {
				SET_STATE(GHOST_STATE_SEEKING);
			}

			agent_toggle_visible(ghost);
			ghost->ghost_flee_flash_times--;
			state_send_message(GHOST_MSG_FLEE_FLASH, 0, s->state_id, GHOST_FLEE_FLASH_DELAY, sm->data);
		ON_MSG(GHOST_MSG_START_FLEEING)
			SET_STATE(GHOST_STATE_FLEEING);
	STATE(GHOST_STATE_FREEZE_KILLED)
		ON_MSG(AGENT_MSG_CONTINUE)
			SET_STATE(GHOST_STATE_SPIRIT);
		ON_EXIT
			ghost->ghost_score_amount = 0;
			game_set_draw_flags(GAME_DRAW_FLAG_REDRAW);
	STATE(GHOST_STATE_SPIRIT)
		ON_ENTER
			ghost->color = g_ghost_colors[GHOST_COLOR_SPIRIT];
		ON_MSG(GAME_AGENT_MSG_BLOCK_CHANGE)
			// we're near the asylum, now go to the entrance point
			if (!agent_ghost_go_to_asylum(ghost)) {
				SET_STATE(GHOST_STATE_GOTO_ASYLUM_ENTRANCE);
			}
	STATE(GHOST_STATE_RESTING)
		ON_ENTER
			ghost->curr_move = fixed_vector_down;
		ON_UPDATE
			int move_result;
			Uint32 time = *(Uint32 *) sm->data;

			move_result = agent_move(ghost, time);
			if (!move_result) {
				FixedVector reverse_dir;

				reverse_dir = fixed_vector_reverse(&ghost->curr_move);
				agent_set_move(ghost, &reverse_dir);
				ghost->ghost_resting_hit_times--;
				if (ghost->ghost_resting_hit_times == 0)
					SET_STATE(GHOST_STATE_GOTO_ASYLUM_EXIT);
			}
	STATE(GHOST_STATE_GOTO_ASYLUM_ENTRANCE)
		ON_UPDATE
			int move_result;
			Uint32 time = *(Uint32 *) sm->data;

			move_result = agent_move(ghost, time);

			if (agent_ghost_adjust_asylum_center_pos(ghost)) {
				SET_STATE(GHOST_STATE_ENTER_ASYLUM);
			}
	STATE(GHOST_STATE_GOTO_ASYLUM_EXIT)
		ON_ENTER
			ghost->curr_move = fixed_vector_up;
		ON_UPDATE
			int move_result;
			Uint32 time = *(Uint32 *) sm->data;

			move_result = agent_move(ghost, time);
			if (!move_result) {
				if ( FIXED_GET_INT(ghost->loc.x) < BLOCK_ASYLUM_CENTER_PIXEL_X) {
					agent_set_move(ghost, &fixed_vector_right);
				} else if ( FIXED_GET_INT(ghost->loc.x) > BLOCK_ASYLUM_CENTER_PIXEL_X) {
					agent_set_move(ghost, &fixed_vector_left);
				} else {
					SET_STATE(GHOST_STATE_LEAVE_ASYLUM);
				}
			}
			if (agent_ghost_adjust_asylum_center_pos(ghost)) {
				SET_STATE(GHOST_STATE_LEAVE_ASYLUM);
			}
	STATE(GHOST_STATE_LEAVE_ASYLUM)
		ON_ENTER
			ghost->can_open_asylum_door = 1;
			ghost->curr_move = fixed_vector_up;		
		ON_UPDATE
			int move_result;
			Uint32 time = *(Uint32 *) sm->data;

			move_result = agent_move(ghost, time);
			if (!move_result) {
				SET_STATE(GHOST_STATE_SEEKING);
			}
		ON_EXIT
			ghost->can_open_asylum_door = 0;
	STATE(GHOST_STATE_ENTER_ASYLUM)
		ON_ENTER
			ghost->can_open_asylum_door = 1;
			ghost->curr_move = fixed_vector_down;
		ON_UPDATE
			int move_result;
			Uint32 time = *(Uint32 *) sm->data;

			move_result = agent_move(ghost, time);
			if (FIXED_GET_INT(ghost->loc.y) >= BLOCK_ASYLUM_CENTER_PIXEL_Y)
				SET_STATE(GHOST_STATE_RESPAWN);
		ON_EXIT
			ghost->can_open_asylum_door = 0;
	STATE(GHOST_STATE_RESPAWN)
		ON_ENTER
			ghost->color = ghost->original_color;
			SET_STATE(GHOST_STATE_LEAVE_ASYLUM);
END_STATE_MACHINE
