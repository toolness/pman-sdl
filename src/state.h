#ifndef INCLUDE_STATE
#define INCLUDE_STATE

/* state.h

   Functions and data for the implementation and coordination of finite state machines.
   Atul Varma - 7/16/2003

   This code is based mostly on concepts covered in the article "Designing a General
   Robust AI Engine" by Steve Rabin, from "Game Programming Gems, Volume I".
   (pp. 221-236)
*/

#include "SDL.h"

/* Maximum number of state machines. A state machine, in this context,
   is a function that performs differently depending on the
   state object passed to it. */
#define MAX_STATE_MACHINES 100

/* Maximum number of state objects.  A state object is a data structure
   that represents the state of an object in the game world, or the
   state of a FSM. */
#define MAX_STATE_OBJECTS  100

/* Maximum number of timers. */
#define MAX_TIMERS   100

/* State ID for a FSM's global state. */
#define STATE_Global 0

/* STATE_MSG_* are pre-defined messages that are sent to states. */

/* Message sent by the state router to a state machine whenever a
   new state is entered. */
#define STATE_MSG_OnEnter  0

/* Message sent by the state router to a state machine whenever a
   state is exited. */
#define STATE_MSG_OnExit   1

/* Message sent to a state machine every "tick" (this message needs to
   be sent manually; it is not automatically sent by anything in the
   base foundation library). */
#define STATE_MSG_OnUpdate 2

/* Declares a state machine function (for use in header files). */
#define DECLARE_STATE_MACHINE(n) int n(State *s, int state, StateMessage *sm)

/* Defines a state machine function and begins its scope (for use in .c files) */
#define BEGIN_STATE_MACHINE(n) DECLARE_STATE_MACHINE(n) {

/* Starts the state machine.  This should be used right after BEGIN_STATE_MACHINE,
   except in the case where function variables are to be used, in which case those
   function variables should be defined and initialized between BEGIN_STATE_MACHINE
   and STATE_MACHINE_HEADER. */
#define STATE_MACHINE_HEADER if (state == STATE_Global) { if (0) {

/* Starts a block in a FSM for the given state.  This should only be used in the
   global state scope. */
#define STATE(x) return 1; } } else if (state == x) { if (0) {

/* Begins the FSM's response to the given message event for the given
   state block. */
#define ON_MSG(x) return 1; } else if (sm->message == x) {

/* Begins the FSM's response to the STATE_MSG_OnEnter event for the given
   state block (i.e., this code is executed when an FSM enters the
   state that this directive is declared in). */
#define ON_ENTER ON_MSG(STATE_MSG_OnEnter)

/* Begins the FSM's response to the STATE_MSG_OnExit event for the given
   state block. */
#define ON_EXIT ON_MSG(STATE_MSG_OnExit)

/* Begins the FSM's response to the STATE_MSG_OnUpdate event for the given
   state block. */
#define ON_UPDATE ON_MSG(STATE_MSG_OnUpdate)

/* When used in an FSM function, this macro sets the state object's state to
   the given state. */
#define SET_STATE(x) state_set_object_state(s, (int) x)

/* Closing declare for a FSM function.  Used to close a FSM function after
   BEGIN_STATE_MACHINE and STATE_MACHINE_HEADER. */
#define END_STATE_MACHINE return 1; } } else \
	assert(!"State message went unhandled!"); return 0; }

/* Structure for a state object that encapsulates the state of a given game object for
   a FSM function to use. */
typedef struct State {
	/* Unique state ID of the state object. */
	int state_id;

	/* Timer ID for this state.  Affects the timing of
	   how delayed messages are sent to the state. */
	int timer_id;

	/* State machine (FSM function) id for the given object.  This effectively
	   determines the state object's "type" or "class". */
	int state_machine_id;

	/* The current state that this state object is in. */
	int state;

	/* Used internally by the message router.  See change_state comments for
	   more information. */
	int next_state;

	/* Used internally by the message router.  When it is 1, the state needs to
	   be changed to next_state and the appropriate OnEnter and OnExit messages
	   need to be called. */
	int change_state;

	/* Pointer to the parent object (if any) that "owns" this state object.  This is
	   always the structure that is compositing/aggregating the state object.  e.g.,
	   if this State object describes a ghost's state, it will be a pointer to a
	   GameAgent structure. */
	void *parent;
} State;

/* State message object.  When a message is sent to a state object/FSM, the details
   of the message are encapsulated in this structure. */
typedef struct StateMessage {
	/* The message ID of the message (e.g., STATE_MSG_OnEnter). */
	int message;

	/* The state ID of the sender and recipient of the state message, respectively. */
	int from, to;

	/* The time at which the message should be delivered. */
	Uint32 delivery_time;

	/* Pointer to any extra parameter data. */
	void *data;
} StateMessage;

/* Node in a state message queue.  A state message queue is a queue (well, technically
   just a linked list) that stores the list of state messages whose delivery times have
   not been reached yet.  This structure is used internally by the message router. */
typedef struct StateMessageQueue {
	StateMessage *sm;
	struct StateMessageQueue *next;
} StateMessageQueue;

typedef int (*StateMachine)(State *, int, StateMessage *);

void state_init();
void state_shutdown();

void state_set_global_state_id(State *s, int state_id);
State *state_get_global_state(int state_id);
void state_set_global_state_machine_id(StateMachine smach, int state_machine_id);
void state_set_object_state(State *s, int new_state);
void state_construct(State *s, int new_state_id, int new_state_machine_id, void *new_parent, int timer_id);

void state_route_message(StateMessage *sm);
void state_send_message(int message, int from, int to, int delivery_time, void *data);
void state_set_object_state(State *s, int new_state);
void state_process_messages();

int *temp_int_pool_get_int();

void state_timer_update(int timer_id, Uint32 ticks);
Uint32 state_timer_get_ticks(int timer_id);

#endif
