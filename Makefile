
.SUFFIXES:      .asm

TARGET        = libucos.a

CC            = gcc
AS            = nasm
AR            = ar

CFLAGS        = -march=i486 -O2 -fomit-frame-pointer -fno-strength-reduce -pipe -Wall -DUCOS_IDLE_COUNTER
ASFLAGS       = -f elf

LIBS          =
INCL          =

OBJS          = ucos.o          \
		ucos-gcc.o      \
		ucos-asm.o      \
		libc.o          \
		keyboard.o      \
		monitor.o

all:            $(TARGET) _startup.o

$(TARGET):      $(OBJS)
		$(AR) rs $(TARGET) $(OBJS)

.c.o:;          $(CC) $(CFLAGS) -c $(INCL) $<

.asm.o:;        $(AS) $(ASFLAGS) $<

clean:;         @rm -f $(OBJS) _startup.o $(TARGET) *.bak .,*
