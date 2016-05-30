#ifndef MW_HEATING_H
#define MW_HEATING_H

#include "smt.h"

/* state id */
typedef enum HEATING_STATES 
{
    BAKING,
    TOASTING
} heating_state_id_t;

extern smt_stateMachine_t HeatingStateMachine;

#endif /* define MW_HEATING_H */