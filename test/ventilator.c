
#include "includes.h"

#define TASKS_COUNT     32
#define STK_SIZE        0x100

UDWORD Stacks[TASKS_COUNT + 1][STK_SIZE];
int pos[TASKS_COUNT];

void Task(void *data);
void Top(void *data);

void UserInit(void)
{
	int i = 0;

	for(i = 0; i < TASKS_COUNT; ++i)
	{
	  pos[i] = i + 1;
	  OSTaskCreate(Task, &pos[i], &Stacks[i][STK_SIZE], i + 1, "X");
	}
	OSTaskCreate(Top, NULL, &Stacks[i][STK_SIZE], i + 1,  "Top");
}

void Task(void *data)
{
	static char ch[] = "|/-\\";
	int t;
	int i;

	t = *(int*)data;
	for(i = 0;; ++i)
	{
	  OSTimeDly(t*1000);
	  OSScreenPutChar(t*2, 0, ch[i & 3]);
	}
}

void Top(void *data)
{
	while(1)
	{
	  OSLock();
	  OSPrintf("%d\n", OSIdleTime);
	  OSIdleTime = 0;
	  OSUnlock();
	  OSTimeDly(1000);
	}
}
