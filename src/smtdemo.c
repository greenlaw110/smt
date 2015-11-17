#include "defs.h"
#include "smt.h"
#include <stdio.h>
#include <stdlib.h>


/* ADB */
typedef struct
{
   appId_t		app_id;
   boolean		rearm_option;
   boolean		alarmed_flag;
   UINT8		num_req;		/* This is the number of requests to turn on*/
} horn_lights_adb_t;

static horn_lights_adb_t hlAdb;

/* STATE IDS */
typedef enum
{
	/* 1 */ 	HL_S_ACTIVE,
	/* 2 */     HL_S_ACTIVE_END,
	/* 3 */ 	HL_S_ARMED,
	/* 4 */ 	HL_S_OPERATIONAL,
	/* 5 */ 	HL_S_DELAY,
	/* 6 */ 	HL_S_ALARMED,
	/* 7 */ 	HL_S_OPERATIONAL_END
} horn_lights_state_t;

/* EVENT IDS */
typedef enum
{
	/* 0 */ 	AM_POLITE_SHUTDOWN,
	/* 1 */ 	AM_IMPOLITE_SHUTDOWN,
	/* 2 */ 	H_L_INPUT_H_L_BUTTON,
	/* 3 */ 	H_L_INPUT_TURN_OFF_ALARMS,
	/* 4 */ 	H_L_INPUT_TURN_ON_ALARMS,
	/* 5 */ 	H_L_INPUT_ANY_KEY_PRESS,
	/* 6 */ 	H_L_INPUT_DELAY_TIMER_EXPIRED,
	/* 7 */ 	H_L_INPUT_ALARMS_TIMER_EXPIRED,
	/* 8 */		H_L_INVALID_EVENT
} horn_lights_event_t;

/* --------------- utilities ---------------*/

static
void 
h_l_debugTurnOnAlarms(horn_lights_adb_t* adb)
{
	printf("Turn on Alarms request received. total number of requests:%d\n", adb->num_req);
}

static
void 
h_l_debugTurnOffAlarms(horn_lights_adb_t* adb)
{
	printf("Turn off Alarms request received. total number of requests:%d\n", adb->num_req);
}

/* -------------- guards ---------------------- */
static
boolean
h_l_g_operational_To_Armed(void* context)
{
	horn_lights_adb_t* adb = (horn_lights_adb_t*)context;
	return (adb->rearm_option || !adb->alarmed_flag);
}

static
boolean
h_l_g_operational_To_End(void* context)
{
	return !h_l_g_operational_To_Armed(context);
}

static
boolean
h_l_g_req_num_not_zero(void* context)
{
	horn_lights_adb_t* adb = (horn_lights_adb_t*)context;
	/* 
	 * use 1 instead of 0 cause this is a precondition guard
	 * we need to take the transition action effect into
	 * consideration 
	 */
	return (adb->num_req > 1);
}

static
boolean
h_l_g_req_num_zero(void* context)
{
	return !h_l_g_req_num_not_zero(context);
}

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

/* -------------- STATE DEFINITIONS ------------------ */
const static smt_state_t h_l_s_ActiveEnd =
{
	/* stateId */		HL_S_ACTIVE_END,
	/* isFinal */		TRUE,
	/* Entry */			NULL,
	/* Exit */			NULL,
	/* InitSub */		NULL
};

const static smt_state_t h_l_s_OperationalEnd =
{
	/* stateId */		HL_S_OPERATIONAL_END,
	/* isFinal */		TRUE,
	/* Entry */			NULL,
	/* Exit */			NULL,
	/* InitSub */		NULL
};

const static smt_state_t h_l_s_Delay =
{
	/* stateId */		HL_S_DELAY,
	/* isFinal */		FALSE,
	/* Entry */			h_l_a_delay_Entry,
	/* Exit */			h_l_a_delay_Exit,
	/* InitSub */		NULL
};

const static smt_state_t h_l_s_Alarmed =
{
	/* stateId */		HL_S_ALARMED,
	/* isFinal */		FALSE,
	/* Entry */			h_l_a_alarmed_Entry,
	/* Exit */			h_l_a_alarmed_Exit,
	/* InitSub */		NULL
};

const static smt_state_t h_l_s_Armed =
{
	/* stateId */		HL_S_ARMED,
	/* isFinal */		FALSE,
	/* Entry */			NULL,
	/* Exit */			NULL,
	/* InitSub */		NULL
};

