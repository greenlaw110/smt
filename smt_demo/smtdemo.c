#include "defs.h"
#include "smt_hl.h"
#include <stdio.h>
#include <stdlib.h>

/* -------------- actions --------------------- */
static
smt_actionReturnStatus_t 
h_l_a_active_Entry(void* context)
{
	horn_lights_adb_t* adb = (horn_lights_adb_t*)context;
	adb->rearm_option = TRUE;
	adb->alarmed_flag = FALSE;
	printf("ADB initalized.\n");
	printf("Timers initialized.\n");
	return SMT_ACTION_DONE;
}

static
smt_actionReturnStatus_t 
h_l_a_active_Exit(void* context)
{
	printf("All display released.\n");
	printf("All timers released.\n");
	printf("ADB freed.\n");
	return SMT_ACTION_DONE;
}

static
smt_actionReturnStatus_t 
h_l_a_TurnOnAlarms(void* context, int * targetState)
{
	horn_lights_adb_t* adb = (horn_lights_adb_t*) context;
	adb->num_req++;
	h_l_debugTurnOnAlarms(adb);
	return SMT_ACTION_DONE;
}

static
smt_actionReturnStatus_t
h_l_a_TurnOffAlarms(void* context, int * targetState)
{
	horn_lights_adb_t* adb = (horn_lights_adb_t*) context;
	if (0 == --(adb->num_req))
	{
		* targetState = HL_S_OPERATIONAL_END;
	}
	h_l_debugTurnOffAlarms(adb);
	return SMT_ACTION_DONE;
}

static
smt_actionReturnStatus_t
h_l_a_operational_Exit(void* context)
{
	horn_lights_adb_t* adb = (horn_lights_adb_t*) context;
	adb->num_req = 0;
	return SMT_ACTION_DONE;
}

static
smt_actionReturnStatus_t
h_l_a_delay_Entry(void* context)
{
	horn_lights_adb_t* adb = (horn_lights_adb_t*) context;
	adb->alarmed_flag = FALSE;
	printf("delay timer started\n");
	return SMT_ACTION_DONE;
}

static
smt_actionReturnStatus_t
h_l_a_delay_Exit(void* context)
{
	printf("delay timer stoped\n");
	return SMT_ACTION_DONE;
}

static
smt_actionReturnStatus_t
h_l_a_alarmed_Entry(void* context)
{
	horn_lights_adb_t* adb = (horn_lights_adb_t*) context;
	printf("alarm timer started\n");
	printf("alarm indicator turned on\n");
	adb->alarmed_flag = TRUE;
	return SMT_ACTION_DONE;
}

static
smt_actionReturnStatus_t
h_l_a_alarmed_Exit(void* context)
{
	printf("alarm timer stopped\n");
	printf("alarm indicator turned off\n");
	return SMT_ACTION_DONE;
}

};




int main()
{
	int input = 0;
	smt_machineStatus_t status = smtMachineInit(&h_l_stateMachine, &hlAdb);
	if (SMT_MACHINE_OK != status)
	{
		printf("init machine failed.\n");
		return 1;
	}

	for (;;)
	{
		printf("\nhorn_lights_machine>");
		scanf("%d", &input);
		getchar();
		smtMachineRun(&h_l_stateMachine, input, &hlAdb);
		if (smtMachineIsFinalized(&h_l_stateMachine))
		{
			printf("machine shutdown\n");
			break;
		}
	}
	
	return 0;
}
