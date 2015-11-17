#ifndef SM_HL_H
#define SM_HL_H

#include <stdio.h>

#include "defs.h"
#include "smt.h"

smt_eventHandler_t op_sm_eventHandler;
smt_eventHandler_t HL_sm_eventHandler;
smt_eventHandler_t act_sm_eventHandler;

extern smt_actionFuncPtr_t h_l_a_active_Entry;
extern smt_actionFuncPtr_t h_l_a_active_Exit;


extern smt_actionFuncPtr_t HL_sm_entryAction;
extern smt_actionFuncPtr_t HL_sm_exitAction;

extern smt_actionFuncPtr_t h_l_a_operational_Exit;
extern smt_actionFuncPtr_t h_l_a_delay_Entry;
extern smt_actionFuncPtr_t h_l_a_delay_Exit;
extern smt_actionFuncPtr_t h_l_a_alarmed_Entry;
extern smt_actionFuncPtr_t h_l_a_alarmed_Exit;
extern smt_actionFuncPtr_t h_l_a_TurnOnAlarms;
extern smt_actionFuncPtr_t h_l_a_TurnOffAlarms;


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


extern smt_stateMachine_t h_l_stateMachine;
extern smt_stateMachine_t SM_Active;
extern smt_stateMachine_t SM_Operational;


//=====================================================================================
// State machine definition of the horn light state machine
//=====================================================================================


/* -------------- STATE DEFINITIONS ------------------ */
static smt_state_t h_l_s_Active =
{
	/* stateId */		HL_S_ACTIVE,
	/* isFinal */		FALSE,
	/* is sub SM */		TRUE,
	/* parent SM */		&SM_Active,
	/* Entry */			NULL,
	/* Exit */			NULL
};

static smt_state_ptr_const_t smt_hl_state_table[] =
{
	&h_l_s_Active
};

/* -------------- TRANSITION DEFINITIONS ------------------ */
const static smt_transition_t smt_hl_transitions[] = 
{
	{
		/* guard */		NULL,
		/* action */	NULL,
		/* source */	&h_l_s_Active,
		/* target */	&SMT_FINAL_STATE
	}
};


/* ------------------- EVENTs ------------------------ */
	const int AM_POLITE_SHUTDOWN = 1001;
	const int AM_IMPOLITE_SHUTDOWN = 1002;
	const int H_L_INPUT_H_L_BUTTON = 1003;
	const int H_L_INPUT_TURN_OFF_ALARMS = 1004;
	const int H_L_INPUT_TURN_ON_ALARMS = 1005;
	const int H_L_INPUT_ANY_KEY_PRESS = 1006;
	const int H_L_INPUT_DELAY_TIMER_EXPIRED = 1007;
	const int H_L_INPUT_ALARMS_TIMER_EXPIRED = 1008;
	const int H_L_INVALID_EVENT = 1009;


/* ------------------------ STATE MACHINE ----------------------- */
static smt_stateMachine_t h_l_stateMachine = 
{
	/* state table */			smt_hl_state_table,
	/* state table size */		sizeof(smt_hl_state_table)/sizeof(smt_state_ptr_const_t),
	/* transition table */		smt_hl_transitions,
	/* transition table size */	sizeof(smt_hl_transitions)/sizeof(smt_transition_t),
	/* is sub state machine */	FALSE,
	/* parent state machine */	NULL,
	/* entry state */			&h_l_s_Active,
	/* event handler */			NULL,
	/* entry action */			NULL,
	/* exit action */			NULL
};



//=====================================================================================
// State machine definition of the "active" sub state machine of the 
// horn light state machine.
//=====================================================================================
/* -------------- STATE DEFINITIONS ------------------ */
static smt_state_t h_l_s_Operational =
{
	/* stateId */		HL_S_OPERATIONAL,
	/* isFinal */		FALSE,
	/* is sub SM */		TRUE,
	/* sub SM */		&SM_Operational,
	/* Entry */			NULL,
	/* Exit */			NULL,
};

static smt_state_t h_l_s_Armed =
{
	/* stateId */		HL_S_ARMED,
	/* isFinal */		FALSE,
	/* is sub SM */		FALSE,
	/* sub SM */		NULL,
	/* Entry */			NULL,
	/* Exit */			NULL,
};

static smt_state_ptr_const_t smt_act_state_table[] =
{
	&h_l_s_Operational,
	&h_l_s_Armed
};

