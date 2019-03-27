start:
	mov	ax,0x9000	; this is done in bootsect already, but...
	mov	ds,ax
	mov	ah,0x03		; read cursor pos
	xor	bh,bh
	int	0x10		; save it in known place, con_init fetches
	mov	[0],dx		; it from 0x90000.

	mov	ah,0x88
	int	0x15
	mov	[2],ax

	mov	ah,0x0f
	int	0x10
	mov	[4],bx		; bh = display page
	mov	[6],ax		; al = video mode, ah = window width

	mov	ah,0x12
	mov	bl,0x10
	int	0x10
	mov	[8],ax
	mov	[10],bx
	mov	[12],cx

	cli

	mov	ax,0x0000
	cld			; `direction`=0, movs moves forward
do_move:
	mov	es,ax		; destination segment
	add	ax,0x1000
	cmp	ax,0x9000
	jz	end_move
	mov	ds,ax		; source segment
	sub	di,di
	sub	si,si
	mov 	cx,0x8000
	rep
	movsw
	jmp	do_move

end_move:
	mov	ax,0x9020	; right, forgot this at first. didn`t work :-)
	mov	ds,ax
	lidt	[idt_48]; load idt with 0,0
	lgdt	[gdt_48]; load gdt with whatever appropriate

	call	empty_8042
	mov	al,0xD1		; command write
	out	0x64,al
	call	empty_8042
	mov	al,0xDF		; A20 on
	out	0x60,al
	call	empty_8042

	mov	al,0x11		; initialization sequence
	out	0x20,al		; send it to 8259A-1
	dw	0x00eb,0x00eb		; jmp $+2, jmp $+2
	out	0xA0,al		; and to 8259A-2
	dw	0x00eb,0x00eb
	mov	al,0x20		; start of hardware int`s (0x20)
	out	0x21,al
	dw	0x00eb,0x00eb
	mov	al,0x28		; start of hardware int`s 2 (0x28)
	out	0xA1,al
	dw	0x00eb,0x00eb
	mov	al,0x04		; 8259-1 is master
	out	0x21,al
	dw	0x00eb,0x00eb
	mov	al,0x02		; 8259-2 is slave
	out	0xA1,al
	dw	0x00eb,0x00eb
	mov	al,0x01		; 8086 mode for both
	out	0x21,al
	dw	0x00eb,0x00eb
	out	0xA1,al
	dw	0x00eb,0x00eb
	mov	al,0xFF		; mask off all interrupts for now
	out	0x21,al
	dw	0x00eb,0x00eb
	out	0xA1,al

	mov	ax,0x0001	; protected mode (PE) bit
	lmsw	ax		; This is it;
	jmp	8:0		; jmp offset 0 of segment 8 (cs)

empty_8042:
	dw	0x00eb,0x00eb
	in	al,0x64	; 8042 status port
	test	al,2		; is input buffer full?
	jnz	empty_8042	; yes - loop
	ret

gdt:
	dw	0,0,0,0		; dummy
	dw	0x07FF		; 8Mb - limit=2047 (2048*4096=8Mb)
	dw	0x0000		; base address=0
	dw	0x9A00		; code read/exec
	dw	0x00C0		; granularity=4096, 386

DATA_DESCRIPTOR:
	dw	0x07FF		; 8Mb - limit=2047 (2048*4096=8Mb)
	dw	0x0000		; base address=0
	dw	0x9200		; data read/write
	dw	0x00C0		; granularity=4096, 386

idt_48:
	dw	0			; idt limit=0
	dw	0,0			; idt base=0L

gdt_48:
	dw	0x800		; gdt limit=2048, 256 GDT entries
	dw	512+gdt,0x9	; gdt base = 0X9xxxx
