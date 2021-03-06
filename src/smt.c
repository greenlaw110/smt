#include <string.h>

#include "smt_defs.h"
#include "smt.h"
#include "smt_user_defs.h"

#ifdef SMT_DEBUG
const char* SMT_FINAL_STATE_STR = "SMT_FINAL_STATE";
const char* SMT_ENTRY_STATE_STR = "SMT_ENTRY_STATE";
const char* SMT_HISTORY_STATE_STR = "SMT_HISTORY_STATE";
#endif

/* ------------------- Internal data structures -------------------- */
typedef enum {
    /* access transition by eventId */
    SMT_TL_COMPACT_ARRAY,
    /* access transition by iterating list and check transition event ID */
    SMT_TL_LINKED_LIST
} smt_transition_lookup_type_t;

typedef struct smt_transition_lookup_node {
    smt_transition_ptr_const_t transition;
    struct smt_transition_lookup_node * next;
} smt_transition_lookup_node_t;

typedef struct smt_transition_lookup {
    smt_transition_lookup_type_t type;
    smt_transition_ptr_const_t * compactArray;
    smt_transition_lookup_node_t * linkedList;
} smt_transition_lookup_t;

typedef struct smt_stateMachine_data {
    /* transition look up table by eventId and source stateId */
    smt_transition_ptr_const_t** transitionLookup;
    /* transition lists indexed by source stateId */
    smt_transition_lookup_t** transitionLookupByStateId;
    smt_state_ptr_const_t entryState;
    smt_state_ptr_const_t activeState;
    smt_state_ptr_const_t historyState;
    smt_transition_ptr_const_t* entryTransitions;
    smt_counter_t entryTransitionCount;
    smt_counter_t maxEventId;
    smt_counter_t maxStateId;
} smt_stateMachine_data_t;

typedef smt_stateMachine_data_t* const smt_stateMachine_data_const_ptr_t;

/* ------------------- Local Macros -------------------------------- */
#define smtTransitionHasGuard(x) (NULL != ((smt_transition_t*)x)->guard)
#define smtTransitionGetGuard(x) (((smt_transition_t*)x)->guard)
#define smtTransitionHasAction(x) (NULL != ((smt_transition_t*)x)->action)
#define smtTransitionGetAction(x) (((smt_transition_t*)x)->action)
#define smtGetTransitionSourceState(x) (((smt_transition_t*)x)->sourceState)
#define smtGetTransitionTargetState(x) (((smt_transition_t*)x)->targetState)
#define smtGetStateId(x) (((smt_state_ptr_const_t)x)->stateId)
#define smtIsStateFinal(x) \
    ((NULL != x) && ((smt_state_ptr_const_t)x)->id == SMT_FINAL_STATE_ID)
#define smtIsSubStateMachine(x) \
    (((smt_state_ptr_const_t)x)->subStateMachine != NULL)
#define smtStateGetEntryAction(x) (((smt_state_ptr_const_t)x)->entryAction)
#define smtStateHasEntryAction(x) \
    (NULL != ((smt_state_ptr_const_t)x)->entryAction)
#define smtStateGetExitAction(x) (((smt_state_ptr_const_t)x)->exitAction)
#define smtStateHasExitAction(x) \
    (NULL != ((smt_state_ptr_const_t)x)->exitAction)
#define smtStateGetSubStateMachine(x) \
    ((const smt_stateMachine_ptr_t)((smt_state_ptr_const_t)x)->subStateMachine)

#define smtMachineGetInternalData(machie)                                 \
    ((smt_stateMachine_data_const_ptr_t)((smt_stateMachine_ptr_t)machine) \
         ->internalData)

static smt_machineStatus_t smtEnterStateMachine(
    smt_stateMachine_ptr_t machine, const smt_transition_ptr_const_t transition,
    boolean restoreHistory, void* context);

static smt_machineStatus_t smtExitStateMachine(smt_stateMachine_ptr_t machine,
                                               void* context);

