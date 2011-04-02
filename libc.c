
#include "includes.h"

OS_SEM  OSPrintfSem;
char    OSPrintfBuf[OS_PRINTF_BUF_LEN];
int     OSCursX = 0, OSCursY = 0;

OS_SEM  OSGetsSem;
OS_SEM  OSGetKeySem;
OS_SEM  OSKbdReadySem;
int     OSKbdReadIndex = 0, OSKbdWriteIndex = 0;
unsigned char   OSKbdBuf[16];

struct malloc_block
{
	struct  malloc_block   *prev, *next;
		int             size;
		int             status;
};

extern struct malloc_block malloc_head;
extern struct malloc_block malloc_after_1m;
struct malloc_block *malloc_head_ptr = (struct malloc_block*)MALLOC_MEM_START_LOW;

/* memory for i386 pc */

struct malloc_block malloc_head =
{
	NULL,                                   /* prev */
	(void*)0x100000,                        /* next */
	640*1024 - MALLOC_MEM_START_LOW,        /* size */
	MALLOC_FREE_BLOCK                       /* stat */
};

struct malloc_block malloc_after_1m =
{
	(void*)MALLOC_MEM_START_LOW,    /* prev */
	NULL,                           /* next */
	MALLOC_MEM_END_HIGH - 0x100000, /* size */
	MALLOC_FREE_BLOCK               /* stat */
};


void OSLibcInit(void)
{
	int i;

	OSmalloc_init();

	OSSemInit(&OSPrintfSem, 1);
	OSSemInit(&OSGetsSem, 1);
	OSSemInit(&OSGetKeySem, 1);
	OSSemInit(&OSKbdReadySem, 0);

	OSScreenMoveCursor(0, 0);
	for(i = 0; i < 80*25*2; i += 2)
	  *(char*)(0x000B8000 + i) = '\0';
}

char *ltos(int n, char *buf, int base)
{
	char *bp, *endp;
	int minus = 0;

	bp = buf;
	if(base == 10 && n < 0)
	{
	  minus = 1;
	  n = -n;
	}

	*bp = '\0';
	do
	{
	  *++bp = "0123456789ABCDEF"[n%base];
	  n /= base;
	}
	while(n);

	if(minus)
	  *++bp = '-';

	for(endp = bp; bp > buf;)
	{
	  minus = *bp;
	  *bp-- = *buf;
	  *buf++ = minus;
	}
	return(endp);
}

int toint(const char **str)
{
	int n;
	for(n = 0; **str >= '0' && **str <= '9'; ++*str)
	  n = n*10 + (**str - '0');
	return(n);
}

int OSPrintf(const char *fmt, ...)
{
	va_list args;
	char *bp, filchar;
	int slen, base, fldwth, prec, lftjust, lnum;

	OSSemPend(&OSPrintfSem, 0);

	va_start(args, fmt);

	for(; *fmt != '\0'; ++fmt)
	{
	  if(*fmt != '%')
	    OSPrintChar(*fmt);
	  else
	  {
	    bp = OSPrintfBuf;
	    filchar  = ' ';
	    fldwth   = 0;
	    lftjust  = 0;
	    prec     = 0;
	    slen     = 0;

	    if(*++fmt == '-')
	    {
	      ++fmt;
	      lftjust = 1;
	    }
	    if(*fmt == '0')
	    {
	      ++fmt;
	      filchar = '0';
	    }
	    if(*fmt != '*')
	      fldwth = toint(&fmt);
	    else
	    {
	      ++fmt;
	      fldwth = va_arg(args, int);
	    }
	    if(*fmt == '.')
	    {
	      ++fmt;
	      prec = toint(&fmt);
	    }
	    switch(*fmt)
	    {
	    default:
	      *bp++ = *fmt;
	      break;

	    case 'c':
	      *bp++ = va_arg(args, char);
	      break;

	    case 's':
	      bp = va_arg(args, char*);
	      break;

	    case 'd':
	      base = 10;
	      goto pnum;

	    case 'x':
	    case 'X':
	      base = 16;
	      goto pnum;

	    case 'b':
	      base = 2;
	      goto pnum;

	    case 'o':
	      base = 8;
	      goto pnum;

pnum:

	      lnum = va_arg(args, int);
	      if(lnum < 0 && base == 10 && filchar == '0')
	      {
		OSPrintChar('-');
		--fldwth;
		lnum = -lnum;
	      }
	      bp = ltos(lnum, bp, base);

	      break;
	    }

	    if(*fmt != 's')
	    {
	      *bp = '\0';
	      slen = bp - OSPrintfBuf;
	      bp = OSPrintfBuf;
	    }
	    else
	    {
	      slen = strlen(bp);
	      if(prec && slen > prec)
		slen = prec;
	    }
	    if((fldwth -= slen) < 0)
	      fldwth = 0;

	    if(!lftjust)
	      while(--fldwth >= 0)
		OSPrintChar(filchar);

	    while(--slen >= 0)
	      OSPrintChar(*bp++);

	    if(lftjust)
	      while(--fldwth >= 0)
		OSPrintChar(filchar);
	  }
	}

	va_end(args);

	OSScreenMoveCursor(OSCursX, OSCursY);

	OSSemPost(&OSPrintfSem);

	return(0);
}

