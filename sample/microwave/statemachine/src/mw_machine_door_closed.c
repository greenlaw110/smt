#include "mw_machine_door_closed.h"
#include "mw_machine.h"
#include "smt.h"
#include "smt_defs.h"
#include "mw_context.h"
#include "mw_machine_heating.h"

/*
 * ACTIONS 
 */
static smt_actionReturnStatus_t enterHeating(void * context) {
    mw_context_t * mwContext = (mw_context_t *) context;
    mwContext->statePath = "doorClosed/heating";
    mwContext->heaterOn = TRUE;
    return SMT_ACTION_DONE;
}

static smt_actionReturnStatus_t exitHeating(void * context) {
    mw_context_t * mwContext = (mw_context_t *) context;
    mwContext->heaterOn = FALSE;
    return SMT_ACTION_DONE;
}

static smt_actionReturnStatus_t enterOff(void * context) {
    mw_context_t * mwContext = (mw_context_t *) context;
    mwContext->statePath = "doorClosed/off";
    return SMT_ACTION_DONE;
}

/*
 * STATES
 */

static smt_state_t off = {
    .id = OFF,
    .entryAction = &enterOff
};

static smt_state_t heating = {
    .id = HEATING,
    .subStateMachine = &HeatingStateMachine,
    .entryAction = &enterHeating,
    .exitAction = &exitHeating
};


static smt_state_ptr_const_t stateList[] = {
    &off, &heating
};

/*
 * Transitions
 */

static smt_transition_t transitionList[] = {
    {
        .event = BAKE,
        .sourceState = &off,
        .targetState = &heating
    },
    {
        .event = TOAST,
        .sourceState = &off,
        .targetState = &heating
    },
    {
        .event = TURN_OFF,
        .sourceState = &heating,
        .targetState = &off
    }
};

/* THE machine */
smt_stateMachine_t DoorClosedStateMachine = {
    .stateList = stateList,
    .stateCount = sizeof(stateList) / sizeof(smt_state_ptr_const_t),
    .transitionList = &transitionList,
    .transitionCount = sizeof(transitionList) / sizeof(smt_transition_t)
};
