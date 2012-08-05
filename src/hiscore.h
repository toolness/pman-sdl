#ifndef INCLUDE_HISCORE
#define INCLUDE_HISCORE

/* hiscore.h

   Functions and data for the game's hi score list.
   Atul Varma - 7/30/2003

*/

#include "game.h"

#define HISCORE_LIST_SPACING 22
#define HISCORE_START_Y 100
#define HISCORE_LIST_X (SCREEN_WIDTH / 2 - 80)

/* Maximum length of a player's name in the hiscore list.  This is
   actually 1 more than the desired maximum number of characters; 1 is
   reserved for the
   underscore (_) displayed in the case of a new hi score entry. */
#define HISCORE_NAME_MAXLENGTH 10

/* Total number of entries in the hiscore list */
#define HISCORE_NUM_ENTRIES 10

/* Number of milliseconds to display the hi score list for, if it's in
   "countdown timer" mode. */
#define HISCORE_COUNTDOWN_DELAY 3000

/* Countdown delay after the player entered a new hi score. */
#define HISCORE_COUNTDOWN_DELAY_ENTRY_FINISHED 1000

/* Filename of the hiscore file. */
#define HISCORE_FILENAME "hiscore.dat"

extern const GameState hiscore_game_state;

/* A hi score entry. */
typedef struct HiScoreEntry
{
	/* The name of the hi scorer.  1 is added to this to make space for
	   the null terminator. */
	char name[HISCORE_NAME_MAXLENGTH+1];
	/* The hi scorer's score. */
	int score;
} HiScoreEntry;

/* A list of hi score entries. */
typedef struct HiScoreList
{
	/* The hi score entries. */
	HiScoreEntry entries[HISCORE_NUM_ENTRIES];
} HiScoreList;

void hiscore_init();
void hiscore_shutdown();
void hiscore_view(SDL_Surface *surface, int game_view_flags);
void hiscore_model(Uint32 frame_time);
int hiscore_controller(SDL_Event *e);
void hiscore_set_test_score(int test_score);

#endif