void OSPrintChar(char c)
{
	int needToPut = 0;

	switch(c)
	{
	case '\r':
	  OSCursX = 0;
	  break;

	case '\n':
	  OSCursX = 0;
	  ++OSCursY;
	  break;

	case '\t':
	  OSCursX += 8;
	  break;

	case '\b':
	  --OSCursX;
	  break;

	default:
	  ++OSCursX;
	  needToPut = 1;
	  break;
	}

	if(OSCursX >= 80)
	{
	  OSCursX = 1;
	  ++OSCursY;
	}

	if(OSCursY == 25)
	{
	  OSScreenScroll();
	  --OSCursY;
	}

	if(needToPut)
	  OSScreenPutChar(OSCursX - 1, OSCursY, c);
}

void OSScreenScroll(void)
{
	int x;

	memmove((void*)0x000B8000, (void*)(0x000B8000 + 80*2), 80*24*2);
	for(x = 0; x < 80; ++x)
	  OSScreenPutChar(x, 24, '\0');
}

void OSScreenPutChar(int x, int y, char c)
{
	*(char*)(0x000B8000 + (80*y + x)*2) = c;
}

void OSScreenMoveCursor(int x, int y)
{
	unsigned short pos = y*80 + x;
	outb(15, 0x3D4);
	outb(pos & 0xFF, 0x3D5);
	outb(14, 0x3D4);
	outb(pos >> 8, 0x3D5);
}

void OSGets(unsigned char *str)
{
	int i;
	unsigned char c;

	OSSemPend(&OSGetsSem, 0);

	for(i = 0;; ++i)
	{
	  c = OSGetKey();
	  if(c == '\b')
	  {
	    if(i > 0)
	    {
	      OSPrintChar('\b');
	      OSPrintChar(' ');
	      OSPrintChar('\b');
	      i -= 2;
	    }
	  }
	  else
	  {
	    str[i] = c;
	    OSPrintChar(c);
	  }

	  OSScreenMoveCursor(OSCursX, OSCursY);

	  if(c == '\n')
	  {
	    str[i] = '\0';
	    break;
	  }
	}

	OSSemPost(&OSGetsSem);

	return;
}

unsigned char OSGetKey(void)
{
	unsigned char c;

	OSSemPend(&OSGetKeySem, 0);

	OSSemPend(&OSKbdReadySem, 0);

	c = OSKbdBuf[OSKbdReadIndex];
	OSKbdReadIndex = (OSKbdReadIndex + 1) & 0x0F;

	OSSemPost(&OSGetKeySem);

	return(c);
}

int OSAtoi(char *str, int base)
{
	int num = 0;

	while(*str == ' ')
	  ++str;

	if(base == 16)
	{
	  while((*str >= '0' && *str <= '9') || (*str >= 'A' && *str <= 'F'))
	  {
	    if(*str >= 'A' && *str <= 'F')
	      num = num*base + *str - 'A' + 10;
	    else
	      num = num*base + *str - '0';
	    ++str;
	  }
	}
	else
	{
	  while(*str >= '0' && *str <= '9')
	  {
	    num = num*base + *str - '0';
	    ++str;
	  }
	}

	return(num);
}

int OSStrNCmp(char *s1, char *s2, int n)
{
	int i;

	for(i = 0; i < n && s1[i] != '\0' && s2[i] != '\0'; ++i)
	{
	  if(s1[i] != s2[i])
	    return(1);
	}
	/*
	if(s1[i] == '\0' && s2[i] == '\0')
	  return(0);
	*/
	return(0);
}

void OSmalloc_init(void)
{
	memcpy((void*)MALLOC_MEM_START_LOW, &malloc_head,     sizeof(struct malloc_block));
	memcpy((void*)0x100000,             &malloc_after_1m, sizeof(struct malloc_block));
}

void *OSmalloc(int size)
{
	struct malloc_block *b, *n;

	size += sizeof(struct malloc_block);

	OS_ENTER_CRITICAL();
	for(b = malloc_head_ptr; b != NULL; b = b->next)
	{
	  if(b->status == MALLOC_FREE_BLOCK)
	  {
	    if(b->size < size) /* too small */
	    {
	      OSPrintf("continue, b->size %d\n", b->size);
	      continue;
	    }
	    if(b->size > size + sizeof(struct malloc_block))
	    {
	      OSPrintf("split start\n");
	      /* split in two blocks */
	      n = (struct malloc_block*)((char*)b + size);
	      n->prev = b;
	      n->next = b->next;
	      n->size = b->size - size;
	      n->status = MALLOC_FREE_BLOCK;
	      b->next = n;
	      b->size = size;
	      OSPrintf("split end\n");
	    }
	    b->status = MALLOC_ALLOCATED_BLOCK;
	    OS_EXIT_CRITICAL();
	    OSPrintf("ret\n");
	    return((void*)((char*)b + sizeof(struct malloc_block)));
	  }
	}

	OS_EXIT_CRITICAL();
	OSPrintf("NULL\n");
	return(NULL);
}

void OSfree(void *ptr)
{
	struct malloc_block *b = (struct malloc_block*)((char*)ptr - sizeof(struct malloc_block));

	OS_ENTER_CRITICAL();
	if(b->next->status == MALLOC_FREE_BLOCK)
	{
	  b->next  = b->next->next;
	  b->size += b->next->size;
	}
	if(b->prev->status == MALLOC_FREE_BLOCK)
	{
	  b->prev->next  = b->next;
	  b->prev->size += b->size;
	  OS_EXIT_CRITICAL();
	  return;
	}
	b->status = MALLOC_FREE_BLOCK;
	OS_EXIT_CRITICAL();
}
