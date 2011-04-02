
#include "includes.h"

void _monitor_ps(unsigned char *params)
{
	int     i, printed;
	char    stat;
	OS_TCB *p;

	for(i = 0, printed = 0; i < OS_LO_PRIO + 1; ++i)
	{
	  if(printed%23 == 0)
	  {
	    if(printed != 0)
	    {
	      OSPrintf("\npress any key to continue");
	      OSGetKey();
	      OSPrintf("\n");
	    }
	    OSPrintf("PRIO STAT      DLY STACK ptr     used NAME");
	  }

	  p = OSTCBPrioTbl[i];
	  if(p != NULL)
	  {
	    switch(p->OSTCBStat)
	    {
	    case OS_STAT_RDY:
	      if(p->OSTCBDly == 0)
		stat = 'R';
	      else
		stat = 'W';
	      break;
	    case OS_STAT_MBOX: stat = 'M'; break;
	    case OS_STAT_SEM:  stat = 'S'; break;
	    case OS_STAT_Q:    stat = 'Q'; break;
	    default:           stat = 'U'; break;
	    }

	    OSPrintf("\n%4d%5c %08X  %08X %08X %s",
		     (int)p->OSTCBPrio,
		     stat,
		     p->OSTCBDly,
		     p->OSTCBStkPtr,
		     (int)((char*)p->OSTCBInitialStkPtr - (char*)p->OSTCBStkPtr),
		     p->TaskName);
	    ++printed;
	  }
	}
	OSPrintf("\n");
	return;
}

struct
{
	char  *Name;
	int    NameLen;
	void (*Func)(unsigned char *params);

}       cmds[] =
{
	{ "PS", 0, _monitor_ps          },
	{ NULL, 0, NULL                 }
};

void OSMonitorTask(void *data)
{
	unsigned char s[128];
	int i, j, badcmd;

	for(i = 0; cmds[i].Name != NULL; ++i)
	  cmds[i].NameLen = strlen(cmds[i].Name);

	while(1)
	{
	  OSPrintf("monitor>");
	  OSGets(s);
	  if(s[0] == '\0')
	    continue;
	  for(i = 0; s[i] == ' ' && s[i] != '\0'; ++i)
	    ;
	  badcmd = 1;
	  for(j = 0; cmds[j].Name != NULL; ++j)
	    if(!strncmp(cmds[j].Name, &s[i], cmds[j].NameLen))
	    {
	      cmds[j].Func(s + i + cmds[j].NameLen);
	      badcmd = 0;
	    }
	  if(badcmd)
	    OSPrintf("monitor: command not found\n");
	}
}
