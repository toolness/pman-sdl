#include "globals.h"

#include <stdlib.h>
#include <assert.h>

#include "SDL.h"

#include "game.h"
#include "font.h"
#include "state.h"
#include "debug.h"
#include "pman.h"
#include "pman_agent.h"
#include "pman_board.h"
#include "pman_score.h"
#include "pman_agent_ghost.h"
#include "pman_agent_fruit.h"
#include "menu.h"
#include "hiscore.h"
#include "audio.h"

const GameState pman_game_state = { pman_model, pman_view, pman_controller, pman_init, pman_shutdown };

const GameState pman_demo_game_state = { pman_model, pman_view, pman_demo_controller, pman_demo_init, pman_demo_shutdown };

/* Game board.  Contains the board itself, and everything on the board (pac man,
   ghosts, etc). */
static Board g_board;

/* The scoreboard.  Keeps track of the player's score, lives left, etc. */
static Score g_score;

/* The current level the player is on. */
static int g_level;

/* State data for the play state FSM. */
static State g_play_state;

/* Whether or not to show the "READY!" text */
static int g_show_ready_text;

/* Whether or not the game is in demo mode. */
static int g_demo_flag = 0;

void pman_load_sounds()
{
	audio_sample_add("start.wav", SAMPLE_ID_START);
	audio_sample_add("doink.wav", SAMPLE_ID_DOINK);
	audio_sample_add("pman_dead.wav", SAMPLE_ID_PMAN_DEAD);
	audio_sample_add("ghost_killed.wav", SAMPLE_ID_GHOST_KILLED);
	audio_sample_add("fruit.wav", SAMPLE_ID_FRUIT_EATEN);
	audio_sample_add("nibblet2.wav", SAMPLE_ID_NIBBLET_EATEN);
}

void pman_free_sounds()
{
	audio_sample_free(SAMPLE_ID_START);
	audio_sample_free(SAMPLE_ID_DOINK);
	audio_sample_free(SAMPLE_ID_PMAN_DEAD);
	audio_sample_free(SAMPLE_ID_GHOST_KILLED);
	audio_sample_free(SAMPLE_ID_FRUIT_EATEN);
	audio_sample_free(SAMPLE_ID_NIBBLET_EATEN);
}

GameAgent *pman_get_game_agent(int state_id)
{
	/* Get the pac man game agent. */
	return (GameAgent *) (state_get_global_state(state_id)->parent);
}

int pman_in_demo_mode()
{
	return g_demo_flag;
}

void pman_set_show_ready_text(int flag)
{
	g_show_ready_text = flag;
	game_set_draw_flags(GAME_DRAW_FLAG_REDRAW);
}

int pman_get_level()
{
	return g_level;
}

Board *pman_get_board()
{
	return &g_board;
}

void intentional_delay(int time)
{
	Uint32 timer;
	timer = SDL_GetTicks();
	timer += time;
	while (SDL_GetTicks() < timer) { }
}

void play_state_init()
{
	state_construct(&g_play_state, STATE_ID_PLAY_STATE, STATE_ID_PLAY_STATE, NULL, TIMER_ID_GAME);
}

void pman_restart_level()
{
	board_restart(&g_board, 1);
	score_restart(&g_score);
}

void pman_restart_level_continue()
{
	board_restart(&g_board, 0);
	score_restart(&g_score);
}

void pman_register_state_machines()
{
	// play state
	state_set_global_state_machine_id(play_state_machine, STATE_ID_PLAY_STATE);

	// board
	state_set_global_state_machine_id(board_state_machine, STATE_ID_BOARD);

	// pacman
	state_set_global_state_machine_id(agent_pman_state_machine, STATE_ID_AGENT_PMAN);

	// ghost
	state_set_global_state_machine_id(agent_ghost1_state_machine, STATE_ID_AGENT_GHOST_1);

	// fruit
	state_set_global_state_machine_id(agent_fruit_state_machine, STATE_ID_AGENT_FRUIT);
}

