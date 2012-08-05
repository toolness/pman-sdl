#include "globals.h"

#include <assert.h>
#include <stdlib.h>

#include "SDL.h"

#include "fixed.h"
#include "drawing.h"
#include "pman.h"
#include "pman_board.h"
#include "pman_agent.h"
#include "pman_agent_pman.h"
#include "pman_agent_ghost.h"
#include "pman_agent_fruit.h"

/* Changes the agent's direction to its next queued move, if
   that direction is viable.  If the direction isn't viable,
   agent_next_move() does nothing.  Returns 1 if changing the
   move was successful, 0 if not. */
int agent_next_move(GameAgent *ga)
{
	FixedVector temp_dir_scaled, new_position;

	/* If there's no next move queued up, exit. */
	if (fixed_vector_is_zero(&ga->next_move)) return 0;

	temp_dir_scaled = fixed_vector_scale(&ga->next_move, FIXED_SET_INT(BLOCK_SIZE));
	
	new_position = fixed_vector_add(&ga->loc, &temp_dir_scaled);

	if (agent_is_position_viable(ga, &new_position)) {
		agent_set_move(ga, &ga->next_move);
		return 1;
	} else
		return 0;
}

/* Set's the agent current move in the given direction
   and changes their next queued move to be nothing. */
void agent_set_move(GameAgent *ga, const FixedVector *v)
{
	ga->curr_move = *v;
	ga->next_move = fixed_vector_zero;
}

/* Sets the next queued directional move for the game agent.
   If the agent is not currently moving, it sets the current
   directional move instead. */
void agent_set_next_move(GameAgent *ga, const FixedVector *v)
{
	if (fixed_vector_is_zero(&ga->curr_move)) {
		agent_set_move(ga, v);
	} else {
		ga->next_move = *v;
	}
}

/* Returns true if the ghost can see pacman from its position; looks at most
   max_dist blocks in the given cardinal direction. */
int agent_can_see_agent(GameAgent *ga, GameAgent *pman, FixedVector *direction, int max_dist)
{
	FixedVector dir_block_size;
	FixedVector ga_new_loc;
	SDL_Rect r_pman;
	SDL_Rect r_block;
	int blocks_seen = 0;

	fixed_vector_to_rect_dimensions(&pman->physical_dim, &r_pman);
	fixed_vector_to_rect_coords(&pman->loc, &r_pman);

	r_block.h = BLOCK_SIZE; r_block.w = BLOCK_SIZE;

	dir_block_size = fixed_vector_scale(direction, FIXED_SET_INT(BLOCK_SIZE));

	ga_new_loc = fixed_vector_add(&ga->loc, &dir_block_size);

	while ( blocks_seen < max_dist && agent_is_position_viable(ga, &ga_new_loc) ) {
		blocks_seen++;
		fixed_vector_to_rect_coords(&ga_new_loc, &r_block);
		if ( rects_intersect(&r_pman, &r_block) ) {
			return 1;
		}
		ga_new_loc = fixed_vector_add(&ga_new_loc, &dir_block_size);

	}
	return 0;
}

/* Returns true if the agent is currently in the wraparound tunnel. */
int agent_in_tunnel(GameAgent *ga)
{
	return (FIXED_GET_INT(ga->loc.x) <= 0 || FIXED_GET_INT(ga->loc.x) >= BOARD_PIXEL_WIDTH);
}

/* Moronic AI.  Just moves in a random direction whenever an intersection
   is reached, but doesn't backtrack. */
void agent_determine_next_random_move(GameAgent *ga)
{
	/* The 4 cardinal directions (normalized) */ 
	FixedVector dir[4];
	/* The new position of the game agent when it's moved one block in each
	   of the cardinal directions. */
	FixedVector new_position[4];
	/* The normalized vector of the REVERSE of the direction in which the agent
	   is moving. */
	FixedVector curr_move_reversed;

	/* List of cardinal directions that are viable for moving towards. */
	FixedVector ok_dirs[4];
	/* Number of directions that are viable for moving towards (i.e., number
	   of elements in ok_dirs[]). */
	int num_ok_dirs = 0;

	int i;

	/* If we're in a wrap-around tunnel, don't do anything. */
	if (agent_in_tunnel(ga)) return;

	dir[0] = fixed_vector_left;
	dir[1] = fixed_vector_right;
	dir[2] = fixed_vector_up;
	dir[3] = fixed_vector_down;

	curr_move_reversed = fixed_vector_reverse(&ga->curr_move);

	for (i = 0; i < 4; i++) {
		if (fixed_vector_equals(&dir[i], &curr_move_reversed)) {
			/* Disregard the move if this direction is the reverse of the one
			   the agent is moving in. */
		} else {
			/* Can we move in the current cardinal direction?  If so, add it to the
			   list of viable directions. */
			FixedVector temp_dir_scaled;

			temp_dir_scaled = fixed_vector_scale(&dir[i], FIXED_SET_INT(BLOCK_SIZE));
			
			new_position[i] = fixed_vector_add(&ga->loc, &temp_dir_scaled);
			if ( agent_is_position_viable(ga, &new_position[i]) ) {
				ok_dirs[num_ok_dirs] = dir[i];
				num_ok_dirs++;
			}
		}
	}

	/* Pick a random direction to move in, out of the viable directions. */
	if (num_ok_dirs == 0) return;
	agent_set_move(ga, fixed_vector_choose_random(ok_dirs, num_ok_dirs));
}

