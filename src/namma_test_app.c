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


// #define OSEK_TASK_DEBUG	1
// #define ENC28J60_DEBUG	1

#define GETEVENT_TEST
//#define GET_RELEASE_RESOURCE_TEST
//#define SCHEDULE_TEST
//#define TERMINATE_TASK_TEST
//#define CHAINTASK_TEST


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
	uint16 phy_reg;
	uint16 rx_dlen;
	uint8 reg_data;
	uint8 eth_data[RECV_PKT_SZ];

#if defined BITOPS_TEST
	// ECON1 register tests
	enc28j60_bitclr_reg(ECON1, 0x03);
	reg_data = enc28j60_read_reg(ECON1);
 #ifdef ENC28J60_DEBUG
	pr_log("ECON1 after bit clr: 0x%02x\n", reg_data);
 #endif
	enc28j60_write_reg(ECON1, 0x00);
	reg_data = enc28j60_read_reg(ECON1);
 #ifdef ENC28J60_DEBUG
	pr_log("ECON1 after write: 0x%02x\n", reg_data);
 #endif
	enc28j60_bitset_reg(ECON1, 0x03);
	reg_data = enc28j60_read_reg(ECON1);
 #ifdef ENC28J60_DEBUG
	pr_log("ECON1 after bit set: 0x%02x\n", reg_data);
 #endif
#endif

#define ETH_LOW_LEVEL_SEND_RECV_TEST 0
#if ETH_LOW_LEVEL_SEND_RECV_TEST
	// mem read / write tests
	send_arp_pkt();
	rx_dlen = macphy_pkt_recv(eth_data, RECV_PKT_SZ);
	if (0 < rx_dlen) {
		pr_log("TEST: Received a new Eth packet with size = %d\n", rx_dlen);
	}
#endif
}


TASK(Task_A) {
	AlarmBaseType info;
	TickType tick_left;
	static bool cycle_started = false;
#ifdef OSEK_TASK_DEBUG
	pr_log("%s, sp=0x%08X\n", __func__, _get_stack_ptr());
#endif

#ifdef ALARM_BASE_TEST
	if (E_OK == GetAlarmBase(0, &info))
		pr_log("0: ticks/base = %d\n", info.ticksperbase);
	if (E_OK == GetAlarmBase(1, &info))
		pr_log("1: ticks/base = %d\n", info.ticksperbase);
	if (E_OK == GetAlarmBase(2, &info))
		pr_log("2: ticks/base = %d\n", info.ticksperbase);
#endif

#ifdef GET_ALARM_TEST
	if (E_OK == GetAlarm(0, &tick_left))
		pr_log("0: ticks remaining = %d\n", tick_left);
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

#ifdef EVENT_SET_CLEAR_TEST
	static int setcnt = 4;
	if (setcnt > 0) {
		setcnt--;
		SetEvent(1, 1);
	}
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
	pr_log("Current task: %d, state = %d\n", task, state);
	task = 1;
	GetTaskState(task, &state);
	pr_log("Task: %d, state = %d\n", task, state);
	task = 2;
	GetTaskState(task, &state);
	pr_log("Task: %d, state = %d\n", task, state);
#endif

	static bool toggle_bit;
	EventMaskType Event = 0;

#ifdef ENABLE_DISABLE_ISR_TEST
	DisableAllInterrupts();
	pr_log("Enable / Disable ISR test\n");
	EnableAllInterrupts();
#endif

#ifdef BOARD_STM32F407VET6
	// Test code - turn on/off LED D2 & D3 on the board
	GPIOA_MODER |= ((1 << 14) | (1 << 12)); // LED - PA7, PA6: GPIO mode
#endif

	if (toggle_bit) {
		toggle_bit = false;
#ifdef BOARD_STM32F407VET6
		GPIOA_ODR |= 0x40;
#endif
#ifdef BOARD_RP2040
		Dio_WriteChannel(25, STD_HIGH);
		macphy_test();
#endif
		SetEvent(1, 0x101);
#ifdef OSEK_TASK_DEBUG
		pr_log("Task A: Triggered event for Task B\n");
#endif
		GetEvent(1, &Event);
#ifdef OSEK_TASK_DEBUG
		pr_log("Task A: Event = 0x%016X\n", Event);
#endif
		#ifdef GET_RELEASE_RESOURCE_TEST
		pr_log("Task A Priority = %d\n", _OsTaskCtrlBlk[_OsCurrentTask.id].ceil_prio);
		ReleaseResource(RES(mutex1));
		pr_log("Task A Priority = %d\n", _OsTaskCtrlBlk[_OsCurrentTask.id].ceil_prio);
		#endif
	}
	else {
		toggle_bit = true;
#ifdef BOARD_STM32F407VET6
		GPIOA_ODR &= ~(0x40);
#endif
#ifdef BOARD_RP2040
		Dio_WriteChannel(25, STD_LOW);
#endif
		#ifdef GET_RELEASE_RESOURCE_TEST
		pr_log("Task A Priority = %d\n", _OsTaskCtrlBlk[_OsCurrentTask.id].ceil_prio);
		GetResource(RES(mutex1));
		pr_log("Task A Priority = %d\n", _OsTaskCtrlBlk[_OsCurrentTask.id].ceil_prio);
		#endif
	}
#endif
}



TASK(Task_B) {
#ifdef OSEK_TASK_DEBUG
	pr_log("%s, sp=0x%08X\n", __func__, _get_stack_ptr());
#endif
#ifdef CANCEL_ALARM
	static int i = 10;
	if (i-- <= 0) 
		CancelAlarm(1);
#endif
#ifdef EVENT_SET_CLEAR_TEST
	ClearEvent(1);
#endif
#ifdef GETEVENT_TEST

#ifdef WAITEVENT_TEST
	WaitEvent(2);
#endif
	EventMaskType Event = 0;
	ClearEvent(1);
	GetEvent(1, &Event);
#ifdef OSEK_TASK_DEBUG
	pr_log("Task B: Event = 0x%016X\n", Event);
#endif // OSEK_TASK_DEBUG
#endif // GETEVENT_TEST
	static bool toggle_bit;
	if (toggle_bit) {
		toggle_bit = false;
#ifdef BOARD_STM32F407VET6
		GPIOA_ODR |= 0x80;
#endif
	}
	else {
		toggle_bit = true;
#ifdef BOARD_STM32F407VET6
		GPIOA_ODR &= ~(0x80);
#endif
	}

}



TASK(Task_C) {
#ifdef OSEK_TASK_DEBUG
	pr_log("%s, sp=0x%08X\n", __func__, _get_stack_ptr());
#endif
}



void Alarm_uSecAlarm_callback(void) {
#ifdef OSEK_TASK_DEBUG
	pr_log("%s, sp=0x%08X\n", __func__, _get_stack_ptr());
#endif
}


TASK(Task_D) {
#ifdef OSEK_TASK_DEBUG
	pr_log("%s, sp=0x%08X\n", __func__, _get_stack_ptr());
#endif
}
