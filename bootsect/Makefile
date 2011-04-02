
PROGRAM       = bootsect

AS            = nasm

ASFLAGS       = -f bin

SRC           = bootsect.asm

all:            $(PROGRAM)

$(PROGRAM):     $(SRC)
		$(AS) $(ASFLAGS) $(SRC) -o $(PROGRAM)


clean:;         @rm -f $(PROGRAM) *.bak
