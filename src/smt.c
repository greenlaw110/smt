#include <string.h>

#include "defs.h"
#include "smt.h"

#ifdef SMT_DEBUG
const char* SMT_FINAL_STATE_STR = "SMT_FINAL_STATE";
#endif

/* ------------------- Local Macros -------------------------------- */
#define smtTransitionHasGuard(x)                    (NULL != ((smt_transition_t*)x)->guard)
#define smtTransitionGetGuard(x)                    (((smt_transition_t*)x)->guard)
#define smtTransitionHasAction(x)                   (NULL != ((smt_transition_t*)x)->action)
#define smtTransitionGetAction(x)                   (((smt_transition_t*)x)->action)
#define smtGetTransitionSourceState(x)              (((smt_transition_t*)x)->sourceState)
#define smtGetTransitionTargetState(x)              (((smt_transition_t*)x)->targetState)
#define smtMachineGetEventCount(machine)            (((smt_stateMachine_ptr_t)machine)->eventCount)
#define smtMachineFindTrigger(machine,event)        (&(((smt_stateMachine_ptr_t)machine)->triggerTable[event]))

#define smtIsStateFinal(x)                          (((smt_state_ptr_const_t)x)->isFinalState)
#define smtGetStateId(x)                            (((smt_state_ptr_const_t)x)->stateId)
#define smtIsStateFinal(x)                          (((smt_state_ptr_const_t)x)->isFinalState)
#define smtIsSubStateMachine(x)                     (((smt_state_ptr_const_t)x)->isSubStateMachine)
#define smtStateGetEntryAction(x)                   (((smt_state_ptr_const_t)x)->entryAction)
#define smtStateHasEntryAction(x)                   (NULL != ((smt_state_ptr_const_t)x)->entryAction)
#define smtStateGetExitAction(x)                    (((smt_state_ptr_const_t)x)->exitAction)
#define smtStateHasExitAction(x)                    (NULL != ((smt_state_ptr_const_t)x)->exitAction)
#define smtStateGetSubStateMachine(x)               (((smt_state_ptr_const_t)x)->subStateMachine)

#define smtStateMachineHasEntryAction(machine)      (NULL != ((smt_stateMachine_t *)machine)->entryAction)
#define smtStateMachineHasExitAction(machine)       (NULL != ((smt_stateMachine_t *)machine)->exitAction)
#define smtStateMachineGetEntryAction(machine)      (((smt_stateMachine_t *)machine)->entryAction)
#define smtStateMachineGetExitAction(machine)       (((smt_stateMachine_t *)machine)->exitAction)

#define smtMachineGetActiveState(machine)           (((smt_stateMachine_t *)machine)->activeState)
#define smtMachineSetActiveState(machine, state)    (((smt_stateMachine_t *)machine)->activeState = state)
#define smtMachineGetEntryState(machine)            (((smt_stateMachine_ptr_t)machine)->entryState)


#define smtMachineIsFinalized(machine)              (SMT_FINAL_STATE.id == ((smt_stateMachine_ptr_t)machine)->activeState->id)


static smt_machineStatus_t
smtEnterStateMachine(smt_stateMachine_ptr_t   machine, void* context);

static smt_machineStatus_t
smtExitStateMachine(smt_stateMachine_ptr_t  machine, void* context);

/* ------------------- local function prototypes --------------------- */

/*
 * ASSUMPTION:
 */
