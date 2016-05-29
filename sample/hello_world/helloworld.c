#include "helloworld.h"
#include <stdio.h>
#include <stdlib.h>
/* -------------- actions --------------------- */
static smt_actionReturnStatus_t sayHelloWorld(void* context, smt_state_ptr_const_t* targetState)
{
    printf("hello world\n");
    return SMT_ACTION_DONE;
}

static smt_actionFuncPtr_t sayBye(void* context)
{
    printf("bye world\n");
    return SMT_ACTION_DONE;
}

/* -------------- data definitions ------------- */
static smt_state_t state_Active = { .id = HW_STATE_ACTIVE, .exitAction = &sayBye };

static smt_state_ptr_const_t stateList[] = { &state_Active };

static smt_transition_t transitionList[] = {
    { .event = HW_EVENT_HI, /* event */
      .sourceState = &state_Active, /* source state */
      .targetState = &state_Active, /* source state */
      .action = &sayHelloWorld /* transition function */
    },
    { .event = HW_EVENT_BYE, /* event */
      .sourceState = &state_Active, /* source state */
      .targetState = &SMT_FINAL_STATE /* target state */
    }
};

smt_stateMachine_t helloworldStateMachine = {
    stateList, /* state list */
    1, /* state list size */
    &transitionList, /* transition list */
    sizeof(transitionList) / sizeof(smt_transition_t), /* transition list size */
    NULL
};

int main()
{
    int input = 0;
    smt_machineStatus_t status = smtMachineInit(&helloworldStateMachine);
    if (SMT_MACHINE_OK != status) {
        printf("init machine failed.\n");
        return 1;
    }

    for (;;) {
        printf("\nhelloworld>");
        scanf("%d", &input);
        getchar();
        smt_machineStatus_t retVal = smtMachineRun(&helloworldStateMachine, input, NULL);
        if (smtIsMachineFinalized(&helloworldStateMachine)) {
            printf("world shutdown\n");
            break;
        }
        if (SMT_MACHINE_ERROR_UNKNOWN == retVal) {
            printf("unknown event\n");
        }
    }

    return 0;
}