static smt_machineStatus_t smtProcessTransition(
      const smt_stateMachine_ptr_t machine,
      const smt_transition_ptr_const_t transition, void* context);

/* ------------------- local function prototypes --------------------- */

static void * _get_buffer(size_t size) {
    void * buf = smt_get_buffer(size);
    memset(buf, 0, size);
    return buf;
}

static smt_transition_ptr_const_t smt_lookupTransition(smt_stateMachine_data_t * data, smt_eventId_t event, smt_stateId_t state) {
    if (event > data->maxEventId) {
        return NULL;
    }
    if (state > data->maxStateId) {
      if (state == SMT_ENTRY_STATE_ID) {
        int i;
        for (i = 0; i < data->entryTransitionCount; ++i) {
          if (event == data->entryTransitions[i]->event) {
            return data->entryTransitions[i];
          }
        }
      }
    }
    smt_transition_ptr_const_t transition = NULL;
    if (NULL != data->transitionLookup) {
        transition = data->transitionLookup[event][state];
    } else if (NULL != data->transitionLookupByStateId) {
        smt_transition_lookup_t * lookup = data->transitionLookupByStateId[state];
        if (SMT_TL_LINKED_LIST == lookup->type) {
            smt_transition_lookup_node_t * node = lookup->linkedList;
            while (NULL != node) {
                if (event == node->transition->event) {
                    transition = node->transition;
                    break;
                }
                node = node->next;
            }
        } else {
            transition = lookup->compactArray[event];
        }
    }
  return transition;
}

static smt_machineStatus_t smtEnterState(
    const smt_stateMachine_ptr_t machine,
    const smt_state_ptr_const_t targetState,
    const smt_transition_ptr_const_t transition,
  boolean restoreHistory,
    /*@null@*/ void* context)
{
    smt_stateMachine_data_const_ptr_t data = smtMachineGetInternalData(machine);
    smt_machineStatus_t retVal = SMT_MACHINE_OK;

    /*
     * 1. execute the entry action
     * 2. If the state linked to a sub state machine, start the sub state machine
     * 3. If entry action failed/error at any level, break process immediately
     */

    /* execute entry action */
    if (smtStateHasEntryAction(targetState)) {
        smt_actionFuncPtr_t func = smtStateGetEntryAction(targetState);
        smt_actionReturnStatus_t v = func(context);
        if (SMT_ACTION_DONE != v) {
            /*
             * entry action fail is considered to be fatal
             * error cause state machine is placed at an
             * unstable status since transition action is
             * completed, but the target state cannot be
             * entered
             */
            retVal = SMT_MACHINE_ERROR_MACHINE_ERROR;
            return retVal;
        }
    };

    data->activeState = targetState;

    /* when transition is NULL, it means initialize the machine to entry state */
    if (NULL != transition) {
      if (smtIsSubStateMachine(targetState)) {
        return smtEnterStateMachine(smtStateGetSubStateMachine(targetState),
                                      transition, restoreHistory, context);
      } else if (targetState->id == SMT_ENTRY_STATE_ID) {
        return smtProcessTransition(machine, transition, context);
      }
    }

    if (restoreHistory && smtIsSubStateMachine(targetState)) {
    return smtEnterStateMachine(smtStateGetSubStateMachine(targetState),
                                  transition, restoreHistory, context);
    }

    return retVal;
}

static smt_machineStatus_t smtExitState(smt_stateMachine_ptr_t machine,
                                        smt_state_ptr_const_t srcState,
                                        void* context)
{
    smt_machineStatus_t retVal = SMT_MACHINE_OK;
    smt_actionReturnStatus_t actionRetVal;

    /*
     * 1. execute the exit action
     * 2. If the state is asociated with a sub state machine, then exit the sub
     * state machine
     */

    /* execute exit action */
    if (smtStateHasExitAction(srcState)) {
        smt_actionFuncPtr_t func = smtStateGetExitAction(srcState);
        actionRetVal = func(context);
        if (SMT_ACTION_DONE != actionRetVal) {
            /*
             * exit action fail is considered to be fatal
             * error cause state machine is placed at an
             * unstable status since transition action is
             * completed, but the target state cannot be
             * entered
             */
            retVal = SMT_MACHINE_ERROR_MACHINE_ERROR;
            return retVal;
        }
    }

    if (smtIsSubStateMachine(srcState)) {
        retVal = smtExitStateMachine(smtStateGetSubStateMachine(srcState), context);
    }

    return retVal;
}

