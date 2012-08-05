#include "globals.h"

#include <stdlib.h>
#include <assert.h>

#include "SDL.h"

#include "game.h"
#include "state.h"
#include "debug.h"
#include "fixed.h"
#include "pman.h"
#include "pman_board.h"
#include "pman_score.h"
#include "pman_agent.h"
#include "pman_agent_ghost.h"
#include "pman_agent_pman.h"
#include "pman_agent_fruit.h"
#include "menu.h"

/* At the given block on the board, returns the cardinal direction (as
   a fixed vector) in which to go to get back to the asylum. */
FixedVector board_get_asylum_directions_at_block(Board *b, int x, int y)
{
	assert(y >= 0 && y < BOARD_HEIGHT);

	if (x < 0 || x >= BOARD_WIDTH) {
		return fixed_vector_left;
	} else {
		return b->goto_asylum_directions[x][y];
	}
}

int board_generate_asylum_directions_helper(Board *b, int temp_board[BOARD_WIDTH][BOARD_HEIGHT], int curr_iter, int x, int y, const FixedVector *direction)
{
	int next_x = x + FIXED_GET_INT(direction->x);
	int next_y = y + FIXED_GET_INT(direction->y);

	if (next_x < 0 || next_x >= BOARD_WIDTH || next_y < 0 || next_y >= BOARD_HEIGHT)
		return 0;

	if ( (board_get_block(b, next_x, next_y) != BLOCK_WALL ) &&
		 (temp_board[next_x][next_y] == 0) ) {
			 temp_board[next_x][next_y] = curr_iter;
			 b->goto_asylum_directions[next_x][next_y] = fixed_vector_reverse(direction);
			 return 1;
	}
	return 0;
}

/* Generates the array that gives information about how to get back to the
   asylum from any given block on the board.  The directions are calculated
   using Moore's Breadth-First Search algorithm. */
void board_generate_asylum_directions(Board *b)
{
	int temp_board[BOARD_WIDTH][BOARD_HEIGHT];
	int curr_iter;
	int i, j;

	for (i = 0; i < BOARD_WIDTH; i++) {
		for (j = 0; j < BOARD_HEIGHT; j++) {
			temp_board[i][j] = 0;
			b->goto_asylum_directions[i][j] = fixed_vector_zero;
		}
	}
	temp_board[BLOCK_ASYLUM_CENTER_X][BLOCK_ASYLUM_ENTER_Y] = 1;
	temp_board[BLOCK_ASYLUM_CENTER_X+1][BLOCK_ASYLUM_ENTER_Y] = 1;

	curr_iter = 1;

	while (1) {
		int blocks_found;

		blocks_found = 0;
		for (j = 0; j < BOARD_HEIGHT; j++) {
			for (i = 0; i < BOARD_WIDTH; i++) {
				if (temp_board[i][j] == curr_iter) {
						blocks_found += 
							board_generate_asylum_directions_helper(b, temp_board, curr_iter+1, i, j, &fixed_vector_left) +
							board_generate_asylum_directions_helper(b, temp_board, curr_iter+1, i, j, &fixed_vector_right) +
							board_generate_asylum_directions_helper(b, temp_board, curr_iter+1, i, j, &fixed_vector_up) +
							board_generate_asylum_directions_helper(b, temp_board, curr_iter+1, i, j, &fixed_vector_down);
				}
			}
		}
		if (blocks_found == 0) break;
		curr_iter++;
	}
}

/* Destroys the nibblet/nibbloon on the given board at the given block coordinates,
   if there's actually a nib there. */
void board_destroy_nib(Board *b, int x, int y)
{
	if (board_get_block(b, x, y) == BLOCK_NIBBLET || board_get_block(b, x, y) == BLOCK_NIBBLOON) {
		SDL_Rect r;
		int block_type_eaten = board_get_block(b, x, y);

		b->blocks[x][y] = BLOCK_NOTHING;
		r.x = (Sint16) (x * BLOCK_SIZE);
		r.y = (Sint16) (y * BLOCK_SIZE);
		r.w = BLOCK_SIZE;
		r.h = BLOCK_SIZE;
		SDL_FillRect(b->background, &r, 0);
		b->nibs_left--;
		if (block_type_eaten == BLOCK_NIBBLOON) {
			// send message to ghosts, change music, etc...
			state_send_message( PLAY_STATE_MSG_NIBBLOON_EATEN, STATE_ID_BOARD, STATE_ID_PLAY_STATE, 0, 0 );
		} else {
			state_send_message( PLAY_STATE_MSG_NIBBLET_EATEN, STATE_ID_BOARD, STATE_ID_PLAY_STATE, 0, 0 );
		}
		if (b->nibs_left == 0) {
			state_send_message( PLAY_STATE_MSG_LEVEL_WON, STATE_ID_BOARD, STATE_ID_PLAY_STATE, 0, 0 );
		}
	}
}

