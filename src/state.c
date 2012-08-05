#include "globals.h"

#include <stdlib.h>
#include <assert.h>

#include "state.h"
#include "game.h"
#include "debug.h"

/* All the state timers, which keep track of time for different
   types of finite state machines. */
static Uint32 g_state_timers[MAX_TIMERS];

/* All the state objects which hold the state data for their
   finite state machines. */
static State *g_state_objects[MAX_STATE_OBJECTS];

/* All the finite state machine functions. */
static StateMachine g_state_machines[MAX_STATE_MACHINES];

/* The message queue used by the state message routing system
   when a message needs to be delivered some amount of time
   in the future. */
static StateMessageQueue *g_state_message_queue;

/* Initializes all the state timers by setting their tick
   values to 0. */
void state_timer_init()
{
	int i;

	for (i = 0; i < MAX_TIMERS; i++) {
		g_state_timers[i] = 0;
	}
}

/* Updates the given timer, adding the given number of ticks
   to it. */
void state_timer_update(int timer_id, Uint32 ticks)
{
	g_state_timers[timer_id] += ticks;
}

/* Gets the current number of ticks from the timer with the
   given timer id. */
Uint32 state_timer_get_ticks(int timer_id)
{
	return g_state_timers[timer_id];
}

/* Temporary integer pool to use for data in state messages, when we need an int to
   store and pass on information independent of the state of the call stack for a
   temporary period of time, without having to worry about dynamic allocation and
   deallocation of memory.
   
   This pool can only give out TEMP_INT_POOL_SIZE ints at the same time; if we're
   using more than that amount at any given time, ints get "overwritten" and
   bad things can happen (there is no error checking for this).  However, if use
   is well below TEMP_INT_POOL_SIZE then there shouldn't be any problems. */

/* Number of temp ints that can be active at the same time. */
#define TEMP_INT_POOL_SIZE 5000

/* This struct represents the temp int pool and is just a circular array with
   an index. */
static struct {
	int ints[TEMP_INT_POOL_SIZE];
	int index;
} g_temp_int_pool;

/* Returns a pointer to a new integer from the temp int pool. */
int *temp_int_pool_get_int()
{
	g_temp_int_pool.index++;
	if (g_temp_int_pool.index >= TEMP_INT_POOL_SIZE) {
		g_temp_int_pool.index = 0;
	}
	return &g_temp_int_pool.ints[g_temp_int_pool.index];
}

/* Resets the temp int pool.  Should only be called once per session, or when
   we know that nobody's using the temp int pool. */
void temp_int_pool_reset()
{
	g_temp_int_pool.index = 0;
}

// private functions
StateMessageQueue *smqueue_create(StateMessage *sm, StateMessageQueue *next);
void smqueue_insert(StateMessageQueue *smqueue, StateMessage *sm);
void smqueue_remove(StateMessageQueue *smqueue);
int smqueue_is_empty(StateMessageQueue *smqueue);
void smqueue_destroy(StateMessageQueue *smqueue);

/* Note that the only reason state_construct() isn't called state_init() is because
   state_init() currently initializes the state module.  This is inconsistent, and
   this notational difference is also present in some other modules. */
void state_construct(State *s, int new_state_id, int new_state_machine_id, void *new_parent, int timer_id)
{
	s->timer_id = timer_id;
	s->change_state = 0;
	s->next_state = 0;
	s->parent = new_parent;
	s->state = 0;
	s->state_id = new_state_id;
	s->state_machine_id = new_state_machine_id;

	state_set_global_state_id(s, new_state_id);
}

/* Initializes the state module. */
void state_init()
{
	int i;

	for (i = 0; i < MAX_STATE_OBJECTS; i++) {
		g_state_objects[i] = NULL;
	}

	for (i = 0; i < MAX_STATE_MACHINES; i++) {
		g_state_machines[i] = NULL;
	}

	g_state_message_queue = smqueue_create(NULL, NULL);

	temp_int_pool_reset();
	state_timer_init();
}

/* Shuts down the state module. */
void state_shutdown()
{
	smqueue_destroy(g_state_message_queue);
	g_state_message_queue = NULL;
}

/* Assigns the given state ID to the given State object. */
void state_set_global_state_id(State *s, int state_id)
{
	assert(state_id < MAX_STATE_OBJECTS);
	//assert(g_state_objects[state_id] == NULL);

	s->state_id = state_id;
	g_state_objects[state_id] = s;
}

/* Returns a pointer to the State object with the given state ID. */
State *state_get_global_state(int state_id)
{
	assert(state_id >= 0);
	assert(state_id < MAX_STATE_OBJECTS);

	return g_state_objects[state_id];
}

/* Sets the given FSM function to the given state machine ID. */
void state_set_global_state_machine_id(StateMachine smach, int state_machine_id)
{
	assert(state_machine_id < MAX_STATE_MACHINES);
	assert(g_state_machines[state_machine_id] == NULL);

	g_state_machines[state_machine_id] = smach;
}

/* Changes the state of the given State object to the new state.
   (Actually, this really just sets data that tells the state
   message router to change the state object's state; the router
   is the one that does the actual changing, and this function
   shouldn't be called outside of a FSM function. */
void state_set_object_state(State *s, int new_state)
{
	s->change_state = 1;
	s->next_state = new_state;
}

