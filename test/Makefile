
IMAGE         = image

IMAGE_OFFSET  = 0x20000

CC            = gcc
LD            = ld
OBJDUMP       = objdump
OBJCOPY       = objcopy

CFLAGS        = -m486 -O2 -fomit-frame-pointer -pipe -Wall
#LDFLAGS       = -static -m elf_i386 -Ttext $(IMAGE_OFFSET) -T ucos-ldscript
LDFLAGS       = -m elf_i386 -Ttext $(IMAGE_OFFSET) -T ucos-ldscript
OBJDUMPFLAGS  = -k -q -o $(IMAGE_OFFSET)
OBJCOPYFLAGS  = -O binary

LIBS          = ./../libucos.a /usr/lib/libc.a
INCL          = -I./../

OBJS          = main.o

all:            $(IMAGE)

$(IMAGE):       $(OBJS) $(LIBS) ./../_startup.o
		$(LD) $(LDFLAGS) $(LIBS) $(OBJS) -o $(IMAGE).linked
		@#$(OBJDUMP) $(OBJDUMPFLAGS) $(IMAGE).linked > $(IMAGE)
		$(OBJCOPY) $(OBJCOPYFLAGS) $(IMAGE).linked $(IMAGE)
		@chmod -x image
		@rm -f $(IMAGE).linked

.c.o:;          $(CC) $(CFLAGS) -c $(INCL) $<

clean:;         @rm -f $(OBJS) $(IMAGE) *.bak