static
smt_machineStatus_t
smtEnterState(
    smt_stateMachine_ptr_t  machine,
    smt_state_ptr_const_t   targetState,
    /*@null@*/ void *       context)
{
    smt_machineStatus_t retVal =  SMT_MACHINE_OK;

    /*
     * 1. execute the entry action
     * 2. If the state is asociated with a sub state machine, then start the sub state machine
     * 4. If entry action failed/errored at any level, break process immediately
     */


    /* execute entry action */
    if (smtStateHasEntryAction(targetState))
    {
        smt_actionFuncPtr_t func = smtStateGetEntryAction(targetState);
        retVal = func(context);
        if (SMT_ACTION_DONE != retVal)
        {
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

    smtMachineSetActiveState(machine, targetState);

    if (smtIsSubStateMachine(targetState))
    {
        retVal = smtEnterStateMachine(smtStateGetSubStateMachine(targetState), context);
    }


    /* Not a compound state, so we end the job */

    return retVal;
}

/*
 * PRECONDITIONS:
 *
 *
 * POSTCONDITINOS:
 *
 * IMPLEMENTATION NOTES:
 */
static
smt_machineStatus_t
smtExitState(
    smt_stateMachine_ptr_t machine,
    smt_state_ptr_const_t  srcState,
    void *                 context)
{
    smt_machineStatus_t retVal =  SMT_MACHINE_OK;

    /*
     * 1. execute the exit action
     * 2. If the state is asociated with a sub state machine, then exit the sub state machine
     */


    /* execute exit action */
    if (smtStateHasExitAction(srcState))
    {
        smt_actionFuncPtr_t func = smtStateGetExitAction(srcState);
        retVal = func(context);
        if (SMT_ACTION_DONE != retVal)
        {
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

    if (smtIsSubStateMachine(srcState))
    {
        retVal = smtExitStateMachine(smtStateGetSubStateMachine(srcState), context);
    }

    return retVal;
}



static smt_machineStatus_t
smtProcessTransition(
    smt_stateMachine_ptr_t machine,
    smt_transition_t       *transition,
    void*                  context)
{
    smt_machineStatus_t retVal = SMT_MACHINE_OK;

    smt_state_ptr_const_t   *srcState = smtGetTransitionSourceState(transition);
    smt_state_ptr_const_t   *tgtState = smtGetTransitionTargetState(transition);
    smt_actionFuncPtr_t     func = smtTransitionGetAction(transition);

    if (smtMachineGetActiveState(machine) != srcState)
    {
        return retVal;
    }

    retVal = smtExitState(machine, srcState, context);
    if (SMT_MACHINE_OK != retVal)
    {
        return SMT_MACHINE_ERROR_MACHINE_ERROR;
    }

    if (smtTransitionHasGuard(transition) && !smtTransitionGetGuard(transition)(context))
    {
        return SMT_MACHINE_OK;
    }

    retVal = func(context);
    if (SMT_ACTION_DONE != retVal)
    {
        /*
        * transition action fail is considered to be fatal
        * error
        */
        return SMT_MACHINE_ERROR_MACHINE_ERROR;
    }

    retVal = smtEnterState(machine, srcState, context);

    return retVal;
}

static smt_machineStatus_t
smtEnterStateMachine(smt_stateMachine_ptr_t   machine, void* context)
{
    smt_state_ptr_t state = NULL;
    smt_machineStatus_t retVal = SMT_MACHINE_OK;

    /* execute entry action */
    if (smtStateMachineHasEntryAction(machine))
    {
        smt_actionFuncPtr_t func = smtStateMachineGetEntryAction(machine);
        retVal = func(context);
        if (SMT_ACTION_DONE != retVal)
        {
            return SMT_MACHINE_ERROR_MACHINE_ERROR;
        }
    };

    state = smtMachineGetEntryState(machine);

    smtMachineSetActiveState(machine, NULL);

    return smtEnterState(machine, state, context);
}

/* leave smate machine */
static smt_machineStatus_t
smtExitStateMachine(smt_stateMachine_ptr_t  machine, void* context)
{
    smt_state_ptr_const_t state = NULL;
    smt_machineStatus_t retVal = SMT_MACHINE_OK;

    /* execute exit action */
    if (smtStateMachineHasExitAction(machine))
    {
        smt_actionFuncPtr_t func = smtStateMachineGetExitAction(machine);
        retVal = func(context);
        if (SMT_ACTION_DONE != retVal)
        {
            return SMT_MACHINE_ERROR_MACHINE_ERROR;
        }
    }

    smtMachineSetActiveState(machine, NULL);
}


static
void
smtMachineFinalize(smt_stateMachine_ptr_t   machine)
{
    //smtMachineSetAssDepth(machine, 0);
    smtEnterState(machine, &SMT_FINAL_STATE, NULL);
}



smt_machineStatus_t
smtMachineRun(
    smt_stateMachine_ptr_t  machine,
    smt_eventId_t       event,
    void*           context)
{
    smt_machineStatus_t retVal = SMT_MACHINE_OK;

    if (smtMachineIsFinalized(machine))
    {
        return SMT_MACHINE_GARBAGE_EVENT;
    }

    //retVal = smtHandleEvent(machine, event, context);
    if (retVal >= SMT_MACHINE_ERROR_FATAL)
    {
        smtMachineFinalize(machine);
    }

    return retVal;
}


static
smt_machineStatus_t
smtHandleEvent(
    smt_stateMachine_ptr_t  machine,
    smt_eventId_t       event,
    void*           context)
{
    smt_state_ptr_const_t   srcState = smtMachineGetActiveState(machine);
    smt_machineStatus_t retVal = SMT_MACHINE_OK;
    smt_trigger_t   *triggerTable = machine->triggerTable;
    smt_trigger_t   *trigger = NULL;
    int i;


    /* check current state, make sure we are not working on pseudo final state */
    if (smtIsStateFinal(srcState))
    {
        /* error: cannot process normal event on final state */
        return SMT_MACHINE_ERROR_MACHINE_ERROR;
    }

    /* check event, make sure we do not respond to garbage event */
    if (((int)event) >= smtMachineGetEventCount(machine))
    {
        return SMT_MACHINE_GARBAGE_EVENT;
    }

    if (!triggerTable)
    {
        return SMT_MACHINE_ERROR_MACHINE_ERROR;
    }

    /* find trigger */
    trigger = smtMachineFindTrigger(machine, event);

    /* Process transitions */
    for(i = 0; i < trigger->transitionNum; i++)
    {
        /* process transition */
        retVal = smtProcessTransition(machine, &(trigger->transitions[i]), context);
        if (SMT_MACHINE_OK != retVal)
        {
            break;
        }
    }

    return retVal;
}