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

/* Restarts the pac man game agent.  Should be called whenever a
   new level starts. */
void agent_pman_restart(GameAgent *ga)
{
	ga->frame_curr = 0;
	ga->is_visible = 1;
	ga->speed = fixed_from_float( CONVERT_PPDS_TO_PPMS(PMAN_BASE_SPEED) );
	ga->curr_move = fixed_vector_zero;
	ga->next_move = fixed_vector_zero;
	fixed_vector_set(&ga->loc, PMAN_START_BLOCK_X*BLOCK_SIZE+(BLOCK_SIZE/2), PMAN_START_BLOCK_Y*BLOCK_SIZE);
	ga->last_loc = ga->loc;
	state_construct(&ga->state, STATE_ID_AGENT_PMAN, STATE_ID_AGENT_PMAN, ga, TIMER_ID_GAME_AGENT);

	if (ga->pman_ai_flag) {
		agent_set_next_move(ga, &fixed_vector_left);
	} else {
		agent_set_next_move(ga, &fixed_vector_zero);
	}
}

/* Initializes the pac man game agent.  This should take care of any
   dynamic memory allocation that needs to be done, and should only
   really be called once per gameplay session. */
void agent_pman_init(GameAgent *ga)
{
	ga->agent_type = GAME_AGENT_PMAN;
	fixed_vector_set(&ga->graphical_offset, -3, -3);
	fixed_vector_set(&ga->graphical_dim, BLOCK_SIZE+6, BLOCK_SIZE+6);
	fixed_vector_set(&ga->physical_dim, BLOCK_SIZE-1, BLOCK_SIZE-1);
	ga->color = game_map_rgb(255, 255, 0);
	agent_pman_generate_frames(ga);
	ga->frame_speed = fixed_from_float( CONVERT_PPDS_TO_PPMS(PMAN_FRAME_SPEED) );
	ga->pman_ai_flag = pman_in_demo_mode();
}

/* Free all memory dynamically allocated by agent_pman_init(). */
void agent_pman_destroy(GameAgent *ga)
{
	SDL_FreeSurface(ga->frames);
}

/* Draw the pac man game agent to the given surface. */
void agent_pman_draw(GameAgent *ga, SDL_Surface *surface, int x_ofs, int y_ofs)
{
	SDL_Rect r_src, r_dst;

	r_src.w = (Uint16) FIXED_GET_INT(ga->graphical_dim.x);
	r_src.h = (Uint16) FIXED_GET_INT(ga->graphical_dim.y);

	r_src.x = (Sint16) FIXED_GET_INT(ga->frame_curr) * r_src.w;
	r_src.y = (Sint16) 0;

	agent_get_draw_bounding_rect(ga, &r_dst, x_ofs, y_ofs);

	game_update_rect_add(&r_dst);

	SDL_BlitSurface(ga->frames, &r_src, surface, &r_dst);
}

/* Advance the frame of pac man's current movement animation, given
   the amount of time (in ms) that has passed. */
void agent_pman_frame_advance(GameAgent *ga, Uint32 time)
{
	int anim_start_frame = -1;
	int anim_end_frame;

	anim_start_frame = map_fixed_vector_to_direction(&ga->curr_move);
	if (anim_start_frame == -1) return;
	anim_start_frame *= PMAN_FRAMES_PER_DIRECTION;

	anim_end_frame = anim_start_frame + PMAN_FRAMES_PER_DIRECTION;

	ga->frame_curr += fixed_mult(FIXED_SET_INT(time), ga->frame_speed);

	if (FIXED_GET_INT(ga->frame_curr) >= anim_end_frame-2) {
		ga->frame_curr = FIXED_SET_INT(anim_end_frame-1-2);
		ga->frame_speed = -abs(ga->frame_speed);
	} else if (FIXED_GET_INT(ga->frame_curr) < anim_start_frame) {
		ga->frame_curr = FIXED_SET_INT(anim_start_frame);
		ga->frame_speed = abs(ga->frame_speed);
	}
}

