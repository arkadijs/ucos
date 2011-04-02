/*
************************************************************
*                      UCOS-GCC.H
************************************************************
*                      CONSTANTS
************************************************************
*/
#define  UCOS  0xF1     /* Context switch vector number */
#define  OS_MAX_TASKS           64
#define  IDLE_TASK_STK_SIZE     0x80
#define  MONITOR_TASK_STK_SIZE  0x200
/*
************************************************************
*                        MACROS
************************************************************
*/
#define  OS_ENTER_CRITICAL()  asm("cli")
#define  OS_EXIT_CRITICAL()   asm("sti")
#define  OS_TASK_SW()         asm("int $0xF1")
/*
************************************************************
*                       DATA TYPES
************************************************************
*/
typedef unsigned char  BOOLEAN;
typedef unsigned char  UBYTE;
typedef signed   char  BYTE;
typedef unsigned int   UDWORD;
typedef signed   int   DWORD;
/*
************************************************************
*                       FUNCTIONS
************************************************************
*/
extern void OSSetVect(unsigned char InterruptNumber, void (*Handler)(void));
extern void _start(void);
extern void OSMonitorTask(void *data);
extern void UserInit(void);