/* Returns the block ID of the block at the given block coordinates. */
int board_get_block(Board *b, int x, int y)
{
	//assert(x >= 0); assert(y >= 0);
	//assert(x < BOARD_WIDTH); assert(y < BOARD_HEIGHT);
	if (x < 0 || y < 0 || x >= BOARD_WIDTH || y >= BOARD_HEIGHT) {
		return BLOCK_NOTHING;
	}
	return b->blocks[x][y];
}

/* Loads the board's block data (e.g., walls, nibs, pathways) from a
   board data file (which is a BMP image). */
void board_load_data(Board *b)
{
	SDL_Surface *s;
	int i,j;
	char *pixels;

	s = game_load_bmp(BOARD_FILE_NAME);

	assert(s != NULL);
	assert(s->w == BOARD_WIDTH);
	assert(s->h == BOARD_HEIGHT);
	assert(s->pitch == BOARD_WIDTH);

	SDL_LockSurface(s);
	pixels = (char *)s->pixels;

	b->nibs_left = 0;

	for (j = 0; j < BOARD_HEIGHT; j++) {
		for (i = 0; i < BOARD_WIDTH; i++) {
			b->blocks[i][j] = pixels[j * BOARD_WIDTH + i];
			if (b->blocks[i][j] == BLOCK_NIBBLET ||
				b->blocks[i][j] == BLOCK_NIBBLOON)
				b->nibs_left++;
		}
	}
	SDL_UnlockSurface(s);
	SDL_FreeSurface(s);

	board_generate_asylum_directions(b);
}

/* Helper function for the board_redraw_walls() function that returns whether
   the given block coordinates contain a wall, an asylum door, or asylum space.
   Used for figuring out what wall tile to use. */
int board_redraw_walls_is_block_wall(Board *b, int x, int y)
{
	if (x < 0 || y < 0 || x >= BOARD_WIDTH || y >= BOARD_HEIGHT)
		return 1;
	if (b->blocks[x][y] == BLOCK_WALL || b->blocks[x][y] == BLOCK_ASYLUM_DOOR ||
		b->blocks[x][y] == BLOCK_ASYLUM_SPACE) return 1;
	return 0;
}

/* Helper function for board_redraw_walls().  Given the given board and block coordinates,
   determines the right kind of wall tile to use and blits it to the location. */
void board_redraw_walls_draw_wall(Board *b, SDL_Rect *dst_rect, int x, int y)
{
	SDL_Surface *surface;
	SDL_Rect src_rect;

	surface = b->background;
	src_rect.h = BLOCK_SIZE;
	src_rect.w = BLOCK_SIZE;

	src_rect.x = src_rect.y = -1;

	if (b->blocks[x][y] == BLOCK_ASYLUM_DOOR) {
		src_rect.x = 0;
		src_rect.y = 3;
	} else if (!board_redraw_walls_is_block_wall(b, x, y-1)) {
		// if the spot to the top is blank
		if (!board_redraw_walls_is_block_wall(b, x-1, y)) {
			// top-left wall
			src_rect.x = 3;
			src_rect.y = 1;
		} else if (!board_redraw_walls_is_block_wall(b, x+1, y)) {
			// top-right wall
			src_rect.x = 2;
			src_rect.y = 1;
		} else {
			// top wall
			src_rect.x = 3;
			src_rect.y = 0;			
		}
	} else if (!board_redraw_walls_is_block_wall(b, x, y+1)) {
		// if the spot to the bottom is blank
		if (!board_redraw_walls_is_block_wall(b, x-1, y)) {
			// bottom-left wall
			src_rect.x = 1;
			src_rect.y = 1;
		} else if (!board_redraw_walls_is_block_wall(b, x+1, y)) {
			// bottom-right wall
			src_rect.x = 0;
			src_rect.y = 1;
		} else {
			// bottom wall
			src_rect.x = 2;
			src_rect.y = 0;			
		}
	} else if (!board_redraw_walls_is_block_wall(b, x-1, y)) {
		// left wall
		src_rect.x = 1;
		src_rect.y = 0;
	} else if (!board_redraw_walls_is_block_wall(b, x+1, y)) {
		// right wall
		src_rect.x = 0;
		src_rect.y = 0;
	} else if (!board_redraw_walls_is_block_wall(b, x+1, y+1)) {
		// juncture w/ space at bottom-right
		src_rect.x = 2;
		src_rect.y = 2;
	} else if (!board_redraw_walls_is_block_wall(b, x+1, y-1)) {
		// juncture w/ space at top-right
		src_rect.x = 1;
		src_rect.y = 2;
	} else if (!board_redraw_walls_is_block_wall(b, x-1, y-1)) {
		// juncture w/ space at top-left
		src_rect.x = 0;
		src_rect.y = 2;
	} else if (!board_redraw_walls_is_block_wall(b, x-1, y+1)) {
		// juncture w/ space at bottom-left
		src_rect.x = 3;
		src_rect.y = 2;
	}

	if (src_rect.x == -1) return;

	src_rect.x *= BLOCK_SIZE;
	src_rect.y *= BLOCK_SIZE;

	SDL_BlitSurface(b->walls_bitmap, &src_rect, surface, dst_rect);
}

