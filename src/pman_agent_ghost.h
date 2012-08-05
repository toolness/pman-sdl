#ifndef INCLUDE_PMAN_AGENT_GHOST
#define INCLUDE_PMAN_AGENT_GHOST

/* pman_agent_ghost.h

   Data and functions for the operation of ghost game agents.
*/

#include "SDL.h"

#include "fixed.h"
#include "pman_agent.h"

/* Total number of colors for ghost. */
#define GHOST_MAX_COLORS            50
/* Color when ghost is scared.  This was actually a color that is now a
   very non-granular "state"; if the ghost's color is set to this, their
   "scared" sprite should be drawn.  The color itself is still used for
   the ghost's body when scared, though. */
#define GHOST_COLOR_SCARED          0
/* Color when ghost is spirited.  This was actually a color that is now a
   very non-granular "state"; if the ghost's color is set to this, their
   "spirit" sprite should be drawn.  The color itself is not used. */
#define GHOST_COLOR_SPIRIT          1
/* Color of the whites' of the ghost's eyes. */
#define GHOST_COLOR_EYES            2
/* Color of the ghost's pupils. */
#define GHOST_COLOR_PUPILS          3
/* Color of the ghost's features (eyes and mouth) when scared. */
#define GHOST_COLOR_SCARED_FEATURES 4

/* Ghost is running away from pman b/c he ate a nibbloon. */
#define GHOST_STATE_FLEEING 1

/* Ghost has been eaten by pman and is going back to the asylum. */
#define GHOST_STATE_SPIRIT  2

/* Ghost is resting/idling in the asylum at the beginning of a level. */
#define GHOST_STATE_RESTING 3

/* Ghost is actively searching for pman. */
#define GHOST_STATE_SEEKING 4

/* Go to the top center of the asylum so as to leave it
   (called when a ghost has stopped resting and is leaving the asylum near the beginning of a level). */
#define GHOST_STATE_GOTO_ASYLUM_EXIT 5

/* Go to the top center above the asylum so as to enter it.
   (called when a ghost is in spirit mode and wants to enter the asylum so it can
   respawn.) */
#define GHOST_STATE_GOTO_ASYLUM_ENTRANCE 6

/* Sent when the ghost is at the center of the asylum and is ready to leave it. */
#define GHOST_STATE_LEAVE_ASYLUM 7

/* Enter the asylum to the south. */
#define GHOST_STATE_ENTER_ASYLUM 8

/* Respawn the ghost once it's in the center of the asylum. */
#define GHOST_STATE_RESPAWN 9

/* Ghost is freezing for a sec to display # of points it died for before it goes into
   spirit mode and runs back to asylum. */
#define GHOST_STATE_FREEZE_KILLED 10

/* Message sent when the ghost is about to go from "fleeing" to "seeking" and starts
   flashing to indicate such. */
#define GHOST_MSG_FLEE_FLASH 501

/* Message sent to the ghost when pman has eaten a nibbloon and the ghost should start
   fleeing. */
#define GHOST_MSG_START_FLEEING 502

/* Number of ms after the ghost starts fleeing that it should start flashing. */
#define GHOST_FLEE_INITIAL_TIME 10000
/* Amount of time to subtract from GHOST_FLEE_INITIAL_TIME per level. */
#define GHOST_FLEE_LESS_TIME_PER_LEVEL 1500
/* Number of times the ghost should flash when it's close to stopping fleeing. */
#define GHOST_FLEE_FLASH_TIMES 9
/* Delay in ms between each flash. */
#define GHOST_FLEE_FLASH_DELAY 500
/* % of normal speed that the ghost should move at when it's fleeing. */
#define GHOST_FLEE_SPEED_MULTIPLIER 0.6

/* Amount of speed to add to ghosts per level, measured in pixels
   per decisecond. */
#define GHOST_ADDED_SPEED_PER_LEVEL 0.35

/* The GHOST_SPRITE_* constants are for drawing of the ghost sprites. */

/* How far away the pupils of the ghost eyes are from the sclera (whites of their eyes)
   when the ghosts are moving in a particular direction. */
#define GHOST_SPRITE_EYEBALL_OFFSET 1
/* Width of triangles at the bottom of the ghost body. */
#define GHOST_SPRITE_TRIANGLE_WIDTH 7
/* Height of triangles at the bottom of the ghost body. */
#define GHOST_SPRITE_TRIANGLE_HEIGHT 3

void agent_ghost_restart(GameAgent *ga, int block_x, int block_y, int state_id, int state_machine_id, int initial_state, int resting_hit_times);
void agent_ghost_init(GameAgent *ga, Uint32 color);
void agent_ghost_draw(GameAgent *ga, SDL_Surface *surface, int x_ofs, int y_ofs);

DECLARE_STATE_MACHINE(agent_ghost1_state_machine);

#endif
