
#ifndef _UCOS_LIBC_H_
#define _UCOS_LIBC_H_

#define OS_PRINTF_BUF_LEN       128
				/* 512 kb */
#define MALLOC_MEM_START_LOW    0x80000
				/* 2 mb   */
#define MALLOC_MEM_END_HIGH     0x200000

extern void OSLibcInit(void);

extern int  OSPrintf(const char *fmt, ...);
extern void OSPrintChar(char c);
extern void OSScreenScroll(void);
extern void OSScreenPutChar(int x, int y, char c);
extern void OSScreenMoveCursor(int x, int y);

extern void          OSGets(unsigned char *str);
extern unsigned char OSGetKey(void);

extern int  OSAtoi(char *str, int base);
extern int  OSStrNCmp(char *s1, char *s2, int n);

extern void  OSmalloc_init(void);
extern void *OSmalloc(int size);
extern void  OSfree(void *ptr);

#define MALLOC_FREE_BLOCK       0
#define MALLOC_ALLOCATED_BLOCK  1

#endif  /* _UCOS_LIBC_H_ */
