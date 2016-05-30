#include "mw_machine_heating.h"
#include "mw_machine.h"
#include "smt.h"
#include "smt_defs.h"
#include "mw_context.h"

/*
 * ACTIONS 
 */
static smt_actionReturnStatus_t enterBaking(void * context) {
    mw_context_t * mwContext = (mw_context_t *) context;
    mwContext->statePath = "doorClosed/heating/baking";
    return SMT_ACTION_DONE;
}

static smt_actionReturnStatus_t enterToasting(void * context) {
    mw_context_t * mwContext = (mw_context_t *) context;
    mwContext->statePath = "doorClosed/heating/toasting";
    return SMT_ACTION_DONE;
}

/*
 * STATES
 */

static smt_state_t baking = {
    .id = BAKING,
    .entryAction = &enterBaking
};

static smt_state_t toasting = {
    .id = TOASTING,
    .entryAction = &enterToasting
};

static smt_state_t entryDispatcher = {
    .id = SMT_ENTRY_STATE_ID
};

static smt_state_ptr_const_t stateList[] = {
    &entryDispatcher, &baking, &toasting
};

/*
 * Transitions
 * ------------------
 * In the heating machine, the only transition is
 * the entry state dispatching, say if entry event
 * is BAKE, then transit to BAKING state; while if
 * entry event is TOAST, then transit to TOASTING 
 * state
 */
static smt_transition_t transitionList[] = {
    {
        .event = BAKE,
        .sourceState = &entryDispatcher,
        .targetState = &baking
    },
    {
        .event = TOAST,
        .sourceState = &entryDispatcher,
        .targetState = &toasting
    }
};

/* THE machine */
smt_stateMachine_t HeatingStateMachine = {
    .stateList = stateList,
    .stateCount = sizeof(stateList) / sizeof(smt_state_ptr_const_t),
    .transitionList = &transitionList,
    .transitionCount = sizeof(transitionList) / sizeof(smt_transition_t)
};