static smt_machineStatus_t smtProcessTransition(
    const smt_stateMachine_ptr_t machine,
    const smt_transition_ptr_const_t transition, void* context)
{
    smt_stateMachine_data_const_ptr_t data = smtMachineGetInternalData(machine);
    smt_state_ptr_const_t srcState = smtGetTransitionSourceState(transition);
    smt_state_ptr_const_t tgtState = smtGetTransitionTargetState(transition);
    const smt_transActionFuncPtr_t func = smtTransitionGetAction(transition);

    if (NULL != data->activeState && data->activeState != srcState) {
        /* something wrong with our SMT */
        return SMT_MACHINE_ERROR_MACHINE_ERROR;
    }

    if (smtTransitionHasGuard(transition) && !smtTransitionGetGuard(transition)(context)) {
        return SMT_MACHINE_OK;
    }

    smt_actionReturnStatus_t actionRetVal = SMT_ACTION_DONE;
    if (srcState != tgtState) {
        actionRetVal = smtExitState(machine, srcState, context);
        if (SMT_ACTION_DONE != actionRetVal) {
            return SMT_MACHINE_ERROR_MACHINE_ERROR;
        }
    }

    if (NULL != func) {
        actionRetVal = func(context, &tgtState);
    }

    if (SMT_ACTION_DONE != actionRetVal) {
        /* transition action fail is considered to be fatal error */
        return SMT_MACHINE_ERROR_MACHINE_ERROR;
    }

    if (srcState != tgtState && srcState == smtGetTransitionTargetState(transition)) {
        smt_actionReturnStatus_t actionRetVal = smtExitState(machine, srcState, context);
        if (SMT_ACTION_DONE != actionRetVal) {
            return SMT_MACHINE_ERROR_MACHINE_ERROR;
        }
    }

    if (srcState != tgtState) {
        return smtEnterState(machine, tgtState, transition, (boolean)FALSE, context);
    }

    return SMT_MACHINE_OK;
}

static smt_machineStatus_t smtEnterStateMachine(
    smt_stateMachine_ptr_t const machine,
    const smt_transition_ptr_const_t transition, boolean restoreHistory, void* context)
{
    smt_state_ptr_const_t state = NULL;
    smt_stateMachine_data_const_ptr_t data = smtMachineGetInternalData(machine);
    if (restoreHistory || transition->restoreHistory) {
        state = data->historyState;
        restoreHistory = TRUE;
    }

    if (NULL == state) {
        state = data->entryState;
    }
    smt_transition_ptr_const_t localTransition = NULL;
    if (NULL != transition) {
    localTransition = smt_lookupTransition(data, transition->event, state->id);
    }
    return smtEnterState(machine, state, localTransition, restoreHistory, context);
}

/* leave smate machine */
static smt_machineStatus_t smtExitStateMachine(smt_stateMachine_ptr_t machine,
                                               void* context)
{
    smt_stateMachine_data_const_ptr_t data = smtMachineGetInternalData(machine);
    if (NULL != data->activeState) {
        if (smtIsSubStateMachine(data->activeState)) {
            smtExitState(machine, data->activeState, context);
        }
        data->historyState = data->activeState;
        data->activeState = data->entryState;
    }
    return SMT_MACHINE_OK;
}

static void smtMachineFinalize(smt_stateMachine_ptr_t machine)
{
    smt_stateMachine_data_const_ptr_t data = smtMachineGetInternalData(machine);
    data->activeState = &SMT_FINAL_STATE;
}

