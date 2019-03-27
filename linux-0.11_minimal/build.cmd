nasm  -Iboot/ -o boot/bootsect.bin boot/bootsect.s
nasm  -Iboot/ -o boot/setup.bin boot/setup.s
gcc -I./include -traditional -c boot/head.s -o boot/head.o
gcc -march=i386 -Wall -O2 -g -fno-builtin -nostdinc -Iinclude -c -o init/main.o init/main.c
cd kernel
gcc -mcpu=i386 -Wall -O -g -fstrength-reduce -fomit-frame-pointer -finline-functions -nostdinc -I../include -c -o sched.o sched.c
gcc -mcpu=i386 -Wall -O -g -fstrength-reduce -fomit-frame-pointer -finline-functions -nostdinc -I../include -c -o printk.o printk.c
gcc -mcpu=i386 -Wall -O -g -fstrength-reduce -fomit-frame-pointer -finline-functions -nostdinc -I../include -c -o vsprintf.o vsprintf.c
ld -r -o kernel.o sched.o printk.o vsprintf.o
cd ../kernel/chr_drv
gcc -mcpu=i386 -Wall -O -g -fstrength-reduce -fomit-frame-pointer -finline-functions -nostdinc -I../../include -c -o tty_io.o tty_io.c
gcc -mcpu=i386 -Wall -O -g -fstrength-reduce -fomit-frame-pointer -finline-functions -nostdinc -I../../include -c -o console.o console.c
ar rcs chr_drv.a tty_io.o console.o
cd ../..
ld -m i386pe -Ttext 0 -e startup_32 -M --image-base 0x0000  boot/head.o init/main.o kernel/kernel.o kernel/chr_drv/chr_drv.a -o tools/system.exe >system.map
tools\objcopy.exe tools\system.exe tools\system.bin
tools\build.exe boot/bootsect.bin boot/setup.bin tools/system.bin