/* Blits the board's walls to the board's background.  This includes asylum doors. */
void board_redraw_walls(Board *b)
{
	int i;
	int j;
	SDL_Rect r;

	r.w = BLOCK_SIZE;
	r.h = BLOCK_SIZE;

	for (j = 0; j < BOARD_HEIGHT; j++) {
		r.y = (Sint16) (BLOCK_SIZE * j);
		for (i = 0; i < BOARD_WIDTH; i++) {
			r.x = (Sint16) (BLOCK_SIZE * i);
			if (b->blocks[i][j] == BLOCK_WALL ||
				b->blocks[i][j] == BLOCK_ASYLUM_DOOR)
				board_redraw_walls_draw_wall(b, &r, i, j);
		}
	}
}

/* Blits the board's nibblets and nibbloons to the board's background. */
void board_redraw_nibbles(Board *b)
{
	int i;
	int j;
	SDL_Rect r;
	SDL_Surface *surface = b->background;
	
	for (j = 0; j < BOARD_HEIGHT; j++) {
		for (i = 0; i < BOARD_WIDTH; i++) {
			r.y = (Sint16) (BLOCK_SIZE * j);
			r.x = (Sint16) (BLOCK_SIZE * i);
			if (b->blocks[i][j] == BLOCK_NIBBLET) {
				r.x += (BLOCK_SIZE / 2) - 1;				
				r.y += (BLOCK_SIZE / 2) - 1;
				r.w = 2;
				r.h = 2;
				SDL_FillRect(surface, &r, game_map_rgb(255, 255, 0));
			}
			if (b->blocks[i][j] == BLOCK_NIBBLOON) {
				r.x += (BLOCK_SIZE / 2) - 2;
				r.y += (BLOCK_SIZE / 2) - 2;
				r.w = 4;
				r.h = 4;
				SDL_FillRect(surface, &r, game_map_rgb(255,255,0));
			}
		}
	}
}

/* Generates the board's background surface (the walls and nibbleats/nibbloons). */
void board_generate_background(Board *b)
{
	board_redraw_walls(b);
	board_redraw_nibbles(b);
}

/* Restarts the game board.  Should be called whenever a new level is started. */
void board_restart(Board *b, int reload_board_data)
{
	b->is_visible = 1;
	if (reload_board_data) board_load_data(b);
	board_generate_background(b);
	state_construct(&b->state, STATE_ID_BOARD, STATE_ID_BOARD, b, TIMER_ID_GAME);
	agent_pman_restart(&b->pman);

	agent_ghost_restart(&b->ghosts[0], 12, 11, STATE_ID_AGENT_GHOST_1, STATE_ID_AGENT_GHOST_1, 0, 0);
	agent_ghost_restart(&b->ghosts[1], BLOCK_ASYLUM_CENTER_X-2, 14, STATE_ID_AGENT_GHOST_2, STATE_ID_AGENT_GHOST_1, GHOST_STATE_RESTING, 10);
	agent_ghost_restart(&b->ghosts[2], BLOCK_ASYLUM_CENTER_X, 15, STATE_ID_AGENT_GHOST_3, STATE_ID_AGENT_GHOST_1, GHOST_STATE_RESTING, 5);
	b->ghosts[2].loc.x = FIXED_SET_INT(BLOCK(BLOCK_ASYLUM_CENTER_X) + (BLOCK_SIZE / 2));
	agent_ghost_restart(&b->ghosts[3], BLOCK_ASYLUM_CENTER_X+3, 14, STATE_ID_AGENT_GHOST_4, STATE_ID_AGENT_GHOST_1, GHOST_STATE_RESTING, 15);

	agent_fruit_restart(&b->fruit);
}