const static smt_state_t h_l_s_Operational =
{
	/* stateId */		HL_S_OPERATIONAL,
	/* isFinal */		FALSE,
	/* Entry */			NULL,
	/* Exit */			h_l_a_operational_Exit,
	/* InitSub */		&h_l_s_Delay
};

const static smt_state_t h_l_s_Active =
{
	/* stateId */		HL_S_ACTIVE,
	/* isFinal */		FALSE,
	/* Entry */			h_l_a_active_Entry,
	/* Exit */			h_l_a_active_Exit,
	/* InitSub */		&h_l_s_Armed
};

/* -------------- TRANSITION DEFINITIONS ------------------ */
const static smt_transition_t h_l_t_Armed_On_TurnOnAlarms[] =
{
	{
		/* guard */		NULL,
		/* action */	h_l_a_TurnOnAlarms,
		/* target */	&h_l_s_Operational,
		/* internal */	FALSE
	}
};
const static smt_transitionGroup_t	h_l_tg_Armed_On_TurnOnAlarms = 
{
	/* transition */	h_l_t_Armed_On_TurnOnAlarms,
	/* number */		1
};

const static smt_transition_t h_l_t_Operational_On_TurnOnAlarms[] =
{
	{
		/* guard */		NULL,
		/* action */	h_l_a_TurnOnAlarms,
		/* target */	&h_l_s_Operational,
		/* internal */	TRUE
	}
};
const static smt_transitionGroup_t	h_l_tg_Operational_On_TurnOnAlarms = 
{
	/* transition */	h_l_t_Operational_On_TurnOnAlarms,
	/* number */		1
};

const static smt_transition_t h_l_t_Operational_On_AnyKeyPress[] =
{
	{
		/* guard */		h_l_g_operational_To_Armed,
		/* action */	NULL,
		/* target */	&h_l_s_Armed,
		/* internal */	FALSE
	},
	{
		/* guard */		h_l_g_operational_To_End,
		/* action */	NULL,
		/* target */	&h_l_s_ActiveEnd,
		/* internal */	FALSE
	}
};
const static smt_transitionGroup_t	h_l_tg_Operational_On_AnyKeyPress = 
{
	/* transition */	h_l_t_Operational_On_AnyKeyPress,
	/* number */		2
};

const static smt_transition_t h_l_t_Delay_On_TimerExpired[] =
{
	{
		/* guard */		NULL,
		/* action */	NULL,
		/* target */	&h_l_s_Alarmed,
		/* internal */	FALSE
	}
};
const static smt_transitionGroup_t	h_l_tg_Delay_On_TimerExpired = 
{
	/* transition */	h_l_t_Delay_On_TimerExpired,
	/* number */		1
};

const static smt_transition_t h_l_t_Delay_On_TurnOffAlarms[] =
{
	{
		/* guard */		NULL, /* h_l_g_req_num_not_zero,*/
		/* action */	h_l_a_TurnOffAlarms,
		/* target */	&h_l_s_Delay,
		/* internal */	TRUE
	},
	{
		/* guard */		NULL, /* h_l_g_req_num_zero,*/
		/* action */	h_l_a_TurnOffAlarms,
		/* target */	&h_l_s_OperationalEnd,
		/* internal */	FALSE
	}
};
const static smt_transitionGroup_t	h_l_tg_Delay_On_TurnOffAlarms = 
{
	/* transition */	h_l_t_Delay_On_TurnOffAlarms,
	/* number */		2
};

const static smt_transition_t h_l_t_Alarmed_On_TimerExpired[] =
{
	{
		/* guard */		NULL,
		/* action */	NULL,
		/* target */	&h_l_s_OperationalEnd,
		/* internal */	FALSE
	}
};
const static smt_transitionGroup_t	h_l_tg_Alarmed_On_TimerExpired = 
{
	/* transition */	h_l_t_Alarmed_On_TimerExpired,
	/* number */		1
};

const static smt_transition_t h_l_t_Alarmed_On_TurnOffAlarms[] =
{
	{
		/* guard */		h_l_g_req_num_not_zero,
		/* action */	h_l_a_TurnOffAlarms,
		/* target */	&h_l_s_Alarmed,
		/* internal */	TRUE
	},
	{
		/* guard */		h_l_g_req_num_zero,
		/* action */	h_l_a_TurnOffAlarms,
		/* target */	&h_l_s_OperationalEnd,
		/* internal */	FALSE
	}
};
const static smt_transitionGroup_t	h_l_tg_Alarmed_On_TurnOffAlarms = 
{
	/* transition */	h_l_t_Alarmed_On_TurnOffAlarms,
	/* number */		2
};

