#include "includes.h"

#define KBD_STATUS_REG          ((unsigned int)0x64)
#define KBD_DATA_REG            ((unsigned int)0x60)
#define KBD_OBF                 ((unsigned int)0x01)
#define KBD_GTO                 ((unsigned int)0x40)
#define KBD_PERR                ((unsigned int)0x80)

#define KBD_ESC                 0x1B
#define KBD_ENTER               '\n'
#define KBD_BACKSPACE           '\b'
#define KBD_TAB                 '\t'
#define KBD_UP                  0
#define KBD_DOWN                1
#define KBD_RIGHT               2
#define KBD_LEFT                3

int _KbdWaitForInput(void)
{
	int status, data;

	while(1)
	{
	  status = inb(KBD_STATUS_REG);

	  if(!(status & KBD_OBF))
	    return -1;

          data = inb(KBD_DATA_REG);

	  if(status & (KBD_GTO | KBD_PERR))
	    continue;

	  return(data & 0xFF);
	}
}

unsigned char _KbdScan2Ascii[] =
{
	KBD_ESC,
	'1',
	'2',
	'3',
	'4',
	'5',
	'6',
	'7',
	'8',
	'9',
	'0',
	'-',
	'=',
	KBD_BACKSPACE,
	KBD_TAB,
	'Q',
	'W',
	'E',
	'R',
	'T',
	'Y',
	'U',
	'I',
	'O',
	'P',
	'[',
	']',
	KBD_ENTER,
	0,
	'A',
	'S',
	'D',
	'F',
	'G',
	'H',
	'J',
	'K',
	'L',
	';',
	'\'',
	0, 0,
	'\\',
	'Z',
	'X',
	'C',
	'V',
	'B',
	'N',
	'M',
	',',
	'.',
	'/',
	0, 0, 0,
	' '
};

extern OS_SEM   OSKbdReadySem;
extern int      OSKbdReadIndex, OSKbdWriteIndex;
extern unsigned char    OSKbdBuf[];

void OSKbdISR(void)
{
	int znak;

	znak = _KbdWaitForInput();

	if(znak < 0)
	  return;

	if(znak != 0xE0 && znak != 0xE1)
	{
	  unsigned char ascii = 0;
	  _KbdWaitForInput();

	  if(znak > 0 && znak < 0x40)
	    ascii = _KbdScan2Ascii[znak - 1];
	  else
	    switch(znak)
	    {
              case 0x48:   ascii = KBD_UP;
	      case 0x4B:   ascii = KBD_LEFT;
	      case 0x4D:   ascii = KBD_RIGHT;
	      case 0x50:   ascii = KBD_DOWN;
  	    }

	  if(ascii != 0)
	  {
	    OSKbdBuf[OSKbdWriteIndex] = ascii;
	    OSKbdWriteIndex = (OSKbdWriteIndex + 1) & 0x0F;
	    OSSemPost(&OSKbdReadySem);
	  }
	}
	else if(znak == 0xE1)
	{
	  _KbdWaitForInput();
	  _KbdWaitForInput();
	  _KbdWaitForInput();
	  _KbdWaitForInput();
	  _KbdWaitForInput();
	}
	else
	{
	  znak = _KbdWaitForInput();

	  if(znak == 0x38 || znak==0x1D || znak==0xB8 || znak==0x9D ||
	     znak == 0x52 || znak==0x47 || znak==0x49 || znak==0x53 ||
	     znak == 0x4F || znak==0x51 || znak==0x35 || znak==0x1C ||
	     znak == 0x5B || znak==0x5C || znak==0x5D)
	  {
	    _KbdWaitForInput();
	    _KbdWaitForInput();
	  }
	  else if(znak == 0x37)
	  {
	    _KbdWaitForInput();
	    _KbdWaitForInput();
	    _KbdWaitForInput();
	    _KbdWaitForInput();
	  }
	  else
	  {
	    _KbdWaitForInput();
	    _KbdWaitForInput();
	    _KbdWaitForInput();
	    _KbdWaitForInput();
	    _KbdWaitForInput();
	    _KbdWaitForInput();
	  }
	}
}
