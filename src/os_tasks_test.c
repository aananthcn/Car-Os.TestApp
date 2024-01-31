#include <stdio.h>
#include <string.h>

#include <osek.h>
#include <os_api.h>

#include <osek_sg.h>
#include <board.h>

#include <Dio.h>
#include <Spi.h>

#include <Eth.h>
#include <macphy.h>

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(namma_test_app, LOG_LEVEL_NONE);

// #define OSEK_TASK_DEBUG	1
//#define SCHEDULE_TEST
//#define TERMINATE_TASK_TEST
//#define CHAINTASK_TEST

#define GETEVENT_TEST
#define EVENT_SET_CLEAR_TEST
//#define GET_RELEASE_RESOURCE_TEST

// #define ENC28J60_DEBUG	1
//#define ETH_LOW_LEVEL_SEND_RECV_TEST 0


/*#############*/
// this block of code is writte only for testing purpose, doesn't mean that
// these can be used public or application code.
#include <os_task.h>
/*#############*/




TASK(Task_A) {
	static bool toggle_bit;

#if ALARM_BASE_TEST
	AlarmBaseType info;
#endif
#if GET_ALARM_TEST
	TickType tick_left;
#endif
#if SET_REL_ALARM_TEST
	static bool cycle_started = false;
#endif

#ifdef ALARM_BASE_TEST
	if (E_OK == GetAlarmBase(0, &info))
		LOG_DBG("0: ticks/base = %d", info.ticksperbase);
	if (E_OK == GetAlarmBase(1, &info))
		LOG_DBG("1: ticks/base = %d", info.ticksperbase);
	if (E_OK == GetAlarmBase(2, &info))
		LOG_DBG("2: ticks/base = %d", info.ticksperbase);
#endif

#ifdef GET_ALARM_TEST
	if (E_OK == GetAlarm(0, &tick_left))
		LOG_DBG("0: ticks remaining = %d", tick_left);
#endif

#ifdef SET_REL_ALARM_TEST
	if (!cycle_started) {
		SetRelAlarm(0, 250, 2000);
		cycle_started = true;
	}
#endif

#ifdef SET_ABS_ALARM_TEST
	SetAbsAlarm(1, 5000, 1500); // start at 5th sec and repeat every 1.5 sec
#endif

#ifdef CANCEL_ALARM_TEST
	SetAbsAlarm(1, 1, 1); // start at 5th sec and repeat every 1.5 sec
#endif



#ifdef GETEVENT_TEST

#ifdef TERMINATE_TASK_TEST
	TerminateTask();
#endif

#ifdef WAITEVENT_TEST
	WaitEvent(1);
#endif

#ifdef CHAINTASK_TEST
	ChainTask(2);
#endif

#ifdef SCHEDULE_TEST
	Schedule();
#endif

#ifdef GETTASKID_GETTASKSTATE_TEST
	TaskType task;
	TaskStateType state;
	GetTaskID(&task);
	GetTaskState(task, &state);
	LOG_DBG("Current task: %d, state = %d", task, state);
	task = 1;
	GetTaskState(task, &state);
	LOG_DBG("Task: %d, state = %d", task, state);
	task = 2;
	GetTaskState(task, &state);
	LOG_DBG("Task: %d, state = %d", task, state);
#endif

#ifdef OSEK_TASK_DEBUG
	EventMaskType Event = 0;
#endif

#ifdef ENABLE_DISABLE_ISR_TEST
	DisableAllInterrupts();
	LOG_DBG("Enable / Disable ISR test");
	EnableAllInterrupts();
#endif

	if (toggle_bit) {
		Dio_WriteChannel(16, STD_HIGH);
		toggle_bit = false;
#ifdef OSEK_TASK_DEBUG
		LOG_DBG("Task A: Triggered event for Task B");
#endif
#ifdef OSEK_TASK_DEBUG
		LOG_DBG("Task A: Event = 0x%016X", Event);
#endif
		#ifdef GET_RELEASE_RESOURCE_TEST
		LOG_DBG("Task A Priority = %d", _OsTaskDataBlk[_OsCurrentTask.id].ceil_prio);
		ReleaseResource(RES(mutex1));
		LOG_DBG("Task A Priority = %d", _OsTaskDataBlk[_OsCurrentTask.id].ceil_prio);
		#endif
	}
	else {
		Dio_WriteChannel(16, STD_LOW);
		SetEvent(TASK_TASK_C_ID, 0x101);
		toggle_bit = true;
		LOG_DBG("Task A toggle_bit set");
#ifdef GET_RELEASE_RESOURCE_TEST
		LOG_DBG("Task A Priority = %d", _OsTaskDataBlk[_OsCurrentTask.id].ceil_prio);
		GetResource(RES(mutex1));
		LOG_DBG("Task A Priority = %d", _OsTaskDataBlk[_OsCurrentTask.id].ceil_prio);
#endif
	}
#endif
}



TASK(Task_B) {
#ifdef CANCEL_ALARM
	static int i = 10;
	if (i-- <= 0) 
		CancelAlarm(1);
#endif
#ifdef EVENT_SET_CLEAR_TEST
	EventMaskType Event = 0;
	GetEvent(TASK_TASK_B_ID, &Event);
	ClearEvent(1);
#endif
#ifdef GETEVENT_TEST

#ifdef WAITEVENT_TEST
	WaitEvent(2);
#endif
#endif // GETEVENT_TEST
	static bool toggle_bit;
	if (toggle_bit) {
		Dio_WriteChannel(17, STD_HIGH);
		toggle_bit = false;
	}
	else {
		Dio_WriteChannel(17, STD_LOW);
		toggle_bit = true;
		LOG_DBG("Task B toggle_bit set");
	}

}



TASK(Task_C) {
#ifdef EVENT_SET_CLEAR_TEST
	uint32_t Event = 1;
	static int setcnt = 4;
	if (setcnt > 0) {
		setcnt--;
	}
	else {
		SetEvent(TASK_TASK_B_ID, &Event);
		setcnt = 4;
	}
#endif
	Dio_FlipChannel(18);
	LOG_DBG("Task C called!");
}



void Alarm_uSecAlarm_callback(void) {
	LOG_DBG("Alarm_uSecAlarm_callback() got called!");
}


TASK(Task_D) {
	Dio_FlipChannel(19);
	LOG_DBG("Task D called!");
}