static smt_machineStatus_t smt_handleEvent(smt_stateMachine_ptr_t machine,
                                           smt_eventId_t event, void* context)
{
    smt_stateMachine_data_const_ptr_t data = smtMachineGetInternalData(machine);
    smt_state_ptr_const_t state = data->activeState;

    if (NULL == state) {
        state = data->entryState;
    }

    /* check current state, make sure we are not working on pseudo final state */
    if (smtIsStateFinal(state)) {
        /* error: cannot process normal event on final state */
        return SMT_MACHINE_ERROR_MACHINE_ERROR;
    }

    /* prevent buffer overflow attack */
    if (event > data->maxEventId) {
      if (smtIsSubStateMachine(state)) {
        smt_stateMachine_ptr_t subMachine = smtStateGetSubStateMachine(state);
        return smt_handleEvent(subMachine, event, context);
      }
        return SMT_MACHINE_ERROR_UNKNOWN;
    }

    smt_transition_ptr_const_t transition = smt_lookupTransition(data, event, state->id);
    if (NULL == transition) {
        return SMT_MACHINE_ERROR_UNKNOWN;
    }

    return smtProcessTransition(machine, transition, context);
}

/*
 * - find max stateId
 * - find entry state
 * - recursively build subStateMachine
 */
static void smt_processStateList(const smt_stateMachine_t* machine,
                                 smt_counter_t* maxId,
                                 smt_state_ptr_const_t* entryState,
                                 smt_lookup_table_optimize_hint_t optimizeHint,
                                 void* context)
{
    smt_stateMachine_t* subMachine;
    smt_state_ptr_const_t state;
    smt_stateId_t stateId;
    *maxId = 0;
    *entryState = machine->stateList[0];
    int i;
    for (i = 0; i < machine->stateCount; i++) {
        state = machine->stateList[i];
        stateId = state->id;
        if (stateId < SMT_ENTRY_STATE_ID) {
            *maxId = MAX(*maxId, stateId);
            subMachine = smtStateGetSubStateMachine(state);
            if (NULL != subMachine) {
                smtMachineInit(subMachine, optimizeHint, context);
            }
        } else if (stateId == SMT_ENTRY_STATE_ID) {
            *entryState = state;
        }
    }
}

/*
 * - find out max event id
 */
static void smt_processTransitionList(
    const smt_stateMachine_t* machine, smt_counter_t* maxEventId,
    smt_counter_t* entryStateTransitionCount)
{
    smt_transition_ptr_const_t transition;
    *maxEventId = 0;
    *entryStateTransitionCount = 0;
    int i;
    for (i = 0; i < machine->transitionCount; i++) {
        transition = &(machine->transitionList[i]);
        *maxEventId = MAX(*maxEventId, transition->event);
        if (SMT_ENTRY_STATE_ID == transition->sourceState->id) {
            *entryStateTransitionCount = (*entryStateTransitionCount) + 1;
        }
    }
}

