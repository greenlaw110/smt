#include "mw_machine.h"
#include "smt.h"
#include "smt_defs.h"
#include "mw_context.h"
#include "mw_machine_door_closed.h"

/*
 * ACTIONS 
 */
static smt_actionReturnStatus_t enterDoorClosed(void * context) {
    mw_context_t * mwContext = (mw_context_t *) context;
    mwContext->statePath = "doorClosed";
    return SMT_ACTION_DONE;
}

static smt_actionReturnStatus_t enterDoorOpen(void * context) {
    mw_context_t * mwContext = (mw_context_t *) context;
    mwContext->statePath = "doorOpen";
    mwContext->lampOn = TRUE;
    return SMT_ACTION_DONE;
}

static smt_actionReturnStatus_t exitDoorOpen(void * context) {
    mw_context_t * mwContext = (mw_context_t *) context;
    mwContext->lampOn = FALSE;
    return SMT_ACTION_DONE;
}

/*
 * Machine destructor 
 */
static void onDestroy(void * context) {
    mw_context_t * mwContext = (mw_context_t *) context;
    mwContext->statePath = "<destoryed>";
    mwContext->lampOn = FALSE;
    mwContext->heaterOn = FALSE;
}

/*
 * STATES
 */

static smt_state_t doorClosed = {
    .id = DOOR_CLOSED,
    .subStateMachine = &DoorClosedStateMachine,
    .entryAction = &enterDoorClosed
};

static smt_state_t doorOpen = {
    .id = DOOR_OPEN,
    .entryAction = &enterDoorOpen,
    .exitAction = &exitDoorOpen
};

static smt_state_ptr_const_t stateList[] = {
    &doorClosed, &doorOpen
};

/*
 * Transitions
 */
static smt_transition_t transitionList[] = {
    {
        .event = OPEN_DOOR,
        .sourceState = &doorClosed,
        .targetState = &doorOpen
    },
    {
        .event = CLOSE_DOOR,
        .sourceState = &doorOpen,
        .targetState = &doorClosed,
		.restoreHistory = TRUE
    }
};

/* THE machine */
smt_stateMachine_t MicrowaveStateMachine = {
    .stateList = stateList,
    .stateCount = sizeof(stateList) / sizeof(smt_state_ptr_const_t),
    .transitionList = &transitionList,
    .transitionCount = sizeof(transitionList) / sizeof(smt_transition_t),
    .destructor = &onDestroy
};
