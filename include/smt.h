#ifndef SM_H
#define SM_H

#include <stdlib.h>
#include "smt_defs.h"

struct smt_stateMachine;

/* -----------------------------------------------------------------------
 * StateMachine Tools Header File
 * -----------------------------------------------------------------------
 * Terms & acronyms
 * - SM: statemachine, encapsulate a set of state and event triggers
 * - SMHL: statemachine hierarchy layer, one hierarchical layer of an
 *   SM
 * - state, define a specific status of an SMHL. A state can link to a
 *   SSM
 * - SSM: sub statemachine, an SM attached to a certain state of the
     parent SM
 * Limits
 * - Unable to check malformed statemachine such as un-ended transition route.
 * - Maximum number event types in one statemachine (including sub machines)
 is 255 including one predefined event: SMT_SHUTDOWN_EVENT
 * - Maximum state in one statemachine (not include sub machines) is 255
 including three pseudo states
 * - Transition cannot across state hierarchy.
 * ----------------------------------------------------------------------*/

/* count the number of element */
typedef UINT8 smt_counter_t;

/* identify event in a single statemachine hierarchy */
typedef smt_counter_t smt_eventId_t;

/* identify state in a single statemachine hierarchy */
typedef smt_counter_t smt_stateId_t;

/* reserved: final state ID */
#define SMT_FINAL_STATE_ID ((smt_stateId_t)0XFF)

/* reserved: history state ID */
#define SMT_HISTORY_STATE_ID ((smt_stateId_t)0XFE)

/* reserved: entry state ID */
#define SMT_ENTRY_STATE_ID ((smt_stateId_t)0XFD)

/*
 * reserved: shutdown ID
 * when statemachine encountered shutdown event
 * it transit to final state immediately
 */
#define SMT_SHUTDOWN_EVENT ((smt_eventId_t)0XFF)

/*
 * Options hints for choosing transition lookup 
 * table data structure
 */
typedef enum {
  SMT_OPTIMIZE_TIME,
  SMT_OPTIMIZE_SPACE,
  SMT_OPTIMIZE_AUTO
} smt_lookup_table_optimize_hint_t;

/* the guard function pointer */
typedef boolean (*smt_guardFuncPtr_t)(/*@null@*/ /*@unused@*/ void* context);

/* the destructor function pointer to be called after destroy a statemachine */
typedef void (*smt_destructorFuncPtr_t)(/*@null@*/ /*@unused@*/ void* context);

/* action return status */
typedef enum {
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
  SMT_ACTION_DONE, /* action complete successfully */
  SMT_ACTION_FAIL, /* action cannot be completed */
  SMT_ACTION_ERROR /* error occurred executing action */
} smt_actionReturnStatus_t;

/*
 * entry/exit action function pointer
 * -------------------------------------
 * @param context the context
 * @return return status
 */
typedef smt_actionReturnStatus_t (*smt_actionFuncPtr_t)(void* context);

/* the state structure */
typedef struct smt_state {
  const smt_stateId_t id;

  /* Note a final/history state will not have following attributes */
  /*@null@*/ const struct smt_stateMachine* const subStateMachine;

  /*
   * Note:
   * if entry Action return FAIL or ERROR
   * then SMT_MACHINE_ERROR_MACHINE_ERROR will be rasied
   * the state machine shutdown immediately.
   * The rationale behind this process is that entry action
   * fail will prevent state machine being stay on a stable
   * status
   */
  /*@null@*/ smt_actionFuncPtr_t entryAction; /* NULL means no entry action */
  /*@null@*/ smt_actionFuncPtr_t exitAction;  /* NULL means no exit action */
} smt_state_t;

typedef const smt_state_t* smt_state_ptr_const_t;

typedef smt_state_t* smt_state_ptr_t;

/*
 * transition action function pointer
 * ------------------------------------
 * @param context the context
 * @param targetState allow the transaction function to specify
 *        a different target state based on the logic
 * @return return status
 */
typedef smt_actionReturnStatus_t (*smt_transActionFuncPtr_t)(
    void* context, smt_state_ptr_const_t* targetState);

extern smt_state_t SMT_FINAL_STATE;
extern smt_state_t SMT_HISTORY_STATE;

/*
 * a transition is consist of
 * - event id
 * - source state id
 * - target state id
 * - guard function pointer
 * - transition function pointer
 * - restoreHistory flag - once set then it shall transit into
 *   the target state's linked sub statemachine's history state
 */
typedef struct {
  const smt_eventId_t event;
  smt_state_ptr_const_t const sourceState;
  smt_state_ptr_const_t const targetState;
  /*@null@*/ const smt_guardFuncPtr_t guard;
  /*@null@*/ const smt_transActionFuncPtr_t action;
  boolean restoreHistory;
} smt_transition_t;

typedef const smt_transition_t* smt_transition_ptr_const_t;

/* type definition for state machine */
typedef struct smt_stateMachine {
  smt_state_ptr_const_t* const stateList;
  const smt_counter_t stateCount;
  smt_transition_ptr_const_t transitionList;
  const smt_counter_t transitionCount;
  const smt_destructorFuncPtr_t destructor;
  /* placeholder for internal data */
  void* internalData;
} smt_stateMachine_t;

typedef smt_stateMachine_t* const smt_stateMachine_ptr_t;

typedef enum {
  SMT_MACHINE_OK,
  SMT_MACHINE_ERROR_UNKNOWN,
  SMT_MACHINE_ERROR_ACTION_ERROR,

  /* if fatal error encountered, the machine shutdown immediately */
  SMT_MACHINE_ERROR_FATAL,
  SMT_MACHINE_ERROR_MACHINE_ERROR = SMT_MACHINE_ERROR_FATAL
} smt_machineStatus_t;

/* statemachine initializer */
smt_machineStatus_t smtMachineInit(smt_stateMachine_ptr_t machine, smt_lookup_table_optimize_hint_t optimizeHint,  void* context);

/* statemachine entry */
smt_machineStatus_t smtMachineRun(smt_stateMachine_ptr_t machine,
                                  smt_eventId_t event, void* context);

/* statemachine terminator */
void smtMachineDestroy(smt_stateMachine_ptr_t machine, void* context);

/* check if a statemachine reached a final state */
boolean smtIsMachineFinalized(smt_stateMachine_ptr_t machine);

#endif
