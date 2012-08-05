#ifndef INCLUDE_PMAN_BOARD
#define INCLUDE_PMAN_BOARD

/* pman_board.h
   
   Defines the functions and data encapsulating the game board.
*/

#include "SDL.h"

#include "pman_agent.h"
#include "state.h"

/* BOARD_FILE_NAME is the location and name of the image file that
   represents the board's blocks.  It is NOT an image file that is
   ever blitted to the screen; it's an 8-bit image where each palette
   entry corresponds to a BLOCK_* constant. */
#define BOARD_FULL_FILE_NAME "pman_board03.bmp"
#define BOARD_EMPTY_FILE_NAME "pman_board03_empty.bmp"
#define BOARD_FILE_NAME BOARD_FULL_FILE_NAME

/* This is the filename and location of the image that contains the
   tile graphics for the board's walls graphics. */
#define BOARD_WALLS_FILE_NAME "pman_walls03.bmp"

/* Width, height of the board in blocks. */
#define BOARD_WIDTH  28
#define BOARD_HEIGHT 31

/* Width and height of each block in pixels. */
#define BLOCK_SIZE 14

#define BOARD_PIXEL_WIDTH (BOARD_WIDTH*BLOCK_SIZE)
#define BOARD_PIXEL_HEIGHT (BOARD_HEIGHT*BLOCK_SIZE)

/* The BLOCK_* constants refer to the values of a
   game board's blocks[][] array and signify the type
   of block at a given location on the game board. */
#define BLOCK_NOTHING        3
#define BLOCK_WALL           0
#define BLOCK_NIBBLET        1
#define BLOCK_NIBBLOON       2
#define BLOCK_ASYLUM_DOOR    4
#define BLOCK_ASYLUM_SPACE   5

/* Base x-coordinate, in blocks, of the asylum's center.  Note that
   the actual center will be this value plus half a block, since the width
   of the asylum in blocks is even. */
#define BLOCK_ASYLUM_CENTER_X 13
/* y-coordinate of the spot on the board where ghosts should go to respawn after
   they die. visually, this should be in the center of the asylum. */
#define BLOCK_ASYLUM_CENTER_Y 14
/* The exact pixel that the upper-left of a ghost should be at to be in the center of the
   asylum. */
#define BLOCK_ASYLUM_CENTER_PIXEL_X (BLOCK_ASYLUM_CENTER_X*BLOCK_SIZE + (BLOCK_SIZE/2))
#define BLOCK_ASYLUM_CENTER_PIXEL_Y (BLOCK_ASYLUM_CENTER_Y*BLOCK_SIZE)

/* y-coordinate of the spot on the board where ghosts should go to enter the
   asylum, and the y-coordinate of where they should be heading to to leave
   it.  visually, this should be right above the asylum door. */
#define BLOCK_ASYLUM_ENTER_Y  11
#define BLOCK_ASYLUM_EXIT_Y  12

/* Pacman just landed on a new block! */
#define BOARD_MSG_PMAN_ON_BLOCK 10

/* Number of times to flash the board when the player wins a level. */
#define BOARD_WIN_FLASH_TIMES 10

/* Number of ms to delay between each flash of the game board. */
#define BOARD_WIN_FLASH_DELAY 500

/* Convert the given block number to its pixel coordinate equivalent. */
#define BLOCK(x) ((x) * BLOCK_SIZE)

/* Find out what block a given pixel coordinate is in. */
#define GET_BLOCK(x) (int)((x) / BLOCK_SIZE)

/* Find out what block a given fixed-point pixel coordinate is in. */
#define GET_BLOCK_FIXED(x) GET_BLOCK(FIXED_GET_INT(x))

/* This structure represents the game board, including its walls, nibblets,
   nibbloons, empty space, and the asylum that the ghosts rest in. */
typedef struct Board {
	/* Array of blocks on the board.  Each element correpsonds to a BLOCK_* constant. */
	int blocks[BOARD_WIDTH][BOARD_HEIGHT];

	/* Number of nibblets/nibloons left.  When this hits 0, the level has been won. */
	int nibs_left;

	/* Whether board is visible or not.  Currently just used for data
	   by pman.c (not used by any of the board functions). */
	int is_visible;

	/* The state of the game board. */
	State state;

	/* The fully-drawn game board surface. */
	SDL_Surface *background;

	/* The bitmap that contains tiles of each wall segment.  Used for creating
	   the bitmap of the board surface. */
	SDL_Surface *walls_bitmap;

	/* Absolute pixel coodinates of the rectangle that the board is drawn on. */
	SDL_Rect draw_rect;

	/* The ghosts on the board. */
	GameAgent ghosts[4];

	/* Pac man on the board. */
	GameAgent pman;

	/* The fruit. */
	GameAgent fruit;

	/* Array that tells ghosts how to get back to the asylum. 
	   Each array index represents a block whose FixedVector indicates
	   the direction to go to get back to the asylum. */
	FixedVector goto_asylum_directions[BOARD_WIDTH][BOARD_HEIGHT];

} Board;

DECLARE_STATE_MACHINE(board_state_machine);

int board_get_block(Board *b, int x, int y);

void board_load_data(Board *b);
void board_generate_background(Board *b);
void board_restart(Board *b, int reload_board_data);
void board_init(Board *b, int x_ofs, int y_ofs);
void board_destroy(Board *b);
void board_draw(Board *b, SDL_Surface *surface, int game_view_flags);
int board_controller(Board *b, SDL_Event *e);
void board_toggle_visible(Board *b);
FixedVector board_get_asylum_directions_at_block(Board *b, int x, int y);

#endif
