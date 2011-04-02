/*
***********************************************************
*                      UCOS-GCC.C
***********************************************************
*                     INCLUDE FILES
***********************************************************
*/
#include "includes.h"
/*
***********************************************************
*                    CREATE A TASK
***********************************************************
*/
UBYTE OSTaskCreate(void (*task)(void *dptr), void *data, void *pstk, UBYTE p, char *name)
{
    OS_TCB *ptr;
    UDWORD  *stk;


    if (OSTCBPrioTbl[p] == (OS_TCB *)0) {               /* Avoid creating task if already exist */
        ptr              = OSTCBGetFree();
	ptr->OSTCBInitialStkPtr = (void *)pstk;
        ptr->OSTCBPrio   = (UBYTE)p;
        ptr->OSTCBStat   = OS_STAT_RDY;
        ptr->OSTCBDly    = 0;
	ptr->TaskName    = name;

	stk              = (UDWORD *)pstk;
	*--stk           = (UDWORD)data;
	*--stk           = (UDWORD)task;
	*--stk           = (UDWORD)0x00000200;          /* PSW = Int. En.   */
	*--stk           = (UDWORD)0x00000008;          /* CS = 0x8         */
	*--stk           = (UDWORD)task;
	*--stk           = (UDWORD)0x00000000;          /* EAX  */
	*--stk           = (UDWORD)0x00000000;          /* ECX  */
	*--stk           = (UDWORD)0x00000000;          /* EDX  */
	*--stk           = (UDWORD)0x00000000;          /* EBX  */
	*--stk           = (UDWORD)0x00000000;          /* ESP  */
	*--stk           = (UDWORD)0x00000000;          /* EBP  */
	*--stk           = (UDWORD)0x00000000;          /* ESI  */
	*--stk           = (UDWORD)0x00000000;          /* EDI  */
	ptr->OSTCBStkPtr = (void *)stk;                 /* Load ESP in TCB  */
	OSTCBPrioTbl[p]  = ptr;
	OS_ENTER_CRITICAL();
	ptr->OSTCBNext        = OSTCBList;
	ptr->OSTCBPrev        = (OS_TCB *)0;
	if (OSTCBList != (OS_TCB *)0) {                 /* Rev. A, This line was missing        */
            OSTCBList->OSTCBPrev = ptr;
        }
        OSTCBList             = ptr;
        OSRdyGrp             |= OSMapTbl[p >> 3];
        OSRdyTbl[p >> 3]     |= OSMapTbl[p & 0x07];
        OS_EXIT_CRITICAL();
        if (OSRunning) {
            OSSched();
        }
        return (OS_NO_ERR);
    } else {
        return (OS_PRIO_EXIST);
    }
}

/*
***********************************************************
*                    SET IDT HANDLER
***********************************************************
*/
void OSSetVect(unsigned char InterruptNumber, void (*Handler)(void))
{
    unsigned short int IDTDescriptor[4];                    /* 2*4 byte     */
    unsigned long  int flags;

    IDTDescriptor[0] = (int)Handler & 0xFFFF;               /* low word     */
    IDTDescriptor[1] = 0x0008;                              /* CS = 0x8     */
    IDTDescriptor[2] = 0x8E00;                              /* P = 1, DPL = 0, interrupt */
    IDTDescriptor[3] = (int)Handler >> 16;                  /* high word    */

    save_flags(flags);
    OS_ENTER_CRITICAL();
    memcpy((void*)(InterruptNumber*8), IDTDescriptor, 8);   /* IDT base = 0 */
    restore_flags(flags);

    return;
}
