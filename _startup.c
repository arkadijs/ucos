
#include "includes.h"

OS_TCB  OSTCBTbl[OS_MAX_TASKS];
UDWORD  OSIdleTaskStk[IDLE_TASK_STK_SIZE];
UDWORD  OSMonitorTaskStk[MONITOR_TASK_STK_SIZE];

void _start(void)
{
	int i;

	for(i = 0; i < 256; ++i)
	  OSSetVect(i, OSDummyISR);

	OSInit(&OSIdleTaskStk[IDLE_TASK_STK_SIZE - 1], OS_MAX_TASKS);
	OSLibcInit();

	OSTaskCreate(OSMonitorTask, NULL, (void*)&OSMonitorTaskStk[MONITOR_TASK_STK_SIZE - 1], 0, "Monitor");

	UserInit();

	outb(0x36, 0x43);
	outb(1193 & 0xFF, 0x40);
	outb(1193 >> 8, 0x40);

	OSSetVect(UCOS, OSCtxSw);
	OSSetVect(0x08, OSTickISR);
	OSSetVect(0x09, OSKbdISRWraper);

	OSStart();
}