/* Returns whether the given fixed-point pixel vector is a viable position
   for the game agent. */
int agent_is_position_viable_helper(GameAgent *ga, FixedVector *v)
{
	/* Get the block coordinates of the pixel vector. */
	int x = GET_BLOCK_FIXED(v->x);
	int y = GET_BLOCK_FIXED(v->y);

	/* Get the block value at that location. */
	int block = board_get_block(pman_get_board(), x, y);

	/* Regardless of what kind of game agent we are, if it's a wall,
	   we can't go there. */
	if (block == BLOCK_WALL) return 0;

	/* If we're pac man, we can't go through the asylum door (only
	   ghosts can do that). */
	if (block == BLOCK_ASYLUM_DOOR) {
		if (ga->agent_type == GAME_AGENT_GHOST &&
			ga->can_open_asylum_door) return 1;
		else
			return 0;
	}

	/* Otherwise, it looks like we can go to that location. */
	return 1;
}

/* Returns whether the game agent can occupy the given vector on the
   game board.  This tests to make sure all four corners of the game
   agent can occupy the space on the game board. */
int agent_is_position_viable(GameAgent *ga, FixedVector *v)
{
	FixedVector topRight;
	FixedVector botLeft;
	FixedVector botRight;

	botRight = fixed_vector_add(v, &ga->physical_dim);
	botLeft.x = v->x; botLeft.y = v->y + ga->physical_dim.y;
	topRight.x = v->x + ga->physical_dim.x; topRight.y = v->y + ga->physical_dim.y;

	return ( agent_is_position_viable_helper(ga, v) &&
			 agent_is_position_viable_helper(ga, &topRight) &&
			 agent_is_position_viable_helper(ga, &botLeft) &&
			 agent_is_position_viable_helper(ga, &botRight) );
}

/* Moves the game agent in its current direction, assuming the
   given amount of time has passed.  Returns 1 if the move was
   successful, 0 otherwise. */
int agent_move(GameAgent *ga, Uint32 time)
{
	int last_block_x, last_block_y, next_block_x, next_block_y;
	int last_block_x2, last_block_y2, next_block_x2, next_block_y2;

	int block_changed;
	FixedVector v1, v2, ga_loc2;

	/* Scale the normalized movement vector relative to the game agent's speed. */
	v1 = fixed_vector_scale(&ga->curr_move, ga->speed);

	/* Now scale that vector to the amount of time that's passed. */
	v1 = fixed_vector_scale(&v1, FIXED_SET_INT(time));

	/* Now add that delta vector to the game agent's current position to
	   get their predicted new position. */
	v1 = fixed_vector_add(&ga->loc, &v1);

	/* If we've crossed a block boundary and we're not on the
	   first pixel of the block boundary, put us on it. */

	/* Block coordinates of the game agent's "last" (actually current) position,
	   relative to the top-right of the game agent. */
	last_block_x = GET_BLOCK_FIXED(ga->loc.x);
	last_block_y = GET_BLOCK_FIXED(ga->loc.y);	

	/* Block coordinates of the game agent's "next" (predicted) position, relative
	   to the top-left of the game agent. */
	next_block_x = GET_BLOCK_FIXED(v1.x);
	next_block_y = GET_BLOCK_FIXED(v1.y);

	/* Now calculate the same info for the bottom-right corner of the game agent. 
	   This is used if the player is moving left or up. */

	ga_loc2 = fixed_vector_add(&ga->loc, &ga->physical_dim);
	v2 = fixed_vector_add(&v1, &ga->physical_dim);

	last_block_x2 = GET_BLOCK_FIXED(ga_loc2.x);
	last_block_y2 = GET_BLOCK_FIXED(ga_loc2.y);	

	next_block_x2 = GET_BLOCK_FIXED(v2.x);
	next_block_y2 = GET_BLOCK_FIXED(v2.y);

	/* block_changed is a boolean that tells us whether the game agent
	   has crossed a block boundary or not. */
	block_changed = 0;
	/* Figure out if we've passed a block boundary.  If we have, we may need
	   to change our next position if it's not exactly on a block boundary.  If
	   this happens, v1 will be changed. */
	if (last_block_x < next_block_x) {
		/* If we're moving to the right (our next block is more than our last one)... */
		block_changed = 1;
		if (FIXED_GET_INT(v1.x) > next_block_x * BLOCK_SIZE)
			v1.x = FIXED_SET_INT(next_block_x * BLOCK_SIZE);
	} else if (last_block_x2 > next_block_x2) {
		/* If we're moving to the left... */
		block_changed = 1;
		if (FIXED_GET_INT(v1.x) < next_block_x2 * BLOCK_SIZE)
			v1.x = FIXED_SET_INT(next_block_x2 * BLOCK_SIZE);
	} else if (last_block_y < next_block_y) {
		/* If we're moving down... */
		block_changed = 1;
		if (FIXED_GET_INT(v1.y) > next_block_y * BLOCK_SIZE)
			v1.y = FIXED_SET_INT(next_block_y * BLOCK_SIZE);
	} else if (last_block_y2 > next_block_y2) {
		/* If we're moving up... */
		block_changed = 1;
		if (FIXED_GET_INT(v1.y) < next_block_y2 * BLOCK_SIZE)
			v1.y = FIXED_SET_INT(next_block_y2 * BLOCK_SIZE);
	}

	if (agent_is_position_viable(ga, &v1)) {
			ga->last_loc = ga->loc;
			ga->loc = v1;
			/* If we passed into a new block, alert the game agent's state machine (FSM). */
			if (block_changed) {
				if (FIXED_GET_INT(ga->loc.x) == (BOARD_WIDTH+2)*BLOCK_SIZE) {
					/* If we've gone through a tunnel to the right, wrap around
					   to the left side of the screen. */
					fixed_vector_set(&ga->loc, -1*BLOCK_SIZE, FIXED_GET_INT(ga->loc.y));
					ga->last_loc = ga->loc;
				} else if (FIXED_GET_INT(ga->loc.x) == -1*BLOCK_SIZE) {
					/* If we've gone through a tunnel to the left, wrap around
					   to the right side of the screen. */
					fixed_vector_set(&ga->loc, (BOARD_WIDTH+2)*BLOCK_SIZE, FIXED_GET_INT(ga->loc.y));
					ga->last_loc = ga->loc;
				}
				state_send_message(GAME_AGENT_MSG_BLOCK_CHANGE, 0, ga->state.state_id, 0, 0);
			}
			return 1;
	} else return 0;
}

