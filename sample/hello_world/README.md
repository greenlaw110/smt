# SMT - Hello world Demo

Demonstrate how to use SMT implement a simple helloworld app

## Build Hello world Demo

SMT use [CMake](https://cmake.org/) as default build system. Below is the steps to build and run the demo

```
cd smt/sample/hello_world
mkdir build
cd build
cmake ..
make
./helloworld
```

The last line run the `helloworld` demo app


## Inside Helloworld Demo

This demo implement a simple statemachine as shown below:

```
 +--------------------------------------+
 |                                      |
 |   hello world state machine          |
 |                                      |
 |                                      |
 |    +-------------+       +-----+     |
 |    |             |  bye  |     |     |
 |    |   active    +------>+ end |     |
 |    |             |       |     |     |
 |    +---+----+----+       +-----+     |
 |        |    ^                        |
 |        |    |                        |
 |        +----+                        |
 |          hi (print "hello world")    |
 |                                      |
 +--------------------------------------+
```

Define the event enumerations:

```c
/* event id definitions */
typedef enum {
    HW_EVENT_HI,
    HW_EVENT_BYE
} helloworld_event_id_t;
```

Define the state enumerations:

```c
/* state ID definitions */
typedef enum {
    HW_STATE_ACTIVE
} helloworld_state_id_t;
```

Declare the statemachine:

```c
extern smt_stateMachine_t helloworldStateMachine;
```

Define the action function triggered on `HW_EVENT_HI` event when state is on `HW_STATE_ACTIVE` state:

```c
static smt_actionReturnStatus_t sayHelloWorld(void* context, smt_state_ptr_const_t* targetState)
{
    printf("hello world\n");
    return SMT_ACTION_DONE;
}
```

Define the exit action of `HW_STATE_ACTIVE` state:

```c
static smt_actionFuncPtr_t sayBye(void* context)
{
    printf("bye world\n");
    return SMT_ACTION_DONE;
}
```

Define the states:

```c
static smt_state_t state_Active = { .id = HW_STATE_ACTIVE, .exitAction = &sayBye };
```

Add all states into state list:

```c
static smt_state_ptr_const_t stateList[] = { &state_Active };
```

Define the transitions:

```c
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
```

Define the statemachine:

```
smt_stateMachine_t helloworldStateMachine = {
    stateList, /* state list */
    1, /* state list size */
    &transitionList, /* transition list */
    sizeof(transitionList) / sizeof(smt_transition_t), /* transition list size */
    NULL
};
```

Define a help function:

```c
void help() {
    printf("\navailable commands: ");
    printf("\n\t0|hi - say hi to the world");
    printf("\n\t1|bye - say bye to the world");
    printf("\n\t255|shutdown - shutdown machine");
}
```

Implement the main method:

```
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
```


