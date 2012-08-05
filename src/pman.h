#ifndef INCLUDE_PMAN
#define INCLUDE_PMAN

#include "SDL.h"

#include "game.h"
#include "pman_board.h"

/* Timer for game agents. */
#define TIMER_ID_GAME_AGENT 1

/* Normal mode of gameplay (the player moves pman around to collect nibblets and avoid/eat
   ghosts, etc...). */
#define PLAY_STATE_NORMAL    1
/* The level has been won and should blink a few times. */
#define PLAY_STATE_LEVEL_WON 2
/* A level (continued or brand new) has just started and the game should be displaying the
   "READY!" message to prep the player for gameplay.  This is reached after
   PLAY_STATE_START_LEVEL_ANEW or PLAY_STATE_START_LEVEL_CONTINUE. */
#define PLAY_STATE_START_LEVEL 4
/* A brand new level is about to start (meaning all nibs on the board should be reset). */
#define PLAY_STATE_START_LEVEL_ANEW 5
/* Pman just died, and is continuing the level now (meaning all the nibs on the board should
   stay put). */
#define PLAY_STATE_START_LEVEL_CONTINUE 6
/* Pman just died and should be dying an animated death. */
#define PLAY_STATE_PMAN_KILLED 7
/* A ghost just got killed and the game pauses for a bit to show how many points
   the player got. */
#define PLAY_STATE_GHOST_KILLED 8

/* Sent by the pman_shutdown() function when the uber-parent in game.c tells
   the game that it needs to shut itself down. */
#define PLAY_STATE_MSG_KILL 10
/* Sent by the game board when pman has eaten the last nibblet on the board. */
#define PLAY_STATE_MSG_LEVEL_WON 11
/* Sent when the board needs to flash on and off, after the level has been won. */
#define PLAY_STATE_MSG_BOARD_FLASH 12

/* Sent as a delayed message by the play state itself when the ready text should stop
   displaying and the gameplay should begin. */
#define PLAY_STATE_MSG_LEAVE_START_LEVEL 13
/* Sent by a ghost when it is seeking pacman and collides into him. */
#define PLAY_STATE_MSG_PMAN_KILLED 14
/* Sent by the play state itself as a delayed message when pman is killed and there needs
   to be a delay between his death and the time when pman is "revived" and
   the board is restarted. */
#define PLAY_STATE_MSG_PMAN_REVIVE 15
/* Sent by the game board to the play state when a nibloon is eaten. */
#define PLAY_STATE_MSG_NIBBLOON_EATEN 16
/* Sent by the game board to the play state when a nibblet is eaten. */
#define PLAY_STATE_MSG_NIBBLET_EATEN 17
/* Sent by a ghost to the play state when an agent other than pman is killed. */
#define PLAY_STATE_MSG_AGENT_KILLED 18
/* Told by a state in PLAY_STATE to tell the play state to go to the normal state
   (always used with a delay). */
#define PLAY_STATE_MSG_GO_NORMAL 19

#define SAMPLE_ID_START 0
#define SAMPLE_ID_DOINK 1
#define SAMPLE_ID_PMAN_DEAD 2
#define SAMPLE_ID_GHOST_KILLED 3
#define SAMPLE_ID_FRUIT_EATEN 4
#define SAMPLE_ID_NIBBLET_EATEN 5

/* The STATE_ID_* constants are global ID's for each instance of any game token that has a
   FSM associated with it.  Functions in state.c like state_get_global_object() can be used
   to retrieve the state objects themselves, as well as their parents. */

/* Pac man (the player). */
#define STATE_ID_AGENT_PMAN      1
/* The 4 ghosts. */
#define STATE_ID_AGENT_GHOST_1   2
#define STATE_ID_AGENT_GHOST_2   3
#define STATE_ID_AGENT_GHOST_3   4
#define STATE_ID_AGENT_GHOST_4   5
/* The game board. */
#define STATE_ID_BOARD           6
/* The play state (coordinates between all objects, sort of like a parent for them all). */
#define STATE_ID_PLAY_STATE      7
/* The fruit. */
#define STATE_ID_AGENT_FRUIT     8

/* The absolute x,y offset of the top-left corner of the game board, in pixels. */
#define PMAN_BOARD_OFFSET_X ( (SCREEN_WIDTH/2) - (BOARD_PIXEL_WIDTH/2) )
#define PMAN_BOARD_OFFSET_Y 10

/* The absolute x,y offset of the top-left corner of the scoreboard, in pixels. */
#define PMAN_SCORE_OFFSET_X PMAN_BOARD_OFFSET_X
#define PMAN_SCORE_OFFSET_Y PMAN_BOARD_OFFSET_Y + BOARD_PIXEL_HEIGHT + 5

/* Text to be displayed at the beginning/continuing of a level. */
#define PMAN_READY_TEXT "READY!"
/* Number of ms that the ready text should display for at the beginning/continuing of the level. */
#define PMAN_READY_TEXT_DELAY 3000

void pman_model(Uint32 frame_time);
void pman_view(SDL_Surface *surface, int game_view_flags);
int pman_controller(SDL_Event *e);
void pman_init();
void pman_shutdown();
void pman_demo_init();
void pman_demo_shutdown();
int pman_demo_controller(SDL_Event *e);
Board *pman_get_board();
int pman_get_level();
int pman_in_demo_mode();
GameAgent *pman_get_game_agent(int state_id);

const extern GameState pman_game_state;
const extern GameState pman_demo_game_state;

DECLARE_STATE_MACHINE(play_state_machine);

#endif