const static smt_transition_t h_l_t_Active_Final[] = 
{
	{
		/* guard */		NULL,
		/* action */	NULL,
		/* target */	&SMT_FINAL_STATE,
		/* internal */	FALSE
	}
};
const static smt_transitionGroup_t	h_l_tg_Active_Final = 
{
	/* transition */	h_l_t_Active_Final,
	/* number */		1
};

/* ------------------- EVENTs ------------------------ */
const static smt_trigger_t _h_l_tr_On_TurnOnAlarms[] =
{
	/*state 					transitionGroup 	*/
	{&h_l_s_Armed,				&h_l_tg_Armed_On_TurnOnAlarms},
	{&h_l_s_Operational,		&h_l_tg_Operational_On_TurnOnAlarms}
};
const static smt_eventData_t h_l_e_On_TurnOnAlarms =
{
	/* trigger list */ 	_h_l_tr_On_TurnOnAlarms,
	/* number */		2
};

const static smt_trigger_t _h_l_tr_On_TurnOffAlarms[] =
{
	/*state 					transitionGroup 	*/
	{&h_l_s_Delay,				&h_l_tg_Delay_On_TurnOffAlarms},
	{&h_l_s_Alarmed,			&h_l_tg_Alarmed_On_TurnOffAlarms}
};
const static smt_eventData_t h_l_e_On_TurnOffAlarms =
{
	/* trigger list */ 	_h_l_tr_On_TurnOffAlarms,
	/* number */		2
};

const static smt_trigger_t _h_l_tr_On_DelayTimerExpire[] =
{
	/*state 					transitionGroup 	*/
	{&h_l_s_Delay,				&h_l_tg_Delay_On_TimerExpired},
};
const static smt_eventData_t h_l_e_On_DelayTimerExpire =
{
	/* trigger list */ 	_h_l_tr_On_DelayTimerExpire,
	/* number */		1
};

const static smt_trigger_t _h_l_tr_On_AlarmTimerExpire[] =
{
	/*state 					transitionGroup 	*/
	{&h_l_s_Alarmed,			&h_l_tg_Alarmed_On_TimerExpired},
};
const static smt_eventData_t h_l_e_On_AlarmTimerExpire =
{
	/* trigger list */ 	_h_l_tr_On_AlarmTimerExpire,
	/* number */		1
};

const static smt_trigger_t _h_l_tr_On_AnyKeyPress[] =
{
	/*state 					transitionGroup 	*/
	{&h_l_s_Operational,		&h_l_tg_Operational_On_AnyKeyPress},
};
const static smt_eventData_t h_l_e_On_AnyKeyPress =
{
	/* trigger list */ 	_h_l_tr_On_AnyKeyPress,
	/* number */		1
};

const static smt_trigger_t _h_l_tr_On_CommonShutdown[] =
{
	/*state 					transitionGroup 	*/
	{&h_l_s_Active,				&h_l_tg_Active_Final},
};
const static smt_eventData_t h_l_e_On_CommonShutdown =
{
	/* trigger list */ 	_h_l_tr_On_CommonShutdown,
	/* number */		1
};

const static smt_eventData_ptr_t _h_l_e_list[] =
{
	/* AM_POLITE_SHUTDOWN */			&h_l_e_On_CommonShutdown,
	/* AM_IMPOLITE_SHUTDOWN */			&h_l_e_On_CommonShutdown,
	/* H_L_INPUT_H_L_BUTTON */			&h_l_e_On_CommonShutdown,
	/* H_L_INPUT_TURN_OFF_ALARMS */		&h_l_e_On_TurnOffAlarms,
	/* H_L_INPUT_TURN_ON_ALARMS */		&h_l_e_On_TurnOnAlarms,
	/* H_L_INPUT_ANY_KEY_PRESS */		&h_l_e_On_AnyKeyPress,
	/* H_L_INPUT_DELAY_TIMER_EXPIRED */	&h_l_e_On_DelayTimerExpire,
	/* H_L_INPUT_ALARMS_TIMER_EXPIRED */&h_l_e_On_AlarmTimerExpire
};

/* ------------------------ STATE MACHINE ----------------------- */
static smt_stateMachine_t h_l_stateMachine = 
{
	/* entry state */		&h_l_s_Active,
	/* event list */		_h_l_e_list,
	/* event count */		(sizeof(_h_l_e_list)/sizeof(_h_l_e_list[0])),
	/* completion event */	&h_l_e_On_AnyKeyPress,
	/* activeStateSeq */	{NULL, NULL, NULL, NULL},
	/* assDepth */			0
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