static smt_machineStatus_t smt_buildStateMachine(
    smt_stateMachine_t* machine, smt_counter_t maxStateId,
    smt_counter_t maxEventId, smt_state_ptr_const_t entryState,
    smt_counter_t entryStateTransitionCount, smt_lookup_table_optimize_hint_t optimizeHint,
     void* context)
{
    smt_transition_ptr_const_t** transitionLookup = NULL;
    smt_transition_ptr_const_t* entryTransitions = NULL;
    smt_transition_lookup_t** transitionLookupByStateId = NULL;
    int entryStateTransitionCursor = 0;
    if (machine->transitionCount > 0) {
        if (SMT_OPTIMIZE_TIME == optimizeHint) {
            transitionLookup = _get_buffer((maxEventId + 1) * sizeof(int*));
            int i;
            for (i = 0; i <= maxEventId; i++) {
                transitionLookup[i] = _get_buffer((maxStateId + 1) * sizeof(smt_transition_ptr_const_t));
            }
            if (entryStateTransitionCount > 0) {
                entryTransitions = _get_buffer(
                  entryStateTransitionCount * sizeof(smt_transition_ptr_const_t));
            }
            smt_transition_ptr_const_t transition;
            smt_counter_t event_id, state_id;
            for (i = 0; i < machine->transitionCount; i++) {
                transition = &machine->transitionList[i];
                switch (transition->sourceState->id) {
                case(SMT_ENTRY_STATE_ID)
                    :
                    entryTransitions[entryStateTransitionCursor++] = transition;
                    continue;
                case(SMT_FINAL_STATE_ID)
                    :
                case(SMT_HISTORY_STATE_ID)
                    :
                    return SMT_MACHINE_ERROR_FATAL;
                default:
                    event_id = transition->event;
                    state_id = transition->sourceState->id;
                    transitionLookup[event_id][state_id] = transition;
                }
            }
        } else {
            transitionLookupByStateId = NULL;
            boolean skipLookupBuild = FALSE;
            if (entryStateTransitionCount > 0) {
                entryTransitions = _get_buffer(
                  entryStateTransitionCount * sizeof(smt_transition_ptr_const_t));
                int i, consumed = 0;
                for (i = 0; i < machine->transitionCount; i++) {
                    smt_transition_ptr_const_t transition = &machine->transitionList[i];
                  if (SMT_ENTRY_STATE_ID == transition->sourceState->id) {
                    entryTransitions[entryStateTransitionCursor++] = transition;
                    consumed++;
                  }
                }
                skipLookupBuild = consumed >= machine->transitionCount;
            }
            if (!skipLookupBuild) {
              transitionLookupByStateId = _get_buffer((maxStateId + 1) * sizeof(smt_transition_lookup_t *));
                int i, j;
                for (i = 0; i <= maxStateId; ++i) {
                    int eventCount = 0;
                    smt_transition_lookup_t * lookup = NULL;
                    for (j = 0; j < machine->transitionCount; j++) {
                        smt_transition_ptr_const_t transition = &machine->transitionList[j];
                        if (i != transition->sourceState->id) {
                            continue;
                        }
                        if (NULL == lookup) {
                            lookup = _get_buffer(sizeof(smt_transition_lookup_t));
                            lookup->type = SMT_TL_LINKED_LIST;
                            lookup->compactArray = NULL;
                            lookup->linkedList = NULL;
                        }
                        if (SMT_OPTIMIZE_AUTO == optimizeHint && ++eventCount > 4) {
                            if (NULL == lookup->compactArray) {
                                /* switch to compact array */
                                lookup->compactArray = _get_buffer((maxEventId + 1) * sizeof(smt_transition_ptr_const_t));
                                smt_transition_lookup_node_t * node = lookup->linkedList;
                                while (NULL != node) {
                                    lookup->compactArray[node->transition->event] = node->transition;
                                    smt_transition_lookup_node_t * tmp = node;
                                    node = node->next;
                                    smt_free_buffer(tmp);
                                }
                            }
                            lookup->compactArray[i] = transition;
                        } else {
                            smt_transition_lookup_node_t * node = _get_buffer(sizeof(smt_transition_lookup_node_t));
                            node->transition = transition;
                            if (NULL != lookup->linkedList) {
                                lookup->linkedList->next = node;
                            } else {
                                lookup->linkedList = node;
                            }
                        }
                    }
                    transitionLookupByStateId[i] = lookup;
                }
            }
        }
    }
    smt_stateMachine_data_t* data = _get_buffer(sizeof(smt_stateMachine_data_t));
    data->transitionLookupByStateId = transitionLookupByStateId;
    data->transitionLookup = transitionLookup;
    data->entryState = entryState;
    data->activeState = NULL;
    data->historyState = NULL;
    data->entryTransitions = entryTransitions;
    data->entryTransitionCount = entryStateTransitionCursor;
    data->maxEventId = maxEventId;
    data->maxStateId = maxStateId;
    machine->internalData = data;
    return smtEnterState(machine, data->entryState, NULL, FALSE, context);;
}


/* ------------------- external const data ---------------------
 */

smt_state_t SMT_FINAL_STATE = { SMT_FINAL_STATE_ID,
                                NULL, /* sub state machine */
                                NULL, /* entry action */
                                NULL, /* exit action */
};