/* Initializes the game board by dynamically allocating memory, etc.  Should be
   used only once per game session, and always countered with board_destroy(). */
void board_init(Board *b, int x_ofs, int y_ofs)
{
	b->draw_rect.h = BOARD_PIXEL_HEIGHT;
	b->draw_rect.w = BOARD_PIXEL_WIDTH;
	b->draw_rect.x = (Uint16) x_ofs;
	b->draw_rect.y = (Uint16) y_ofs;

	b->background = game_create_bitmap(0, BOARD_WIDTH*BLOCK_SIZE, BOARD_HEIGHT*BLOCK_SIZE);
	b->walls_bitmap = game_load_bmp(BOARD_WALLS_FILE_NAME);
	agent_pman_init(&b->pman);

	agent_ghost_init(&b->ghosts[0], game_map_rgb(255, 0, 255));
	agent_ghost_init(&b->ghosts[1], game_map_rgb(0, 0, 255));
	agent_ghost_init(&b->ghosts[2], game_map_rgb(0, 255, 0));
	agent_ghost_init(&b->ghosts[3], game_map_rgb(255, 0, 0));

	agent_fruit_init(&b->fruit);
}

/* Deallocates memory gathered in board_init(). */
void board_destroy(Board *b)
{
	SDL_FreeSurface(b->background);
	SDL_FreeSurface(b->walls_bitmap);
	agent_pman_destroy(&b->pman);
	agent_fruit_destroy(&b->fruit);
}

/* Draws the game board to the given surface. */
void board_draw(Board *b, SDL_Surface *surface, int game_view_flags)
{
	SDL_Rect old_clip_rect;
	int i;

	/* Set the clipping rectangle of the game board to the surface, so we
	   don't blit outside it (this is used for when a game agent is in
	   the wraparound tunnel and part of it can't be seen). */
	SDL_GetClipRect(surface, &old_clip_rect);
	SDL_SetClipRect(surface, &b->draw_rect);

	if (b->is_visible) {
		if (game_view_flags & GAME_DRAW_FLAG_REDRAW)
			/* If we have to redraw the whole board, blit it to the surface. */
			SDL_BlitSurface(b->background, NULL, surface, &b->draw_rect);
		else {
			/* Otherwise, only blit the area of the board behind the location of
			   pac man and the ghosts in the LAST frame. */
			agent_replace_background(&b->pman, surface, b->background, b->draw_rect.x, b->draw_rect.y);
			for (i = 0; i < 4; i++) {
				agent_replace_background(&b->ghosts[i], surface, b->background, b->draw_rect.x, b->draw_rect.y);
			}
			agent_replace_background(&b->fruit, surface, b->background, b->draw_rect.x, b->draw_rect.y);
		}
	} else {
		/* If we're not visible, blit a big black box to where we're supposed
		   to be. */
		if (game_view_flags & GAME_DRAW_FLAG_REDRAW)
			SDL_FillRect(surface, &b->draw_rect, 0);
	}

	/* Draw the fruit. */
	agent_draw(&b->fruit, surface, b->draw_rect.x, b->draw_rect.y);

	/* Draw pac man and the ghosts. */
	agent_draw(&b->pman, surface, b->draw_rect.x, b->draw_rect.y);
	for (i = 0; i < 4; i++) {
		agent_draw(&b->ghosts[i], surface, b->draw_rect.x, b->draw_rect.y);
	}

	/* Restore the surface's old clipping rectangle. */
	SDL_SetClipRect(surface, &old_clip_rect);
}

/* The board controller handles input and connects keystrokes to the
   control of pac man. */
