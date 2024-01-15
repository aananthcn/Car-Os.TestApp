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


#define ARP_PKT_SZ 14
void send_arp_pkt(void) {
	const uint8 brdcst_a[] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
	uint8 eth_data[ARP_PKT_SZ];
	int i, j;

	// destination mac address
	for (i = 0; i < 6; i++) {
		eth_data[i] = brdcst_a[i];
	}

	// source mac address
	for (j = 0; j < 6; j++) {
		eth_data[i+j] = EthConfigs[0].ctrlcfg.mac_addres[j];
	}
	i += j;

	eth_data[i++] = 0x08; // ARP type = 0x0806
	eth_data[i++] = 0x06; // ARP type = 0x0806

	macphy_pkt_send((uint8*)eth_data, ARP_PKT_SZ);
}


#define RECV_PKT_SZ	(1522)
void macphy_test(void) {
#if ETH_LOW_LEVEL_SEND_RECV_TEST
	uint16 phy_reg;
	uint16 rx_dlen;
	uint8 reg_data;
	uint8 eth_data[RECV_PKT_SZ];
#endif


#if ETH_DRIVER_MAX_CHANNEL > 0
 #if defined BITOPS_TEST
	// ECON1 register tests
	enc28j60_bitclr_reg(ECON1, 0x03);
	reg_data = enc28j60_read_reg(ECON1);
  #ifdef ENC28J60_DEBUG
	LOG_DBG("ECON1 after bit clr: 0x%02x", reg_data);
  #endif
	enc28j60_write_reg(ECON1, 0x00);
	reg_data = enc28j60_read_reg(ECON1);
  #ifdef ENC28J60_DEBUG
	LOG_DBG("ECON1 after write: 0x%02x", reg_data);
  #endif
	enc28j60_bitset_reg(ECON1, 0x03);
	reg_data = enc28j60_read_reg(ECON1);
  #ifdef ENC28J60_DEBUG
	LOG_DBG("ECON1 after bit set: 0x%02x", reg_data);
  #endif
 #endif

 #if ETH_LOW_LEVEL_SEND_RECV_TEST
	// mem read / write tests
	send_arp_pkt();
	rx_dlen = macphy_pkt_recv(eth_data, RECV_PKT_SZ);
	if (0 < rx_dlen) {
		LOG_DBG("TEST: Received a new Eth packet with size = %d", rx_dlen);
	}
 #endif
#endif
}


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
#ifdef ETHERNET_TEST
		macphy_test();
#endif
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
