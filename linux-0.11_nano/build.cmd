nasm -o bootsect.bin bootsect.s
nasm -o setup.bin setup.s
gcc -o head.o -c head.s
gcc -O2 -o main.o -c main.c
ld --image-base 0x0000 -o system.exe head.o main.o >system.map
objcopy.exe system.exe system.bin
build.exe bootsect.bin setup.bin system.bin
