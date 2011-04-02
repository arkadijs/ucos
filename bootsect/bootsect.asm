[BITS 16]
[ORG 0x0]

SYSSIZE         equ     1024            ; SYSSIZE is the number of blocks of
					; 16 bytes length to be loaded
					; (image_file_size + 15)/16
BOOTSEG         equ     0x07C0          ; original address of boot-sector
INITSEG         equ     0x9000          ; move boot at 0x90000
SYSSEG          equ     0x2000          ; system loaded at 0x20000
SYSSTART        equ     0x20000         ; system start at 0x20000

start
	cld                             ; move loader
	mov     ax, BOOTSEG             ; from
	mov     ds, ax
	mov     ax, INITSEG             ; to
	mov     es, ax
	mov     cx, 0xFF                ; boot-sector length
	xor     si, si
	xor     di, di
	rep movsw

	jmp     INITSEG:go

go
	mov     ds, ax                  ; establish ds and stack
	mov     ss, ax
	mov     di, 0x4000
	mov     sp, di

	mov     fs, cx                  ; setup multi-sector reads, cx = 0 after rep movs
	push    ds
	push    di
	lds     si, [fs:0x78]
	mov     cx, 6
	rep movsw
	pop     di
	pop     ds
	mov     byte [di + 4], 36
	mov     [fs:0x78], di
	mov     [fs:0x78 + 2], ds

	xor     ah, ah                  ; reset FDC
	xor     dl, dl
	int     0x13

	mov     si, disksizes
probe_loop
	lodsb
	cbw
	mov     [sectors], ax
	cmp     si, disksizes + 4
	jae     got_sectors
	mov     cx, ax
	mov     ax, 0x0201
	mov     bx, 0x0200
	xor     dx, dx
	int     0x13
	jc      probe_loop

got_sectors

	mov     ax, INITSEG
	mov     es, ax

	mov     ah, 0x03                ; read cursor pos
	xor     bh, bh
	int	0x10
	
	mov     cx, 9
	mov     bx, 0x0007              ; page 0, attribute 7 (normal)
	mov     bp, msg
	mov     ax, 0x1301              ; write string, move cursor
	int	0x10

	mov     ax, SYSSEG
	mov     es, ax

	mov     al, 1
	mov     [sread], al

	xor     bx, bx                  ; bx is starting address within segment

rp_read
	mov     ax, es
	sub     ax, SYSSEG
	cmp     ax, SYSSIZE             ; if loaded all yet?
	jbe     ok1_read
	jmp     read_done
ok1_read
	mov     ax, [sectors]
	sub     ax, [sread]
	mov     cx, ax
	shl     cx, 9
	add     cx, bx
	jnc     ok2_read
	je      ok2_read
	xor     ax, ax
	sub     ax, bx
	shr     ax, 9
ok2_read
	call    read_track
	mov     cx, ax
	add     ax, [sread]
	cmp     ax, [sectors]
	jne     ok3_read
	mov     ax, 1
	sub     ax, [head]
	jne     ok4_read
	inc     word [track]
ok4_read
	mov     [head], ax
	xor     ax, ax
ok3_read
	mov     [sread], ax
	shl     cx, 9
	add     bx, cx
	jnc     rp_read
	mov     ax, es
	add     ah, 0x10
	mov     es, ax
	xor     bx, bx
	jmp     rp_read

read_track
	pusha
	pusha	
	mov     ax, 0x0E2E              ; loading... message 2e = .
	mov     bx, 7
 	int	0x10
	popa		

	mov     dx, [track]
	mov     cx, [sread]
	inc	cx
	mov     ch, dl
	mov     dx, [head]
	mov     dh, dl
	and     dx, 0x0100
	mov     ah, 2
	
	push    dx
	push	cx
	push	bx
	push	ax

	int	0x13
	jc	bad_rt
	add     sp, 8
	popa
	ret

bad_rt
	push    ax
	xor     ah, ah
	xor     dl, dl
	int     0x13

	add     sp, 10
	popa	
	jmp     read_track

read_done
	push    dx
	mov     dx, 0x03F2
	xor     al, al
	out     dx, al
	pop     dx

	mov     ax, 0x0E0D              ; CR
	int     0x10
	mov     al, 0x0A                ; LF
	int     0x10

	cli
	lidt    [idt_48]                ; load idt
	lgdt    [gdt_48]                ; load gdt
	cld
	xor     ax, ax
	mov     es, ax
	mov     di, 0x0800
	mov     si, gdt
	mov     cx, 12
	rep movsw

	mov     ax, 1
	lmsw    ax
	jmp     dword 0x8:(INITSEG*0x10 + flush_instr)
flush_instr
[BITS 32]
	mov     ax, 0x0010
	mov     ds, ax
	mov     es, ax
	mov     fs, ax
	mov     gs, ax
	mov     ss, ax
	mov     esp, 0x0009FFF0

	call    empty_8042              ; enable A20
	mov     al, 0xD1
	out     0x64, al
	call    empty_8042
	mov     al, 0xDF
	out     0x60, al
	call    empty_8042

;        mov     esi, SYSSEG*0x10
;        mov     edi, SYSSTART
;        mov     ecx, SYSSIZE*0x4
;        rep movsd

	jmp     0x8:SYSSTART

empty_8042
	call    delay
	in      al, 0x64
	test    al, 1
	jz      no_output
	call    delay
	in      al, 0x60
	jmp     empty_8042
no_output
	test    al, 2
	jnz     empty_8042
	ret

delay
	jmp     next_com
next_com
	ret

disksizes       db      36, 18, 15, 9
sectors         dw      0

msg             db      13, 10, 'Loading'

sread           dw      0                       ; sectors read of current track
head            dw      0                       ; current head
track           dw      0                       ; current track

gdt
		dw      0, 0, 0, 0              ; dummy

		dw      0xFFFF                  ; 4Gb
		dw      0x0000                  ; base address = 0
		dw      0x9A00                  ; code read/exec
		dw      0x00CF                  ; granularity = 4096

		dw      0xFFFF                  ; 4Gb
		dw      0x0000                  ; base address = 0
		dw      0x9200                  ; data read/write
		dw      0x00CF                  ; granularity = 4096

idt_48
		dw      0x0800                  ; idt limit = 2048, 256 IDT entries
		dd      0x00000000              ; idt base = 0

gdt_48
		dw      0x0800                  ; gdt limit = 2048, 256 GDT entries
		dd      0x00000800              ; gdt base = 0x0800

times start - $ + 510   db      0

		dw      0xAA55                  ; signature
