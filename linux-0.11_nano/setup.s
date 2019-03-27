start:

	lgdt	[gdt_48]; load gdt with whatever appropriate

	mov	al,0xDF		; A20 on
	out	0x60,al

	mov	ax,0x0001	; protected mode (PE) bit
	lmsw	ax		; This is it;
	jmp	8:0		; jmp offset 0 of segment 8 (cs)

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

gdt_48:
	dw	0x800		; gdt limit=2048, 256 GDT entries
	dw	512+gdt,0x9	; gdt base = 0X9xxxx

; head.s
;.text
;.globl _idt,_gdt,_pg_dir
;_pg_dir:
;.globl startup_32
;startup_32:
;	movl $0x10,%eax
;	mov %ax,%ds
;	mov %ax,%es
;	mov %ax,%fs
;	mov %ax,%gs
;	lss _stack_start,%esp
;	pushl $0		# These are the parameters to main :-)
;	pushl $0
;	pushl $0
;	pushl $0		# return address for main, if it decides to.
;	pushl $_start_kernel
;    ret
;
;
;main.c
;#define PAGE_SIZE 4096
;
;long user_stack[PAGE_SIZE>>2];
;
;struct {
;	long * a;
;	short b;
;} stack_start = { & user_stack [PAGE_SIZE>>2] , 0x10 };
;
;#define ORIG_VIDEO_COLS (((*(unsigned short *)0x90006) & 0xff00) >> 8)
;
;void start_kernel(void)
;{
;	char *display_desc = "EGA3";
;	char *display_ptr = ((char *)0xb8000) + ORIG_VIDEO_COLS * 2 - 8;
;
;	while (*display_desc)
;	{
;		*display_ptr++ = *display_desc++;
;		display_ptr++;
;	}
;	for(;;);
;}
;
;C:\Pf\linux-0.11\linux-0.11_nano>build.bat
;C:\Pf\linux-0.11\linux-0.11_nano>nasm -o bootsect.bin bootsect.s
;C:\Pf\linux-0.11\linux-0.11_nano>nasm -o setup.bin setup.s
;C:\Pf\linux-0.11\linux-0.11_nano>gcc -o head.o -c head.s
;C:\Pf\linux-0.11\linux-0.11_nano>gcc -O2 -o main.o -c main.c
;C:\Pf\linux-0.11\linux-0.11_nano>ld --image-base 0x0000 -o system.exe head.o main.o  1>system.map
;ld: warning: cannot find entry symbol _mainCRTStartup; defaulting to 00001000
;C:\Pf\linux-0.11\linux-0.11_nano>Trans.exe System.exe system.bin
;[Trans] base=004E0000
;[Trans] vSize=00004014, VirtualAddress=00004000, VirtualSize=00000014
;[Trans] .text  PEOff 00000400, PEEnd 00000600, BinOff 00001000, BinEnd 00001200, size 00000200
;[Trans] .data  PEOff 00000600, PEEnd 00000800, BinOff 00002000, BinEnd 00002200, size 00000200
;[Trans] .bss   PEOff 00000000, PEEnd 00000000, BinOff 00003000, BinEnd 00003000, size 00000000 <no data>
;[Trans] .idata PEOff 00000800, PEEnd 00000a00, BinOff 00004000, BinEnd 00004200, size 00000200
;C:\Pf\linux-0.11\linux-0.11_nano>build.exe bootsect.bin setup.bin system.bin
;Root device is (2, 29)
;Boot sector 512 bytes. offset 0x0, end 0x200
;Setup is 95 bytes. offset 0x0200, end 0x0a00
;System is 16896 bytes. offset 0x0a00, end 0x00004c00
;total size is  1474560 bytes.
;C:\Pf\linux-0.11\linux-0.11_nano>
;
;# bochsrc.bxrc file for Tinix.
;megs: 32
;romimage: file="../bochs/BIOS-bochs-latest"
;vgaromimage: file="../bochs/VGABIOS-lgpl-latest"
;vga: extension=vbe
;floppya: 1_44=boot.img, status=inserted
;com1: enabled=1, dev="\\.\pipe\com_1"
;boot: a
;log: bochsout.txt
;mouse: enabled=0
;keyboard_mapping: enabled=1, map="../bochs/keymaps/x11-pc-us.map"
;
;..\Bochs\bochs -q -f bochsrc.bxrc
;