smt_state_t SMT_HISTORY_STATE = { SMT_HISTORY_STATE_ID,
                                  NULL, /* sub state machine */
                                  NULL, /* entry action */
                                  NULL, /* exit action */
};

/* ------------------- external function implementations ---------------------
 */

smt_machineStatus_t smtMachineInit(smt_stateMachine_ptr_t machine, smt_lookup_table_optimize_hint_t optimizeHint, void* context)
{
    smt_machineStatus_t retVal = SMT_MACHINE_OK;
    smt_counter_t maxStateId = -1;
    smt_state_ptr_const_t entryState = NULL;
    smt_counter_t maxEventId = -1;
    smt_counter_t entryStateTransitionCount = -1;
    smt_processStateList(machine, &maxStateId, &entryState, optimizeHint, context);
    smt_processTransitionList(machine, &maxEventId, &entryStateTransitionCount);
    smt_buildStateMachine(machine, maxStateId, maxEventId, entryState,
                          entryStateTransitionCount, optimizeHint, context);
    return retVal;
}

smt_machineStatus_t smtMachineRun(smt_stateMachine_ptr_t machine,
                                  smt_eventId_t event, void* context)
{
  if (SMT_SHUTDOWN_EVENT == event) {
    smtMachineFinalize(machine);
    return SMT_MACHINE_OK;
  }

  smt_machineStatus_t retVal = SMT_MACHINE_OK;
    smt_stateMachine_data_const_ptr_t data = smtMachineGetInternalData(machine);

    if (NULL != data->activeState && data->activeState->id == SMT_FINAL_STATE_ID) {
        return SMT_MACHINE_ERROR_UNKNOWN;
    }

    retVal = smt_handleEvent(machine, event, context);
    if (retVal >= SMT_MACHINE_ERROR_FATAL) {
        smtMachineFinalize(machine);
    }

    return retVal;
}

void smtMachineDestroy(smt_stateMachine_ptr_t machine, void* context) {
    smt_stateMachine_t* subMachine;
    smt_state_ptr_const_t state;
    smt_stateId_t stateId;
    int i;
    for (i = 0; i < machine->stateCount; i++) {
        state = machine->stateList[i];
        subMachine = smtStateGetSubStateMachine(state);
        if (NULL != subMachine) {
          smtMachineDestroy(subMachine, context);
        }
    }
    smt_stateMachine_data_const_ptr_t data = smtMachineGetInternalData(machine);
    if (NULL == data) {
        return;
    }
    if (NULL != data->transitionLookup) {
        int i;
        for (i = 0; i < data->maxEventId + 1; ++i) {
        smt_free_buffer(data->transitionLookup[i]);
        }
        smt_free_buffer(data->transitionLookup);
    } else if (NULL != data->transitionLookupByStateId) {
        int i;
        for (i = 0; i < data->maxStateId + 1; ++i) {
            smt_transition_lookup_t* lookup = data->transitionLookupByStateId[i];
            if (NULL != lookup->linkedList) {
                smt_transition_lookup_node_t * node = lookup->linkedList;
                while (NULL != node) {
                    smt_transition_lookup_node_t * tmp = node;
                    node = node->next;
                    smt_free_buffer(tmp);
                }
            } else if (NULL != lookup->compactArray) {
                smt_free_buffer(lookup->compactArray);
            }
            smt_free_buffer(lookup);
        }
        smt_free_buffer(data->transitionLookupByStateId);
    }
    if (NULL != data->entryTransitions) {
        smt_free_buffer(data->entryTransitions);
    }
    smt_free_buffer(data);
    machine->internalData = NULL;
    if (NULL != machine->destructor) {
        machine->destructor(context);
    }
}

boolean smtIsMachineFinalized(smt_stateMachine_ptr_t machine)
{
    smt_stateMachine_data_const_ptr_t data = smtMachineGetInternalData(machine);
    return smtIsStateFinal(data->activeState);
}