/* Puts the bounding rectangle of the game agent's sprite into the rect "r", given
   the x and y offset of the sprite in pixels. */
void agent_get_draw_bounding_rect(GameAgent *ga, SDL_Rect *r, int x_ofs, int y_ofs)
{
	r->x = (Sint16) ((FIXED_GET_INT(ga->loc.x) + FIXED_GET_INT(ga->graphical_offset.x)) + x_ofs);
	r->y = (Sint16) ((FIXED_GET_INT(ga->loc.y) + FIXED_GET_INT(ga->graphical_offset.y)) + y_ofs);

	r->h = (Uint16) FIXED_GET_INT(ga->graphical_dim.x);
	r->w = (Uint16) FIXED_GET_INT(ga->graphical_dim.y);
}

/* Fills the game agent's OLD location (its location last frame) with the same bounding
   rectangle from the given background surface.  Used for dirty rectangle animation. */
void agent_replace_background(GameAgent *ga, SDL_Surface *surface, SDL_Surface *background, int x_ofs, int y_ofs)
{
	SDL_Rect r_src;
	SDL_Rect r_dst;

	r_src.x = (Sint16) (FIXED_GET_INT(ga->last_loc.x) + FIXED_GET_INT(ga->graphical_offset.x));
	r_src.y = (Sint16) (FIXED_GET_INT(ga->last_loc.y) + FIXED_GET_INT(ga->graphical_offset.y));

	r_src.h = r_dst.h = (Uint16) FIXED_GET_INT(ga->graphical_dim.x);
	r_src.w = r_dst.w = (Uint16) FIXED_GET_INT(ga->graphical_dim.y);

	r_dst.x = (Sint16) (r_src.x + x_ofs);
	r_dst.y = (Sint16) (r_src.y + y_ofs);

	game_update_rect_add(&r_dst);
	SDL_BlitSurface(background, &r_src, surface, &r_dst);
}

/* Draws the game agent to the given surface. */
void agent_draw(GameAgent *ga, SDL_Surface *surface, int x_ofs, int y_ofs)
{
	if (!ga->is_visible) return;

	/* This is a yucky sort of virtual method... */
	if (ga->agent_type == GAME_AGENT_PMAN) {
		agent_pman_draw(ga, surface, x_ofs, y_ofs);
	} else if (ga->agent_type == GAME_AGENT_GHOST) {
		agent_ghost_draw(ga, surface, x_ofs, y_ofs);		
	} else if (ga->agent_type == GAME_AGENT_FRUIT) {
		agent_fruit_draw(ga, surface, x_ofs, y_ofs);
	}
}

/* Draws the score amount of the game agent into the center of the given rectangle.
   Used when a ghost or fruit is gobbled for points. */
void agent_draw_score_amount(GameAgent *ga, SDL_Surface *surface, SDL_Rect *r)
{
	char buffer[50];
	Font *f;
	int x1, y1;

	f = game_get_font_small();
	sprintf(buffer, "%d", ga->ghost_score_amount);
	x1 = r->x + (r->w / 2);
	y1 = r->y + (r->h / 2);
	font_draw_string_centered(f, surface, x1, y1, buffer);
}

/* Toggles whether the game agent is visible or not. */
void agent_toggle_visible(GameAgent *ga)
{
	if (ga->is_visible)
		ga->is_visible = 0;
	else
		ga->is_visible = 1;

	/* No need to set view redraw flag b/c agent is redrawn every frame regardless. */
}