/* Generate the frames for pac man's movement animation. */
void agent_pman_generate_frames(GameAgent *ga)
{
	int w, h;
	int f, i;
	int radius;
	int x, y;
	SDL_Surface *s;

	w = FIXED_GET_INT(ga->graphical_dim.x);
	h = FIXED_GET_INT(ga->graphical_dim.y);
	
	s = game_create_bitmap(0, w * PMAN_FRAMES_PER_DIRECTION * 4, h);

	assert(s != NULL);

	radius = (h / 2) - 1;

	x = y = 0;
	for (f = 0; f < 4; f++) {
		for (i = 0; i < PMAN_FRAMES_PER_DIRECTION; i++) {
			int center_x = x + radius;
			int center_y = y + radius;

			pman_draw(s, center_x, center_y, radius, i, 1, f, ga->color);

			//drawCircle(s, center_x, center_y, radius, FILL_FULL, ga->color);
			//drawPmanWedge(s, center_x, center_y, radius, i, 1, f);
			x += w;
		}
	}
	SDL_SetColorKey(s, SDL_SRCCOLORKEY | SDL_RLEACCEL, game_map_rgb(0,0,0));
	ga->frames = SDL_DisplayFormatAlpha(s);
	assert(ga->frames != NULL);

	SDL_FreeSurface(s);
}

/* Draw a pac man sprite to the screen. */
void pman_draw(SDL_Surface *screen, int x1, int y1, int r, int mouth_open, int mouth_inset, int direction, Uint32 color)
{
	drawCircle(screen, x1, y1, r, FILL_FULL, color);
	pman_draw_wedge(screen, x1, y1, r, mouth_open, mouth_inset, direction);
}

