AS86 = nasm 

LD	=ld
LDFLAGS	  = -m i386pe -Ttext 0 -e startup_32 -M --image-base 0x0000 
CC	=gcc -march=i386
CFLAGS	=-Wall -O2 -g -fno-builtin
CPP	=cpp -nostdinc -Iinclude

#
# ROOT_DEV specifies the default root-device when making the image.
# This can be either FLOPPY, /dev/xxxx or empty, in which case the
# default of /dev/hd6 is used by 'build'.
#
ROOT_DEV=

CHR_DRV_SRC  = kernel/chr_drv/tty_io.c kernel/chr_drv/console.c
KERNEL_SRC  = kernel/sched.c kernel/printk.c kernel/vsprintf.c
ARCHIVES=kernel/kernel.o
DRIVERS = kernel/chr_drv/chr_drv.a

.c.s:
	$(CC) $(CFLAGS) -nostdinc -Iinclude -S -o $*.s $<

.c.o:
	$(CC) $(CFLAGS) -nostdinc -Iinclude -c -o $*.o $<

all:	Boot.img

tools/system.bin : tools/system.exe 
	tools/objcopy.exe tools/system.exe tools/system.bin
Boot.img: boot/bootsect.bin boot/setup.bin tools/system.bin tools/build.exe Makefile
	tools\build.exe boot/bootsect.bin boot/setup.bin tools/system.bin $(ROOT_DEV)

disk: Image
	dd bs=8192 if=Image of=/dev/fd0

boot/head.o: boot/head.s
	gcc -I./include -traditional -c boot/head.s -o boot/head.o

tools/system.exe:	boot/head.o init/main.o $(ARCHIVES) $(DRIVERS)
	$(LD) $(LDFLAGS) boot/head.o init/main.o $(ARCHIVES) $(DRIVERS) -o tools/system.exe >system.map


kernel/chr_drv/chr_drv.a:$(CHR_DRV_SRC)
	(cd kernel/chr_drv & make)

kernel/kernel.o:$(KERNEL_SRC)
	(cd kernel & make)

boot/setup.bin: boot/setup.s 
	$(AS86) -Iboot/ -o boot/setup.bin boot/setup.s

boot/bootsect.bin:	boot/bootsect.s 
	$(AS86) -Iboot/ -o boot/bootsect.bin boot/bootsect.s

tmp.s:	boot/bootsect.s tools/system
	cat boot/bootsect.s >> tmp.s

clean:
	@del /S /Q *.a *.o system.map tools\system.exe

backup: clean
	(cd .. ; tar cf - linux | compress16 - > backup.Z)
