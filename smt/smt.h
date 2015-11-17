#ifndef SM_H
#define SM_H

#include <stdlib.h>
#include "smt_defs.h"

struct smt_stateMachine;

/* ------------------------------------------------------------------------------
 * StateMachine Tools Header File
 * ------------------------------------------------------------------------------
 * Limitation
 * - Concurrent Sub state not supported.
 * - History and Deep History are not supported.
 * - If the same event triggered on both compound state and sub state, then only
 *   compound state event is handled, the sub state event is ignored.
 * - Unable to check malformed statemachine such as un-ended transition route.
 * - Maximum compound state hierarchy is 4.
 * - Transition cannot across state hierarchy.
 * -----------------------------------------------------------------------------*/

/* the event ID type */
typedef UINT8 smt_eventId_t;

/* the maximum hierarchy of compound state */
#define SMT_MAX_HIERARCHY 5

#define SMT_FINAL_STATE_ID 255

/* ----- define pseudo events ----- */

/*
 * completionEvent triggers when transition within a composite
 * state comes to endState
 */
#define SMT_COMPLETION_EVENT	((smt_eventId_t)0XFF)

/* ----- eof define pseudo events ----- */

/* the guard function pointer */
typedef boolean (* smt_guardFuncPtr_t)(/*@null@*/ /*@unused@*/ void* context);

/* action return status */
typedef enum
{
	/* Note 1
	 * both SMT_ACTION_ERROR and SMT_ACTION_FAIL will cause state
	 * transition failed (stay at the current state), however, 
	 * stateMachine return SMT_MACHINE_ERROR_ACTION_ERROR 
	 * if SMT_ACTION_ERROR returned, while SMT_MACHINE_OK is returned
	 * if SMT_ACTION_FAIL returned by action function
	 *
	 * Note 2
	 * If exit action and event triggered action return FAIL or ERROR
	 * the transition cannot be completed and cause Machine stay at
	 * the current state. If entry action return FAIL or ERROR, 
	 * the transition is completed, however machine will return
	 * SMT_MACHINE_ERROR_ACTION_ERROR regardless whether FAIL or
	 * ERROR returned by the entry action
	 */
	SMT_ACTION_DONE,	/* action complete successfully */
	SMT_ACTION_FAIL,	/* action cannot be completed */
	SMT_ACTION_ERROR	/* error occurred executing action */
} smt_actionReturnStatus_t;

/* transition action function pointer */
typedef smt_actionReturnStatus_t (* smt_transActionFuncPtr_t)(void* context, int * targetState);

/* entry/exit action function pointer */
typedef smt_actionReturnStatus_t (* smt_actionFuncPtr_t)(void* context);

typedef int (* const smt_eventHandler_t)(int eventType);

/* the state structure */
typedef struct smt_state
{
	const int				id; 
	const boolean			isFinalState;	/* Final state will not have following attributes */
	boolean					isSubStateMachine; /* Is the state a sub state machine */
	const struct smt_stateMachine * const subStateMachine; /* Pointer to the sub state machine */
														   /* if it is a sub state machine, or NULL. */ 
	/*
	 * Note: 
	 * if entry Action return FAIL or ERROR
	 * then SMT_MACHINE_ERROR_MACHINE_ERROR will be rasied
	 * the state machine shutdown immediately. 
	 * The rationale behind this process is that entry action
	 * fail will prevent state machine being stay on a stable
	 * status
	 */
	/*@null@*/	smt_actionFuncPtr_t	entryAction;	/* NULL means no entry action */
	/*@null@*/	smt_actionFuncPtr_t	exitAction;		/* NULL means no exit action */
} smt_state_t;

typedef smt_state_t const * smt_state_ptr_const_t;

typedef smt_state_t * smt_state_ptr_t;


const smt_state_t SMT_FINAL_STATE = 
{
    (UINT8)SMT_FINAL_STATE_ID,
	TRUE, /* is final state */
    FALSE, /* is sub state machine */
	NULL, /* sub state machine */
    NULL, /* entry action */
    NULL, /* exit action */
};


/* the transition type */
typedef struct 
{
	const smt_eventId_t							id;
	/*@null@*/	const smt_guardFuncPtr_t		guard;
	/*@null@*/	const smt_transActionFuncPtr_t	action;
	smt_state_ptr_const_t	const				sourceState;
	smt_state_ptr_const_t	const				targetState;
} smt_transition_t;

/* the trigger type */
typedef struct 
{
	/* Event type. Event type must start from 0 and grows incrementally */
	const smt_eventId_t				eventId;
	const unsigned int				transitionNum;
	smt_transition_t const * const	transitions;
} smt_trigger_t;


/* type definition for state machine */
typedef struct smt_stateMachine
{
	const int id;

	/* state table */
	smt_state_ptr_const_t * const stateTable;

	/* state number */
	const int stateNum;

	/* transition table */
	const smt_transition_t *transitionTable;

	/* transition number */
	const int transitionNum;

	/* trigger table. Triggers must be put in this table in order of its event id */
	smt_trigger_t const * const triggerTable;

	/* event number */
	const smt_eventId_t eventCount;

	/* is a sub state machine */
	const boolean isSubStateMachine;
	
	/* parent state machine */
	struct smt_stateMachine const * const parent;

	/* the initial state */
	smt_state_ptr_const_t	entryState;

	/* entry action */
	smt_actionFuncPtr_t const entryAction;

	/* exit action */
	smt_actionFuncPtr_t const exitAction;

	/* active state of the machine */
	smt_state_t	*activeState;

} smt_stateMachine_t;

typedef smt_stateMachine_t * const smt_stateMachine_ptr_t;


typedef enum
{
	SMT_MACHINE_OK,
	SMT_MACHINE_GARBAGE_EVENT,
	SMT_MACHINE_ERROR_ACTION_ERROR,

	/* if fatal error encountered, the machine shutdown immediately */
	SMT_MACHINE_ERROR_FATAL,
	SMT_MACHINE_ERROR_MACHINE_ERROR = SMT_MACHINE_ERROR_FATAL
} smt_machineStatus_t;


//#define smtMachineIsFinalized(machine)\
//	(SMT_FINAL_STATE.id == ((smt_stateMachine_ptr_t)machine)->activeState->id)



/* entry function to use the statemachine tool */
smt_machineStatus_t
smtMachineRun(
		smt_stateMachine_ptr_t	machine,
		smt_eventId_t	        event,
		void*	                context);



/* used to build statemachine dynamically */
//smt_stateMachine_t* createNewStateMachine();




#endif