/* Draws a black pman "wedge".  Should be drawn on top of a circle to "cut out" a wedge and
   make it look like pac man.

   r - radius of the circle
   mouth_open - "radius" of mouth showing (the actual opening of the wedge will be this
     number times 2)
   mouth_inset - where the mouth begins.  If this is 0, it will be at the center of the
     circle, but if it's 1, it will be 1 pixel away from the center.
   segment - where the wedge is placed.  Should be one of the DIRECTION_* constants.
*/
void pman_draw_wedge(SDL_Surface *screen, int x1, int y1, int r, int mouth_open, int mouth_inset, int segment)
{
	int curr_val = (5 - r*4)/4;

	int x = 0;
	int y = r;

	int p1_x, p1_y, p2_x, p2_y;
	SDL_Rect rect;

	while (x < y) {
		x++;
		if (curr_val >= 0) {
			curr_val += 2 + (x << 1) - (y << 1);
			y--;
		} else {
			curr_val += 1 + (x << 1);
		}

		switch (segment) {
			case DIRECTION_UP:
				// wedge at top of pman
				//drawLine(screen, x1-x, y1-y, x1+x, y1-y, 0);
				if ( x == mouth_open ) {
					p1_x = x1-x +1;
					p1_y = y1-y;
					p2_x = x1+x -1;
					p2_y = y1-y;
					drawFlatTopTriangle(screen, p1_x, p1_y, x1, y1 + mouth_inset, p2_x, p2_y, 0);
					rect.x = (Sint16) p1_x;
					rect.y = (Sint16) (y1-r);
					rect.h = (Uint16) (p1_y - rect.y);
					rect.w = (Uint16) (p2_x - rect.x);
					SDL_FillRect(screen, &rect, 0);
					return;
				}
			case DIRECTION_DOWN:
				// wedge at bottom of pman
				// drawLine(screen, x1-x, y1+y, x1+x, y1+y, 0);
				if ( x == mouth_open ) {
					p1_x = x1-x +1;
					p1_y = y1+y;
					p2_x = x1+x -1;
					p2_y = y1+y;
					drawFlatBottomTriangle(screen, p1_x, p1_y, x1, y1 - mouth_inset, p2_x, p2_y, 0);
					rect.x = (Sint16) p1_x;
					rect.y = (Sint16) p1_y;
					rect.h = (Uint16) ((y1+r) - rect.y + 1);
					rect.w = (Uint16) (p2_x - rect.x);
					SDL_FillRect(screen, &rect, 0);
					return;
				}
			case DIRECTION_LEFT:
				// wedge at left of pman
				// drawLine(screen, x1-y, y1-x, x1-y, y1+x, 0);
				if ( x == mouth_open ) {
					p1_x = x1-y;
					p1_y = y1-x +1;
					p2_x = x1-y;
					p2_y = y1+x -1;
					drawFlatBottomTriangle(screen, p1_x, y1, p1_x, p1_y, x1 + mouth_inset, y1, 0);
					drawFlatTopTriangle(screen, p2_x, y1, p2_x, p2_y, x1 + mouth_inset, y1, 0);
					rect.x = (Sint16) (x1 - r);
					rect.y = (Sint16) p1_y;
					rect.h = (Uint16) (p2_y - p1_y);
					rect.w = (Uint16) (p1_x - rect.x);
					SDL_FillRect(screen, &rect, 0);
					return;
				}
			case DIRECTION_RIGHT:
				// wedge at right of pman
				// drawLine(screen, x1+y, y1-x, x1+y, y1+x, 0);
				if ( x == mouth_open ) {
					p1_x = x1+y;
					p1_y = y1-x +1;
					p2_x = x1+y;
					p2_y = y1+x -1;
					drawFlatBottomTriangle(screen, x1 - mouth_inset, y1, p1_x, p1_y, p1_x, y1, 0);
					drawFlatTopTriangle(screen, x1 - mouth_inset, y1, p2_x, p2_y, p2_x, y1, 0);
					rect.x = (Sint16) p1_x;
					rect.y = (Sint16) p1_y;
					rect.h = (Uint16) (p2_y - p1_y);
					rect.w = (Uint16) ((x1+r) - rect.x + 1);
					SDL_FillRect(screen, &rect, 0);
					return;
				}
		}

		/* Drawing a circle in vertical strips: */
		// left-middle segment
		// drawLine(screen, x1-x, y1-y, x1-x, y1+y, 0);
		// right-middle segment
		// drawLine(screen, x1+x, y1-y, x1+x, y1-y, 0);
		// leftmost segment
		// drawLine(screen, x1-y, y1-x, x1-y, y1+x, 0);
		// rightmost segment
		// drawLine(screen, x1+y, y1-x, x1+y, y1+x, 0);

		/* Drawing a circle in horizontal strips: */
		// topmost segment
		// drawLine(screen, x1-x, y1-y, x1+x, y1-y, 0);
		// top-middle segment
		// drawLine(screen, x1-y, y1-x, x1+y, y1-x, 0);
		// bottom-middle segment
		// drawLine(screen, x1-y, y1+x, x1+y, y1+x, 0);
		// bottommost segment
		// drawLine(screen, x1-x, y1+y, x1+x, y1+y, 0);
	}
}

/* The pac man game agent state machine. */
BEGIN_STATE_MACHINE(agent_pman_state_machine)
	GameAgent *pman = (GameAgent *) s->parent;
	STATE_MACHINE_HEADER
	ON_UPDATE
		int move_result;
		Uint32 time = *(Uint32 *) sm->data;

		move_result = agent_move(pman, time);
		agent_pman_frame_advance(pman, time);
		if (!move_result)
			if (!agent_next_move(pman))
				agent_set_move(pman, &fixed_vector_zero);
			else if (pman->pman_ai_flag)
				agent_determine_next_random_move(pman);
	ON_MSG(GAME_AGENT_MSG_BLOCK_CHANGE)
		if (pman->pman_ai_flag)
			agent_determine_next_random_move(pman);
		else
			agent_next_move(pman);
		state_send_message(BOARD_MSG_PMAN_ON_BLOCK, sm->to, STATE_ID_BOARD, 0, &pman->loc);
END_STATE_MACHINE