/* This is the main function for sending a message from one FSM to another.

   message - an integer representing the type of message.  See the *_MSG_*
     constants for any FSM.

   from, to - the state ID's of the sender and recipient of the message,
     respectively.

   delivery_time - number of milliseconds/ticks in the future at which the
     message should be sent (relative to the recipient's timer).

   data - pointer to any extra data to be passed with the message.
*/
void state_send_message(int message, int from, int to, int delivery_time, void *data)
{
	StateMessage *sm;
	State *s_to;

	/* Allocate the message object and fill in the appropriate fields. */
	assert(from < MAX_STATE_OBJECTS);
	assert(to < MAX_STATE_OBJECTS);
	assert(delivery_time >= 0);

	sm = malloc(sizeof(StateMessage));

	sm->to = to;

	s_to = state_get_global_state(sm->to);

	assert(s_to != NULL);

	sm->message = message;
	sm->from = from;
	sm->data = data;
	/* For delivery time, take the current time according to the recipient's
	   timer, and add the delivery time parameter to it, thus converting
	   a "relative" time measurement to an absolute one. */
	sm->delivery_time = state_timer_get_ticks(s_to->timer_id) + delivery_time;

	/* Now send off the message to the message router. */
	state_route_message(sm);
}

/* Routes the given state message.  If the recipient doesn't exist, the
   message is thrown away; if the message can be delivered now, it is
   sent now; otherwise, the message is queued for later delivery. */
void state_route_message(StateMessage *sm)
{
	State *s_to;

	assert(sm->from < MAX_STATE_OBJECTS);
	assert(sm->to < MAX_STATE_OBJECTS);

	if ( state_get_global_state(sm->to) == NULL ) {
		/* The object we wanted to send to no longer exists.  Oh well. */
		free(sm);
		//err("  message discarded\n", 0);
		return;
	}

	s_to = state_get_global_state(sm->to);

	if ( sm->delivery_time <= state_timer_get_ticks(s_to->timer_id) ) {
		/* Deliver the message now. */
		int smid;
		State *gobj;
		StateMachine smach;

		//err("  delivering message\n", 0);

		gobj = state_get_global_state(sm->to);
		smid = gobj->state_machine_id;
		smach = g_state_machines[smid];

		assert(smid < MAX_STATE_MACHINES && smach != NULL);

		if (!smach(gobj, gobj->state, sm))
			smach(gobj, 0, sm);

		while (gobj->change_state) {
			StateMessage temp_sm;

			gobj->change_state = 0;

			temp_sm.from = gobj->state_id;
			temp_sm.to   = gobj->state_id;
			temp_sm.message = STATE_MSG_OnExit;
			temp_sm.data = NULL;
			temp_sm.delivery_time = 0;

			smach(gobj, gobj->state, &temp_sm);

			gobj->state = gobj->next_state;

			temp_sm.message = STATE_MSG_OnEnter;
			smach(gobj, gobj->state, &temp_sm);
		}

		free(sm);
	} else {
		/* Queue the message for delivery later. */
		//err("  queueing message\n", 0);
		assert(g_state_message_queue != NULL);
		smqueue_insert(g_state_message_queue, sm);
		return;
	}
}

/* Creates a state message queue object.  Note that a StateMessageQueue
   object is also a node in a state message queue, so this function can
   be used for creating a new smqueue node as well. */
StateMessageQueue *smqueue_create(StateMessage *sm, StateMessageQueue *next)
{
	StateMessageQueue *smqueue;

	assert( ( sm == NULL && next == NULL) || ( sm != NULL && next != NULL ) );

	smqueue = (StateMessageQueue *) malloc(sizeof(StateMessageQueue));
	smqueue->sm = sm;
	smqueue->next = next;
	return smqueue;
}

/* Inserts the given state message in the front of the state message queue. */
void smqueue_insert(StateMessageQueue *smqueue, StateMessage *sm)
{
	StateMessageQueue *new_smqueue;

	assert(smqueue != NULL);
	assert(sm != NULL);

	new_smqueue = smqueue_create(smqueue->sm, smqueue->next);
	smqueue->sm = sm;
	smqueue->next = new_smqueue;
}

/* Removes the first element from the state message queue. */
void smqueue_remove(StateMessageQueue *smqueue)
{
	if (smqueue->sm == NULL) {
		assert(smqueue_is_empty(smqueue));
		return;
	} else {
		StateMessageQueue *old_next_smqueue;

		/* Don't bother freeing the StateMessage, it should have already been done
		   by state_route_message(). */

		assert (smqueue->next != NULL);

		old_next_smqueue = smqueue->next;
		smqueue->sm = old_next_smqueue->sm;
		smqueue->next = old_next_smqueue->next;
		free(old_next_smqueue);
	}
}

/* Returns true if the state message queue is empty. */
int smqueue_is_empty(StateMessageQueue *smqueue)
{
	if ( smqueue->next == NULL ) {
		assert(smqueue->sm == NULL);
		return 1;
	} else {
		return 0;
	}
}

/* Destroy the given state message queue by removing all its nodes. */
void smqueue_destroy(StateMessageQueue *smqueue)
{
	while ( !smqueue_is_empty(smqueue) ) {
		/* Since the message hasn't been processed by state_route_message(),
		   we'll have to destroy it manually. */
		free(smqueue->sm);
		smqueue_remove(smqueue);
	}
	free(smqueue);
}

/* Process the state message queue by iterating through all its state
   messages and executing any messages whose delivery time has been
   reached. */
void smqueue_process(StateMessageQueue *smqueue)
{
	StateMessageQueue *cursor;

	cursor = smqueue;

	while ( !smqueue_is_empty(cursor) ) {
		State *s_to;

		s_to = state_get_global_state(cursor->sm->to);
		if (s_to) {
			if (cursor->sm->delivery_time <=
				state_timer_get_ticks(s_to->timer_id) ) {

				StateMessage *sm;

				sm = cursor->sm;
				smqueue_remove(cursor);
				state_route_message(sm);
			} else {
				cursor = cursor->next;
			}
		} else {
			smqueue_remove(cursor);
		}
	}
}

/* Process the messages in the global state message queue. */
void state_process_messages()
{
	smqueue_process(g_state_message_queue);
}
