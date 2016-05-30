#ifndef MW_MACHINE_H
#define MW_MACHINE_H

#include "smt_defs.h"
#include "smt.h"

/* ---------------------------------------
 * First level states 
 * --------------------------------------*/

/* state id */
typedef enum L1_STATES 
{
    DOOR_CLOSED,
    DOOR_OPEN
} mw_state_id_t;

/* event id */
typedef enum MW_EVENTS
{
    OPEN_DOOR,
    CLOSE_DOOR,
    TURN_OFF,
    BAKE,
    TOAST
} mw_event_id_t;

extern smt_stateMachine_t MicrowaveStateMachine;

#endif /* define MW_MACHINE_H */