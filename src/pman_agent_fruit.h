#ifndef INCLUDE_FRUIT
#define INCLUDE_FRUIT

/* pman_agent_fruit.h

   Data and functions for the operation of fruit bonuses.
   Atul Varma - 8/1/2003
*/

/* Filename that stores tiled fruit images. (each tile is a different fruit.) */
#define FRUIT_BMP "fruit.bmp"

/* The top-left corner coordinates of the fruit, in pixels. */
#define FRUIT_PIXEL_X BLOCK_ASYLUM_CENTER_PIXEL_X
#define FRUIT_PIXEL_Y (17 * BLOCK_SIZE)

/* Sent when the fruit should toggle its visibility on/off. */
#define FRUIT_MSG_DISPLAY_TOGGLE   1500

/* The total number of fruits in the tiled fruit image file. */
#define FRUIT_NUM_FRUITS 7

/* The *_BASE_TIME and *_RAND_TIME constants determine how much
   time (in ms) must pass before a fruit appears or reappears after a
   certain event.  The time is calculated such that if the
   base time is 10 seconds and rand time is 5 seconds, then the
   amount of time that will need to pass for the fruit to toggle
   its visibility will be 10-15 seconds. */

/* Time to appear on the board after the level starts/continues. */
#define FRUIT_INITIAL_BASE_TIME  10000
#define FRUIT_INITIAL_RAND_TIME  5000

/* Time to reappear on the board after the fruit is eaten. */
#define FRUIT_EATEN_BASE_TIME    30000
#define FRUIT_EATEN_RAND_TIME    30000

/* Time to reappear on the board after the fruit has
   disappeared. */
#define FRUIT_DISAPPEARED_BASE_TIME    20000
#define FRUIT_DISAPPEARED_RAND_TIME    20000

/* Time to disappear on the board after the fruit
   has reappeared. */
#define FRUIT_APPEARED_BASE_TIME    20000
#define FRUIT_APPEARED_RAND_TIME    0

void agent_fruit_restart(GameAgent *ga);
void agent_fruit_init(GameAgent *ga);
void agent_fruit_draw(GameAgent *ga, SDL_Surface *surface, int x_ofs, int y_ofs);
void agent_fruit_destroy(GameAgent *ga);

DECLARE_STATE_MACHINE(agent_fruit_state_machine);

#endif
