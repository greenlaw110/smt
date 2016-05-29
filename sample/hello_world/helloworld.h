#ifndef HELLOWORLD_H
#define HELLOWORLD_H
/**
 * The hellowrold sample implement the following statemachine:
 * +--------------------------------------+
 * |                                      |
 * |   hello world state machine          |
 * |                                      |
 * |                                      |
 * |    +-------------+       +-----+     |
 * |    |             |  bye  |     |     |
 * |    |   active    +------>+ end |     |
 * |    |             |       |     |     |
 * |    +---+----+----+       +-----+     |
 * |        |    ^                        |
 * |        |    |                        |
 * |        +----+                        |
 * |          hi (print "hello world")    |
 * |                                      |
 * +--------------------------------------+
 */
#include "smt.h"
#include "smt_defs.h"

/* event id definitions */
typedef enum {
    HW_EVENT_HI,
    HW_EVENT_BYE
} helloworld_event_id_t;

/* state ID definitions */
typedef enum {
    HW_STATE_ACTIVE
} helloworld_state_id_t;


extern smt_stateMachine_t helloworldStateMachine;

#endif /* define HELLOWORLD_H */