void pman_init()
{
	pman_register_state_machines();

	g_level = 0;

	board_init(&g_board, PMAN_BOARD_OFFSET_X, PMAN_BOARD_OFFSET_Y);
	score_init(&g_score, PMAN_SCORE_OFFSET_X, PMAN_SCORE_OFFSET_Y);
	play_state_init();

	pman_load_sounds();
	audio_pause(0);

	state_send_message(STATE_MSG_OnEnter, 0, STATE_ID_PLAY_STATE, 0, 0);
}

void pman_shutdown()
{
	audio_pause(1);
	pman_free_sounds();

	board_destroy(&g_board);
	score_destroy(&g_score);
}

void pman_model(Uint32 frame_time)
{
	state_send_message(STATE_MSG_OnUpdate, 0, STATE_ID_PLAY_STATE, 0, &frame_time);
}

void pman_view(SDL_Surface *surface, int game_view_flags)
{
	board_draw(&g_board, surface, game_view_flags);
	score_draw(&g_score, surface, game_view_flags);
	if (g_show_ready_text) {
		font_draw_string_centered(game_get_font_big(), surface, g_board.draw_rect.x + (g_board.draw_rect.w / 2), g_board.draw_rect.y + (BLOCK(17) + (BLOCK_SIZE/2)), PMAN_READY_TEXT);
	}
}

void pman_demo_init()
{
	g_demo_flag = 1;
	pman_init();
}

void pman_demo_shutdown()
{
	pman_shutdown();
	g_demo_flag = 0;
}

int pman_demo_controller(SDL_Event *e)
{
	/* If the user presses a key, exit demo mode. */
	if (e->type == SDL_KEYDOWN) {
		game_set_state(&menu_game_state);
		return 1;
	}
	return 0;
}

int pman_controller(SDL_Event *e)
{
	return board_controller(&g_board, e);
}