/* -------------- TRANSITION DEFINITIONS ------------------ */
static smt_transition_t smt_act_transitions[] =
{
	{
		/* guard */		NULL,
		/* action */	NULL,
		/* source */	&h_l_s_Armed,
		/* target */	&h_l_s_Operational,
	},
	{
		/* guard */		NULL,
		/* action */	NULL,
		/* source */	&h_l_s_Operational,
		/* target */	&h_l_s_Armed,
	},
	{
		/* guard */		NULL,
		/* action */	NULL,
		/* source */	&h_l_s_Operational,
		/* target */	&SMT_FINAL_STATE
	}
};


/* ------------------- EVENTs ------------------------ */
	const int ACT_INPUT_TURN_OFF_ALARMS = 2001;
	const int ACT_INPUT_TURN_ON_ALARMS = 2002;
	const int ACT_INPUT_ANY_KEY_PRESS = 2003;


/* ------------------------ STATE MACHINE ----------------------- */
static smt_stateMachine_t SM_Active = 
{
	/* state table */			smt_act_state_table,
	/* state table size */		sizeof(smt_act_state_table)/sizeof(smt_state_ptr_const_t),
	/* transition table */		smt_act_transitions,
	/* transition table size */	sizeof(smt_act_transitions)/sizeof(smt_transition_t),
	/* is sub state machine */	TRUE,
	/* parent state machine */	&h_l_stateMachine,
	/* entry state */			&h_l_s_Armed,
	/* event handler */			NULL,
	/* entry action */			NULL,
	/* exit action */			NULL,
	/* current state */			-1
};



//=====================================================================================
// State machine definition of the "Operational" sub state machine of the 
// horn light state machine.
//=====================================================================================
/* -------------- STATE DEFINITIONS ------------------ */
const static smt_state_t h_l_s_Delay =
{
	/* stateId */		HL_S_DELAY,
	/* isFinal */		FALSE,
	/* is sub SM */		FALSE,
	/* sub SM */		NULL,
	/* Entry */			NULL,
	/* Exit */			NULL,
};

const static smt_state_t h_l_s_Alarmed =
{
	/* stateId */		HL_S_ALARMED,
	/* isFinal */		FALSE,
	/* is sub SM */		FALSE,
	/* sub SM */		NULL,
	/* Entry */			NULL,
	/* Exit */			NULL,
};

static smt_state_ptr_const_t smt_op_state_table[] =
{
	&h_l_s_Delay,
	&h_l_s_Alarmed
};

/* -------------- TRANSITION DEFINITIONS ------------------ */


const static smt_transition_t smt_op_transitions[] =
{
	{
		/* guard */		NULL,
		/* action */	NULL,
		/* source */	&h_l_s_Delay,
		/* target */	&h_l_s_Alarmed
	},
	{
		/* guard */		NULL,
		/* action */	NULL,
		/* source */	&h_l_s_Delay,
		/* target */	&SMT_FINAL_STATE
	},
	{
		/* guard */		NULL, 
		/* action */	NULL,
		/* source */	&h_l_s_Alarmed,
		/* target */	&SMT_FINAL_STATE
	}
};

/* ------------------- EVENTs ------------------------ */
	const int OP_INPUT_TURN_OFF_ALARMS = 3001;
	const int OP_INPUT_TURN_ON_ALARMS = 3002;
	//const int OP_INPUT_ANY_KEY_PRESS = 3003;
	const int OP_INPUT_DELAY_TIMER_EXPIRED = 3004;
	const int OP_INPUT_ALARMS_TIMER_EXPIRED = 3005;

/* ------------------------ STATE MACHINE ----------------------- */
static smt_stateMachine_t SM_Operational = 
{
	/* state table */			smt_op_state_table,
	/* state table size */		sizeof(smt_op_state_table)/sizeof(smt_state_ptr_const_t),
	/* transition table */		smt_op_transitions,
	/* transition table size */	sizeof(smt_op_transitions)/sizeof(smt_transition_t),
	/* is sub state machine */	TRUE,
	/* parent state machine */	&SM_Active,
	/* entry state */			&h_l_s_Alarmed,
	/* event handler */			NULL,
	/* entry action */			NULL,
	/* exit action */			NULL,
	/* current state */			-1
};






#endif //SM_HL_H