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

void help() {
    printf("\navailable commands: ");
    printf("\n\t0|hi - say hi to the world");
    printf("\n\t1|bye - say bye to the world");
    printf("\n\t255|shutdown - shutdown machine");
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
    char input[10];
    smt_machineStatus_t status = smtMachineInit(&helloworldStateMachine, NULL);
    if (SMT_MACHINE_OK != status) {
        printf("init machine failed.\n");
        return 1;
    }
    printf("Hello World Statemachine initialized");
    help();

    for (;;) {
        printf("\nhelloworld>");
        int n = scanf("%s", input);
        if (n < 0) {
            printf("bad input");
            continue;
        }
        while(getchar() != '\n');
        int event = -1;
        if (!strcmp("hi", input) || !strcmp("0", input)) {
            event = HW_EVENT_HI;
        } else if (!strcmp("bye", input) || !strcmp("1", input)) {
            event = HW_EVENT_BYE;
        } else if (!strcmp("shutdown", input) || !strcmp("exit", input) || !strcmp("quit", input) || !strcmp("bye", input) || !strcmp("255", input)) {
            event = SMT_SHUTDOWN_EVENT;
        } else if (!strcmp("help", input)) {
            help();
            continue;
        } else {
            printf("unknown command");
            help();
            continue;
        }
        smt_machineStatus_t retVal = smtMachineRun(&helloworldStateMachine, event, NULL);
        if (smtIsMachineFinalized(&helloworldStateMachine)) {
            smtMachineDestroy(&helloworldStateMachine, NULL);
            printf("world shutdown\n");
            break;
        }
        if (SMT_MACHINE_ERROR_UNKNOWN == retVal) {
            printf("unknown command\n");
        }
    }

    return 0;
}
