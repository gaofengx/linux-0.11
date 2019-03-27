INITSEG  equ 0x9000			; we move boot here - out of the way
[section .s16]
[BITS 16]
_start:
	mov	ax,0x07c0
	mov	ds,ax
	mov	ax,INITSEG
	mov	es,ax
	mov	cx,256
	sub	si,si
	sub	di,di
	rep
	movsw
	jmp	INITSEG:go
go:	mov	ax,cs
	mov	ds,ax
	mov	es,ax
	mov	ss,ax
	mov	sp,0xFF00		; arbitrary value >>512

load_setup:
	mov	dx,0x0000		; drive 0, head 0
	mov	cx,0x0002		; sector 2, track 0
	mov	bx,0x0200		; address = 512, in INITSEG
	mov	ax,0x0200 + 4	; service 2, nr of sectors
	int	0x13			; read it
	jnc	ok_load_setup		; ok - continue
	mov	dx,0x0000
	mov	ax,0x0000		; reset the diskette
	int	0x13
	jmp	load_setup

ok_load_setup:
	mov	dl,0x00
	mov	ax,0x0800		; AH=8 is get drive parameters
	int	0x13
	mov	ch,0x00
	mov	[sectors],cx
	mov	ax,INITSEG
	mov	es,ax

	mov	ah,0x03		; read cursor pos
	xor	bh,bh
	int	0x10

	mov	cx,24
	mov	bx,0x0007		; page 0, attribute 7 (normal)
	mov	bp,msg1
	mov	ax,0x1301		; write string, move cursor
	int	0x10

	mov	ax,0x1000
	mov	es,ax		; segment of 0x010000
	call	read_it
	call	kill_motor
	jmp	0x9020:0

sread:	dw 1+4	; sectors read of current track
head:	dw 0			; current head
track:	dw 0			; current track

read_it:
	mov ax,es
	test ax,0x0fff
die:	jne die			; es must be at 64kB boundary
	xor bx,bx		; bx is starting address within segment
rp_read:
	mov ax,es
	cmp ax,0x1000 + 0x2220	; have we loaded all yet?
	jb ok1_read
	ret
ok1_read:
	mov ax,[sectors]
	sub ax,[sread]
	mov cx,ax
	shl cx,9
	add cx,bx
	jnc ok2_read
	je ok2_read
	xor ax,ax
	sub ax,bx
	shr ax,9
ok2_read:
	call read_track
	mov cx,ax
	add ax,[sread]

	cmp ax,[sectors]
	jne ok3_read
	mov ax,1
	sub ax,[head]
	jne ok4_read
	inc word [track]
ok4_read:
	mov [head],ax
	xor ax,ax
ok3_read:
	mov [sread],ax
	shl cx,9
	add bx,cx
	jnc rp_read
	mov ax,es
	add ax,0x1000
	mov es,ax
	xor bx,bx
	jmp rp_read

read_track:
	push ax
	push bx
	push cx
	push dx
	mov dx,[track]
	mov cx,[sread]
	inc cx
	mov ch,dl
	mov dx,[head]
	mov dh,dl
	mov dl,0
	and dx,0x0100
	mov ah,2
	int 0x13
	jc bad_rt
	pop dx
	pop cx
	pop bx
	pop ax
	ret
bad_rt:	mov ax,0
	mov dx,0
	int 0x13
	pop dx
	pop cx
	pop bx
	pop ax
	jmp read_track

kill_motor:
	push dx
	mov dx,0x3f2
	mov al,0
	out dx,ax
	pop dx
	ret

sectors:   dw 0
msg1:      db 13,10, "Loading system ...", 13,10,13,10
times 	   508-($-$$)	db	0	
root_dev:  dw 0
boot_flag: dw 0xAA55
