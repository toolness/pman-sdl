#ifndef INCLUDE_PMAN_AGENT
#define INCLUDE_PMAN_AGENT

/* pman_agent.h

   Data and functions for the operation of generic game agents and the pac man game agent.
*/

#include "SDL.h"

#include "fixed.h"

// Game agent types (not to be confused w/ game agent state ID's, which track instances)
#define GAME_AGENT_PMAN    1
#define GAME_AGENT_GHOST   2
#define GAME_AGENT_FRUIT   3

/* Message sent whenever an agent has moved entirely onto a new block. */
#define GAME_AGENT_MSG_BLOCK_CHANGE 10

/* Sent by the game board to an agent when pman has collided with it. */
#define AGENT_MSG_HIT_PMAN 500

/* Delayed message sent by states that do nothing but pause until some time has passed. */
#define AGENT_MSG_CONTINUE 503

/* Sent to the agent by the gameplay state when the agent has been hit by pman and needs to
   freeze and die.  The play state needs to send this event to the agent b/c if pman hits two
   agents at the same time, they must die serially (since the game has to "freeze" for a
   second after each kill) and not in "parallel".  In reality, the play state ignores one
   of the PLAY_STATE_MSG_AGENT_KILLED messages sent to it in the case of a "multiple
   kill", and the ignored agent will send the play state another PLAY_STATE_MSG_AGENT_KILLED
   message in the next "unfrozen" frame since there is still a sprite collision. */
#define AGENT_MSG_FREEZE_AND_DIE 504

/* # of ms to pause when a game agent has been killed. */
#define AGENT_KILLED_FREEZE_DELAY 1000

/* Converts a speed value in pixels-per-decisecond to pixels-per-millisecond. */
#define CONVERT_PPDS_TO_PPMS(x) ( (double) (x) / 100 )

/* Game agent structure.  This stores all information for a game agent, which
   generally means either pac man or a ghost. */
typedef struct GameAgent {
	/* The type ID of the agent.  Refers to one of the GAME_AGENT_* constants. */
	int agent_type;
	/* The state of the game agent. */
	State state;
	/* The on-screen graphical height and width of the game agent
	   (i.e., the sprite dimensions).*/
	FixedVector graphical_dim;
	/* The offset of the on-screen representation of the game agent
	   from its physical location (i.e., GameAgent.loc) */
	FixedVector graphical_offset;
	/* The physical height and width of the game agent, used for
	   collision detection. */
	FixedVector physical_dim;
	/* The physical location of the game agent. */
	FixedVector loc;
	/* The location of the game agent in the last frame.  This is used for
	   "dirty rectangle" animation so the game can just erase where the game agent
	   used to be instead of having to redraw the entire screen. */
	FixedVector last_loc;
	/* The current move (direction of movement) of the game agent.  This is
	   a fixed_vector_* constant, as defined in fixed.h. */
	FixedVector curr_move;
	/* The next queued move of the game agent.  Currently this is only used by
	   pman, although maybe it might be used by some ghost AI. */
	FixedVector next_move;
	/* The speed of the game agent. */
	fixed speed;
	/* The color of the game agent. */
	Uint32 color;
	/* Whether the agent is visible.  Only really used by ghosts, but could
	   be used by pman. */
	int is_visible;
	/* The original color of the game agent.  Used for color switching during ghost fleeing. */
	Uint32 original_color;
	/* The original speed of the game agent.  Used for speed switching during ghost fleeing. */
	fixed original_speed;
	/* Number of times left the ghost has to flash. */
	int ghost_flee_flash_times;
	/* Number of times the ghost has fled so far.  This is used as a "unique id" to
	   fix cases of "lagging state messages" when the ghosts are already fleeing and
	   pman eats another nibbloon. */
	int ghost_flee_times;
	/* Whether the ghost can open the asylum door.  Not used for pman. */
	int can_open_asylum_door;
	/* # of times ghost hits asylum walls while resting before he decides to leave. 
	   This is a countdown timer. */
	int ghost_resting_hit_times;
	/* If the ghost just got killed, this is how many points the player got for it.
	   If this is nonzero, it means the ghost should display the points instead of its
	   ghost-form. */
	int ghost_score_amount;
	/* Tiled bitmap surface representing the sprite frames of game agent.  Used by
	   pac man game agent only. */
	SDL_Surface *frames;
	/* Current frame.  Used by pacman only. */
	fixed frame_curr;
	/* Animation speed ("frame velocity").  Used by pacman only. */
	fixed frame_speed;
	/* Flag that tells us whether pman is in "ai" mode (i.e., whether he should be
	   controlled by the CPU or player).  AI mode generally means that the game is
	   in demo mode, although it could be used in the future to dynamically change
	   what controls pman. */
	int pman_ai_flag;
	/* Instance ID of the current fruit being displayed. */
	int fruit_id;
} GameAgent;

int agent_is_position_viable(GameAgent *ga, FixedVector *v);
int agent_move(GameAgent *ga, Uint32 time);

int agent_next_move(GameAgent *ga);

void agent_set_move(GameAgent *ga, const FixedVector *v);
void agent_set_next_move(GameAgent *ga, const FixedVector *v);

void agent_draw(GameAgent *ga, SDL_Surface *surface, int x_ofs, int y_ofs);
void agent_replace_background(GameAgent *ga, SDL_Surface *surface, SDL_Surface *background, int x_ofs, int y_ofs);

void agent_toggle_visible(GameAgent *ga);

void agent_get_draw_bounding_rect(GameAgent *ga, SDL_Rect *r, int x_ofs, int y_ofs);

int agent_in_tunnel(GameAgent *ga);
void agent_determine_next_random_move(GameAgent *ga);
int agent_can_see_agent(GameAgent *ga, GameAgent *pman, FixedVector *direction, int max_dist);
void agent_draw_score_amount(GameAgent *ga, SDL_Surface *surface, SDL_Rect *r);

DECLARE_STATE_MACHINE(agent_pman_state_machine);

#endif
