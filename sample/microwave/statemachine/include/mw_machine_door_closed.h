#ifndef MW_DOOR_CLOSED_H
#define MW_DOOR_CLOSED_H

#include "smt.h"

/* state id */
typedef enum DOOR_CLOSED_STATES 
{
    OFF,
    HEATING
} door_closed_state_id_t;

extern smt_stateMachine_t DoorClosedStateMachine;

#endif /* define MW_DOOR_CLOSED_H */