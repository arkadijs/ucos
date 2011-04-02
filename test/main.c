
#include "includes.h"

#define TASKS_COUNT     1
#define STK_SIZE        0x200

UDWORD Stacks[TASKS_COUNT + 1][STK_SIZE];
int pos[TASKS_COUNT];
OS_SEM sm[TASKS_COUNT];

void Task(void *data);
void Top(void *data);

struct malloc_block
{
	struct  malloc_block   *prev, *next;
		int             size;
		int             status;
};

extern struct malloc_block *malloc_head_ptr;

void UserInit(void)
{
	int i = 0;

	for(i = 0; i < TASKS_COUNT; ++i)
	{
	  pos[i] = i + 1;
	  OSSemInit(&sm[i], 0);
	  OSTaskCreate(Task, &pos[i], &Stacks[i][STK_SIZE], i + 1, "X");
	}
	OSTaskCreate(Top, NULL, &Stacks[i][STK_SIZE], i + 1,  "Top");
}

void Task(void *data)
{
	char *ch[8];
	int t, i, p;

	for(i = 0; i < 8; ++i) ch[i] = NULL;

	t = *(int*)data;
	for(i = 0; i < 1000000; ++i)
	{
	  p = i & 7;
	  if(ch[p] == NULL)
	  {
	    OSPrintf("malloc size %d\n", t*100 + i%100);
	    ch[p] = OSmalloc(t*100 + i%100);
	    OSPrintf("sleep\n");
	    OSTimeDly(10);
	  }
	  else
	  {
	    OSPrintf("free\n");
	    OSfree(ch[p]);
	    ch[p] = NULL;
	  }
	}
	OSPrintf("Task %d end\n", t);
	OSSemPost(&sm[t - 1]);
	OSTaskDelete();
}

void Top(void *data)
{
	int i;
	struct malloc_block *p;

	for(i = 0; i < TASKS_COUNT; ++i)
	  OSSemPend(&sm[i], 0);

	for(p = malloc_head_ptr; p != NULL; p = p->next)
	  OSPrintf("next %8X prev %8X size %d status %d\n", p->next, p->prev, p->size, p->status);

	while(1)
	{
	  /*
	  OSLock();
	  OSPrintf("%d\n", OSIdleTime);
	  OSIdleTime = 0;
	  OSUnlock();
	  OSTimeDly(1000);
	  */
	}
}