BEGIN_STATE_MACHINE(play_state_machine)
	STATE_MACHINE_HEADER
	ON_ENTER
		SET_STATE(PLAY_STATE_START_LEVEL_ANEW);

	STATE(PLAY_STATE_START_LEVEL_ANEW)
		ON_ENTER
			pman_restart_level();
			SET_STATE(PLAY_STATE_START_LEVEL);

	STATE(PLAY_STATE_START_LEVEL_CONTINUE)
		ON_ENTER
			pman_restart_level_continue();
			SET_STATE(PLAY_STATE_START_LEVEL);

	STATE(PLAY_STATE_START_LEVEL)
		ON_ENTER
			pman_set_show_ready_text(1);
			state_send_message(STATE_MSG_OnEnter, STATE_ID_PLAY_STATE, STATE_ID_BOARD, 0, NULL);
			state_send_message(PLAY_STATE_MSG_LEAVE_START_LEVEL, STATE_ID_PLAY_STATE, STATE_ID_PLAY_STATE, PMAN_READY_TEXT_DELAY, 0);
			audio_sample_play(SAMPLE_ID_START);
		ON_MSG(PLAY_STATE_MSG_LEAVE_START_LEVEL)
			SET_STATE(PLAY_STATE_NORMAL);
		ON_EXIT
			pman_set_show_ready_text(0);
	STATE(PLAY_STATE_NORMAL)
		ON_UPDATE
			// call state machine update messages here, w/ time parameter
			state_send_message(STATE_MSG_OnUpdate, 0, STATE_ID_BOARD, 0, sm->data);
		ON_MSG(PLAY_STATE_MSG_LEVEL_WON)
			SET_STATE(PLAY_STATE_LEVEL_WON);
		ON_MSG(PLAY_STATE_MSG_PMAN_KILLED)
			SET_STATE(PLAY_STATE_PMAN_KILLED);
		ON_MSG(PLAY_STATE_MSG_NIBBLET_EATEN)
			audio_sample_play(SAMPLE_ID_NIBBLET_EATEN);
			score_add(&g_score, SCORE_NIBBLET_SCORE);
		ON_MSG(PLAY_STATE_MSG_NIBBLOON_EATEN)
			int i;

			audio_sample_play(SAMPLE_ID_NIBBLET_EATEN);
		
			for (i = 0; i < 4; i++) {
				state_send_message( GHOST_MSG_START_FLEEING, 0, g_board.ghosts[i].state.state_id, 0, 0 );
			}
			score_add_nibbloon(&g_score);
		ON_MSG(PLAY_STATE_MSG_AGENT_KILLED)
			int *data;
			GameAgent *agent = (GameAgent *) (state_get_global_state(sm->from)->parent);

			if (agent->agent_type == GAME_AGENT_GHOST) {
				audio_sample_play(SAMPLE_ID_GHOST_KILLED);
			} else if (agent->agent_type == GAME_AGENT_FRUIT) {
				audio_sample_play(SAMPLE_ID_FRUIT_EATEN);
			}

			agent->ghost_score_amount = score_add_agent_kill(&g_score, agent);

			data = temp_int_pool_get_int();
			*data = sm->from;

			state_send_message(AGENT_MSG_FREEZE_AND_DIE, 0, sm->from, 0, 0);
			state_send_message(PLAY_STATE_MSG_GO_NORMAL, 0, STATE_ID_PLAY_STATE, AGENT_KILLED_FREEZE_DELAY, data);
			SET_STATE(PLAY_STATE_GHOST_KILLED);
	STATE(PLAY_STATE_GHOST_KILLED)
		ON_MSG(PLAY_STATE_MSG_GO_NORMAL)
			state_send_message(AGENT_MSG_CONTINUE, 0, *(int *) sm->data, 0, 0);
			SET_STATE(PLAY_STATE_NORMAL);
	STATE(PLAY_STATE_PMAN_KILLED)
		ON_ENTER
			state_send_message(PLAY_STATE_MSG_GO_NORMAL, 0, STATE_ID_PLAY_STATE, 1000, 0);
		ON_MSG(PLAY_STATE_MSG_GO_NORMAL)
			audio_sample_play(SAMPLE_ID_PMAN_DEAD);
			state_send_message(PLAY_STATE_MSG_PMAN_REVIVE, 0, STATE_ID_PLAY_STATE, 3000, 0);
		ON_MSG(PLAY_STATE_MSG_PMAN_REVIVE)
			if (score_lives_decrement(&g_score)) {
				SET_STATE(PLAY_STATE_START_LEVEL_CONTINUE);
			} else {
				if (pman_in_demo_mode()) {
					/* If in demo mode, set the test score to -1 so the hiscore module doesn't
					   attempt to set a new hi score, but still displays the hiscore list for
					   some amount of time. */
					hiscore_set_test_score(-1);
				} else {
					hiscore_set_test_score(g_score.score);
				}
				game_set_state(&hiscore_game_state);
			}
	STATE(PLAY_STATE_LEVEL_WON)
		ON_ENTER
			int *data;

			data = temp_int_pool_get_int();
			*data = BOARD_WIN_FLASH_TIMES;
			state_send_message(PLAY_STATE_MSG_BOARD_FLASH, 0, STATE_ID_PLAY_STATE, 0, data);
		ON_MSG(PLAY_STATE_MSG_BOARD_FLASH)
			int *num_times = (int *) sm->data;

			if (*num_times == 0) {
				g_level++;
				SET_STATE(PLAY_STATE_START_LEVEL_ANEW);
			}

			board_toggle_visible(&g_board);

			*num_times = *num_times - 1;

			state_send_message(PLAY_STATE_MSG_BOARD_FLASH, 0, STATE_ID_PLAY_STATE, BOARD_WIN_FLASH_DELAY, num_times);

END_STATE_MACHINE