int board_controller(Board *b, SDL_Event *e)
{
	int input_handled = 0;
	if (e->type == SDL_KEYDOWN) {
		FixedVector new_move = fixed_vector_zero;

		switch (e->key.keysym.sym) {
			case SDLK_LEFT:
				new_move = fixed_vector_left;
				input_handled = 1;
				break;
			case SDLK_RIGHT:
				new_move = fixed_vector_right;
				input_handled = 1;
				break;

			case SDLK_UP:
				new_move = fixed_vector_up;
				input_handled = 1;
				break;

			case SDLK_DOWN:
				new_move = fixed_vector_down;
				input_handled = 1;
				break;

			case SDLK_ESCAPE:
				game_set_state(&menu_game_state);
				return 1;
				break;
		}
		if (!fixed_vector_equals(&new_move, &fixed_vector_zero)) {
			agent_set_next_move(&b->pman, &new_move);

			/* Old "move instantly if they want to reverse direction" code
			   is below.  Currently, pman can only reverse direction on a
			   block boundary, but if we want him to be able to reverse
			   direction instantly, uncomment this code. */
			/* FixedVector curr_move_reversed;

			curr_move_reversed = fixed_vector_scale(&g_pman.curr_move, FIXED_SET_INT(-1));
			if (fixed_vector_equals(&curr_move_reversed, &new_move))
				agent_set_move(&g_pman, &new_move);
			else
				agent_set_next_move(&g_pman, &new_move); */
		}
	}
	return input_handled;
}

/* Detects whether there are any collisions between pac man and the ghosts.
   If there are, the appropriate message is sent to the ghost's FSM. */
void board_detect_agent_collisions(Board *b)
{
	SDL_Rect r_pman, r_ghost, r_fruit;
	int i;

	fixed_vector_to_rect_coords(&b->pman.loc, &r_pman);
	fixed_vector_to_rect_dimensions(&b->pman.physical_dim, &r_pman);
	
	for (i = 0; i < 4; i++) {
		fixed_vector_to_rect_coords(&b->ghosts[i].loc, &r_ghost);
		fixed_vector_to_rect_dimensions(&b->ghosts[i].physical_dim, &r_ghost);

		if (rects_intersect(&r_pman, &r_ghost)) {
			/* TODO: make sure the intersection isn't over a diagonal block.
			   (pman should only die if he's in a cardinal direction away
			   from the ghost.) */
			//if (GET_BLOCK(r_pman.x) == GET_BLOCK(r_ghost.x) ||
			//	GET_BLOCK(r_pman.y) == GET_BLOCK(r_ghost.y))
				state_send_message( AGENT_MSG_HIT_PMAN, STATE_ID_BOARD, b->ghosts[i].state.state_id, 0, NULL );
		}
	}

	/* Test for fruit collision. */
	fixed_vector_to_rect_coords(&b->fruit.loc, &r_fruit);
	fixed_vector_to_rect_dimensions(&b->fruit.physical_dim, &r_fruit);
	if (rects_intersect(&r_pman, &r_fruit)) {
		state_send_message( AGENT_MSG_HIT_PMAN, STATE_ID_BOARD, b->fruit.state.state_id, 0, NULL );
	}
}

/* Toggles whether the board is visible or not. */
void board_toggle_visible(Board *b)
{
	if (b->is_visible)
		b->is_visible = 0;
	else
		b->is_visible = 1;

	game_set_draw_flags(GAME_DRAW_FLAG_REDRAW);
}

/* The game board's state machine function. */

BEGIN_STATE_MACHINE(board_state_machine)
	Board *board = (Board *) s->parent;
	int i;

	STATE_MACHINE_HEADER
	ON_ENTER
		state_send_message(STATE_MSG_OnEnter, STATE_ID_BOARD, STATE_ID_AGENT_PMAN, 0, NULL);
		for (i = 0; i < 4; i++) {
			state_send_message(STATE_MSG_OnEnter, STATE_ID_BOARD, STATE_ID_AGENT_GHOST_1+i, 0, NULL);
		}
		state_send_message(STATE_MSG_OnEnter, STATE_ID_BOARD, STATE_ID_AGENT_FRUIT, 0, NULL);
	ON_UPDATE
		state_timer_update(TIMER_ID_GAME_AGENT, *(Uint32 *) sm->data);
		state_send_message(STATE_MSG_OnUpdate, 0, STATE_ID_AGENT_PMAN, 0, sm->data);
		for (i = 0; i < 4; i++) {
			state_send_message(STATE_MSG_OnUpdate, 0, STATE_ID_AGENT_GHOST_1+i, 0, sm->data);
		}
		board_detect_agent_collisions(board);
	ON_MSG(BOARD_MSG_PMAN_ON_BLOCK)
		FixedVector *v;
		int x,y;

		v = (FixedVector *) sm->data;
		x = GET_BLOCK_FIXED(v->x);
		y = GET_BLOCK_FIXED(v->y);

		board_destroy_nib(board,x,y);
END_STATE_MACHINE
