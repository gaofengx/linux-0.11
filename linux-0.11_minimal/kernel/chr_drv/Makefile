AR	=ar
AS	=as
LD	=ld
LDFLAGS	= -x
CC	=gcc -mcpu=i386
CFLAGS	=-Wall -O -g -fstrength-reduce -fomit-frame-pointer -finline-functions -nostdinc -I../../include
CPP	=gcc -E -nostdinc -I../../include

.c.s:
	$(CC) $(CFLAGS) -S -o $*.s $<
.s.o:
	$(AS) -o $*.o $<
.c.o:
	$(CC) $(CFLAGS) -c -o $*.o $<

OBJS  = tty_io.o console.o

chr_drv.a: $(OBJS)
	$(AR) rcs chr_drv.a $(OBJS)

clean:
	rm -f core *.o *.a tmp_make
	for i in *.c;do rm -f `basename $$i .c`.s;done
