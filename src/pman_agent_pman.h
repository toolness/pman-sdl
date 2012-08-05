#ifndef INCLUDE_PMAN_AGENT_PMAN
#define INCLUDE_PMAN_AGENT_PMAN

/* pman_agent_pman.h

   Pac Man game agent routines and FSM.
   Atul Varma - 7/28/2003

*/

#include "SDL.h"

#include "fixed.h"
#include "pman_agent.h"

#define PMAN_STATE_MOVING  0
#define PMAN_STATE_DYING   1

/* Frames per "direction of movement" animation for pac man. */
#define PMAN_FRAMES_PER_DIRECTION  10

/* Base speed for pacman (current level increases this). Measured in
   pixels per decisecond (i.e. pixels per tenth of a second). */
#define PMAN_BASE_SPEED 10.0

/* Speed of pacman animation, in frames per decisecond. */
#define PMAN_FRAME_SPEED 4.0

/* Starting coords for Pman, in block coordinates. */
#define PMAN_START_BLOCK_X 13
#define PMAN_START_BLOCK_Y 23

void agent_pman_generate_frames(GameAgent *ga);
void agent_pman_restart(GameAgent *ga);
void agent_pman_init(GameAgent *ga);
void agent_pman_destroy(GameAgent *ga);
void agent_pman_draw(GameAgent *ga, SDL_Surface *surface, int x_ofs, int y_ofs);

void pman_draw_wedge(SDL_Surface *screen, int x1, int y1, int r, int mouth_open, int mouth_inset, int segment);

void pman_draw(SDL_Surface *screen, int x1, int y1, int r, int mouth_open, int mouth_inset, int direction, Uint32 color);

#endif
