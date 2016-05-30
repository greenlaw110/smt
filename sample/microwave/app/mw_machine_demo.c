#include "mw_machine.h"
#include "mw_context.h"
#include <stdio.h>
#include <stdlib.h>

static mw_context_t context = {
    .statePath = "/",
    .lampOn = FALSE,
    .heaterOn = FALSE
};

void mw_debugContext(mw_context_t * context) {
    printf("\nMICROWAVE INFO\n\tstate: %s\n\tlamp: %s\n\theater: %s\n", context->statePath, context->lampOn ? "on" : "off", context->heaterOn ? "on" : "off");
}

void mw_help() {
    printf("\navailable commands: ");
    printf("\n\t0|open - open door");
    printf("\n\t1|close - close door");
    printf("\n\t2|off - turn off operation");
    printf("\n\t3|bake - bake");
    printf("\n\t4|toast - toast");
    printf("\n\t255|shutdown - shutdown machine");
}

int main() {
    char input[10];
    smt_machineStatus_t status = smtMachineInit(&MicrowaveStateMachine, &context);
    if (SMT_MACHINE_OK != status) {
        printf("init machine failed.\n");
        return 1;
    }
    printf("Microwave State Machine Initialized\n");
    mw_help();
    printf("\n");
    mw_debugContext(&context);

    for (;;) {
        printf("\nmicrowave>");
        int n = scanf("%s", input);
        if (n < 0) {
            printf("bad input");
            continue;
        }
        while(getchar() != '\n');
        int event = -1;
        if (!strcmp("open", input) || !strcmp("0", input)) {
            event = OPEN_DOOR;
        } else if (!strcmp("close", input) || !strcmp("1", input)) {
            event = CLOSE_DOOR;
        } else if (!strcmp("off", input) || !strcmp("2", input)) {
            event = TURN_OFF;
        } else if (!strcmp("bake", input) || !strcmp("3", input)) {
            event = BAKE;
        } else if (!strcmp("toast", input) || !strcmp("4", input)) {
            event = TOAST;
        } else if (!strcmp("shutdown", input) || !strcmp("exit", input) || !strcmp("quit", input) || !strcmp("bye", input) || !strcmp("255", input)) {
            event = SMT_SHUTDOWN_EVENT;
        } else if (!strcmp("help", input)) {
            mw_help();
            printf("\n");
            mw_debugContext(&context);
            continue;
        } else {
            printf("unknown command");
            mw_help();
            continue;
        }
        smt_machineStatus_t status = smtMachineRun(&MicrowaveStateMachine, event, &context);
        if (SMT_MACHINE_ERROR_UNKNOWN == status) {
        	printf("command not applied to the current state");
        } else if (smtIsMachineFinalized(&MicrowaveStateMachine)) {
        	smtMachineDestroy(&MicrowaveStateMachine, &context);
            printf("machine shutdown\n");
            break;
        }
        mw_debugContext(&context);
    }

    return 0;
}

