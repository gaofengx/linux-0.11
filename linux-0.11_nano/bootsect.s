[section .s16]
[BITS 16]
_start:
	mov	ax,0x07c0
	mov	ds,ax
	mov	ax,0x9000
	mov	es,ax
	mov	cx,256
	sub	si,si
	sub	di,di
	rep movsw
	jmp	0x9000:go
go:	mov	ax,cs
	mov	ds,ax
	mov	es,ax
	mov	ss,ax
	mov	sp,0xFF00		; arbitrary value >>512

load_setup:
	mov	dx,0x0000		; drive 0, side 0
	mov	cx,0x0002		; sector 2, track 0
	mov	bx,0x0200		; address = 512, in 0x9000
	mov	ax,0x0200 + 4	; service 2, nr of sectors
	int	0x13			; read it
	jnc	ok_load_setup	; ok - continue
	mov	dx,0x0000
	mov	ax,0x0000		; reset the diskette
	int	0x13
	jmp	load_setup

ok_load_setup:
	mov	ah,0x03		    ; read page 0 cursor x, y to ch and cl
	xor	bh,bh
	int	0x10
    mov [0], dx

	mov	cx,11
	mov	bx,0x0007		; page 0, attribute 7 (normal)
	mov	bp,msg1
	mov	ax,0x1301		; write string, move cursor
	int	0x10

	mov	ax,0x1000
	mov	es,ax		    ; es=0x010000, bx=0
    xor bx, bx

	call	load_system
	call	kill_motor
	mov	ax,0x9000	    ; this is done in bootsect already, but...
	mov	ds,ax

	mov	ah,0x0f
	int	0x10
	mov	[6],ax		    ; al = video mode, ah = window width

	cli

	mov	ax,0x0000
	cld			        ; `direction`=0, movs moves forward
do_move:
	mov	es,ax		    ; destination segment
	add	ax,0x1000
	cmp	ax,0x9000
	jz	move_done
	mov	ds,ax		    ; source segment
	sub	di,di
	sub	si,si
	mov 	cx,0x8000
	rep
	movsw
	jmp	do_move

move_done:
	mov	ax,0x9020	    ; right, forgot this at first. didn`t work :-)
	mov	ds,ax
	jmp	0x9020:0

load_system:
rp_read:
	mov ax,es               ; es = 0x1000
	cmp ax,0x1000 + 0x2220	; have we loaded all yet?
	jb ok1_read
	ret
ok1_read:
	mov ax,18           ; 18 sector per track
	sub ax,[sector]     ; sector init value is 5, so sub result is 13
	mov cx,ax
	shl cx,9            ; cl=0x1A00, left shift 9 bit is multiple with 512
	add cx,bx           ; bx init value=0, second time cx=0x2400, bx=0x1A00, add result 3E00
	jnc ok2_read
	je ok2_read
	xor ax,ax           ; ax = 0
	sub ax,bx           ; ax = 0 - ax, bx init value is 0
	shr ax,9            ; divide with 512
ok2_read:
	call read_track
	mov cx,ax           ; first loop ax = 13
	add ax,[sector]     ; first loop sector = 5, add result is 18

	cmp ax,18
	jne ok3_read
	mov ax,1
	sub ax,[side]       ; side = 1 - side
	jne ok4_read
	inc word [track]    ; track++
ok4_read:
	mov [side],ax       ; first loop ax = 1, save to side
	xor ax,ax           ; first loop track read complete, change sector, clear to 0
ok3_read:
	mov [sector],ax     ; first loop, clear sector to 0
	shl cx,9            ; multiple with 512, result is 0x1A00
	add bx,cx           ; then add to bx, bx init value is 0, now bx = 0x1A00
	jnc rp_read
	mov ax,es
	add ax,0x1000       ; 4K
	mov es,ax
	xor bx,bx
	jmp rp_read

read_track:             ; first loop read 13 sector, offset 0x1A00, second loop offset 0x2400
	push ax
	push bx
	push cx
	push dx
	mov dx,[track]      ; track init value 0, dx: disk & side
	mov cx,[sector]     ; cx: track & sector
	inc cx              ; first loop cx = 6
	mov ch,dl
	mov dx,[side]       ; first loop side = 0
	mov dh,dl
	mov dl,0
	and dx,0x0100
	mov ah,2            ; ax: func id & sector, now ax = 0x020D, read 13 sector
	int 0x13            ; bx = 0000, to 0000, cx = 0x0006, sector 6, dx = 0x0000, disk 0 & side 0
	jc bad_rt
	pop dx
	pop cx
	pop bx
	pop ax
	ret
bad_rt:	mov ax,0        ; reset disk
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

sector:	   dw 1+4	; sectors read of current track
side:	   dw 0		; current side
track:	   dw 0		; current track
msg1:      db 13,10, "Load...", 13,10,13,10
times 	   508-($-$$)	db	0	
root_dev:  dw 0
boot_flag: dw 0xAA55
