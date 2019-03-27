//Editplus4.0(632)´òÓ¡ÉèÖÃ:×óÓÒ±ß¾à0.8cm£¬ÏÂ±ß¾à2.6cm,×ÖÌåCourier New£¬9ºÅ
//C:\Pf\linux-0.11\linux-0.11_minimal>make
//nasm -o bootsect.bin bootsect.s
//nasm -o setup.bin setup.s
//gcc -o head.o -c head.s
//gcc -O2 -o main.o -c main.c
//ld -Ttext 0 --image-base 0x0000 -o system.exe head.o main.o  1>system.map
//Trans.exe System.exe system.bin
//[Trans] base=009A0000
//[Trans] vSize=00005014, VirtualAddress=00005000, VirtualSize=00000014
//[Trans] .text  PEOff 00000400, PEEnd 00000e00, BinOff 00001000, BinEnd 00001a00, size 00000a00
//[Trans] .data  PEOff 00000e00, PEEnd 00001400, BinOff 00002000, BinEnd 00002600, size 00000600
//[Trans] .bss   PEOff 00000000, PEEnd 00000000, BinOff 00003000, BinEnd 00003000, size 00000000 <no data>
//[Trans] .idata PEOff 00001400, PEEnd 00001600, BinOff 00005000, BinEnd 00005200, size 00000200
//
//C:\Pf\linux-0.11\linux-0.11_nano>build.exe bootsect.bin setup.bin system.bin
//Root device is (2, 29)
//Boot sector 512 bytes. offset 0x0, end 0x200
//Setup is 95 bytes. offset 0x0200, end 0x0a00
//System is 20992 bytes. offset 0x0a00, end 0x00005c00
//total size is  1474560 bytes.
#if 0
#define PAGE_SIZE 4096

long user_stack[PAGE_SIZE>>2];

struct {
	long * a;
	short b;
} stack_start = { & user_stack [PAGE_SIZE>>2] , 0x10 };

#define ORIG_VIDEO_COLS (((*(unsigned short *)0x90006) & 0xff00) >> 8)

void start_kernel(void)
{
	char *display_desc = "EGA3";
	char *display_ptr = ((char *)0xb8000) + ORIG_VIDEO_COLS * 2 - 8;

	while (*display_desc)
	{
		*display_ptr++ = *display_desc++;
		display_ptr++;
	}
	for(;;);
}
#else
/* tty.h */
#define TTY_BUF_SIZE 1024

struct tty_queue {
	unsigned long head;
	unsigned long tail;
	char buf[TTY_BUF_SIZE];
};

#define INC(a) ((a) = ((a)+1) & (TTY_BUF_SIZE-1))
#define LEFT(a) (((a).tail-(a).head-1)&(TTY_BUF_SIZE-1))
#define FULL(a) (!LEFT(a))
#define CHARS(a) (((a).head-(a).tail)&(TTY_BUF_SIZE-1))
#define GETCH(queue,c) (void)({c=(queue).buf[(queue).tail];INC((queue).tail);})
#define PUTCH(c,queue) (void)({(queue).buf[(queue).head]=(c);INC((queue).head);})

struct tty_struct {
	void (*write)(struct tty_struct * tty);
	struct tty_queue write_q;
};

void con_init(void);
void tty_init(void);
int tty_write(unsigned c, char * buf, int n);
void con_write(struct tty_struct * tty);

/* stdarg.h */
typedef char *va_list;

#define __va_rounded_size(TYPE)  \
    (((sizeof (TYPE) + sizeof (int) - 1) / sizeof (int)) * sizeof (int))

#define va_start(AP, LASTARG) 						\
    (AP = ((char *) &(LASTARG) + __va_rounded_size (LASTARG)))

void va_end (va_list);		/* Defined in gnulib */
#define va_end(AP)

#define va_arg(AP, TYPE)						\
    (AP += __va_rounded_size (TYPE),					\
    *((TYPE *) (AP - __va_rounded_size (TYPE))))
	
/* sched.c */
#define PAGE_SIZE 4096

long user_stack [ PAGE_SIZE>>2 ] ;

struct {
	long * a;
	short b;
} stack_start = { & user_stack [PAGE_SIZE>>2] , 0x10 };

/* tty_io.c */
//#include <linux/tty.h>

#define OPOST	0000001
#define ONLCR	0000004
#define _O_FLAG(tty,f)	((OPOST|ONLCR) & f)
#define O_POST(tty)	_O_FLAG((tty),0000001)
#define O_NLCR(tty)	_O_FLAG((tty),0000004)

struct tty_struct tty_table[] = {
	{
		con_write,
		{0,0,""},		/* console write-queue */
	}
};

void tty_init(void)
{
	con_init();
}

static inline unsigned char get_fs_byte(const char * addr)
{
	unsigned register char _v;

	__asm__ ("movb %%fs:%1,%0":"=r" (_v):"m" (*addr));
	return _v;
}

int tty_write(unsigned channel, char * buf, int nr)
{
	static int cr_flag=0;
	struct tty_struct * tty;
	char c, *b=buf;

	if (channel>2 || nr<0) return -1;
	tty = channel + tty_table;
	while (nr>0) {
		while (nr>0 && !FULL(tty->write_q)) {
			c=get_fs_byte(b);
			if (O_POST(tty)) {
				if (c=='\n' && !cr_flag && O_NLCR(tty)) {
					cr_flag = 1;
					PUTCH(13,tty->write_q);
					continue;
				}
			}
			b++; nr--;
			cr_flag = 0;
			PUTCH(c,tty->write_q);
		}
		tty->write(tty);
	}
	return (b-buf);
}
/* console.c */
//#include <linux/tty.h>

#define ORIG_X			(*(unsigned char *)0x90000)
#define ORIG_Y			(*(unsigned char *)0x90001)
#define ORIG_VIDEO_MODE		((*(unsigned short *)0x90006) & 0xff)
#define ORIG_VIDEO_COLS 	(((*(unsigned short *)0x90006) & 0xff00) >> 8)
#define ORIG_VIDEO_LINES	(25)

static unsigned long	video_num_columns;	/* Number of text columns	*/
static unsigned long	video_size_row;		/* Bytes per row		*/
static unsigned long	video_num_lines;	/* Number of test lines		*/
static unsigned long	video_mem_start;	/* Start of video RAM		*/
static unsigned long	video_mem_end;		/* End of video RAM (sort of)	*/
static unsigned short	video_port_reg;		/* Video register select port	*/
static unsigned short	video_port_val;		/* Video register value port	*/
static unsigned short	video_erase_char;	/* Char+Attrib to erase with	*/

static unsigned long	origin;		/* Used for EGA/VGA fast scroll	*/
static unsigned long	scr_end;	/* Used for EGA/VGA fast scroll	*/
static unsigned long	pos;
static unsigned long	x,y;
static unsigned long	top,bottom;
static unsigned long	state=0;
static unsigned char	attr=0x07;

#define outb(value,port) \
__asm__ ("outb %%al,%%dx"::"a" (value),"d" (port))


#define inb(port) ({ \
unsigned char _v; \
__asm__ volatile ("inb %%dx,%%al":"=a" (_v):"d" (port)); \
_v; \
})

#define outb_p(value,port) \
__asm__ ("outb %%al,%%dx\n" \
		"\tjmp 1f\n" \
		"1:\tjmp 1f\n" \
		"1:"::"a" (value),"d" (port))

#define inb_p(port) ({ \
unsigned char _v; \
__asm__ volatile ("inb %%dx,%%al\n" \
	"\tjmp 1f\n" \
	"1:\tjmp 1f\n" \
	"1:":"=a" (_v):"d" (port)); \
_v; \
})

static inline void gotoxy(unsigned int new_x,unsigned int new_y)
{
	if (new_x > video_num_columns || new_y >= video_num_lines)
		return;

	x=new_x;
	y=new_y;
	pos=origin + y*video_size_row + (x<<1);
}

static inline void set_origin(void)
{
	outb_p(12, video_port_reg);
	outb_p(0xff&((origin-video_mem_start)>>9), video_port_val);
	outb_p(13, video_port_reg);
	outb_p(0xff&((origin-video_mem_start)>>1), video_port_val);
}

static void scrup(void)
{
	if (!top && bottom == video_num_lines) {
		origin += video_size_row;
		pos += video_size_row;
		scr_end += video_size_row;
		if (scr_end > video_mem_end) {
			__asm__("cld\n\t"
				"rep\n\t"
				"movsl\n\t"
				"movl _video_num_columns,%1\n\t"
				"rep\n\t"
				"stosw"
				::"a" (video_erase_char),
				"c" ((video_num_lines-1)*video_num_columns>>1),
				"D" (video_mem_start),
				"S" (origin)
				);
			scr_end -= origin-video_mem_start;
			pos -= origin-video_mem_start;
			origin = video_mem_start;
		} else {
			__asm__("cld\n\t"
				"rep\n\t"
				"stosw"
				::"a" (video_erase_char),
				"c" (video_num_columns),
				"D" (scr_end-video_size_row)
				);
		}
		set_origin();
	}
}

static void lf(void)
{
	if (y+1<bottom) {
		y++;
		pos += video_size_row;
		return;
	}
	scrup();
}

static void cr(void)
{
	pos -= x<<1;
	x=0;
}

static inline void set_cursor(void)
{
	outb_p(14, video_port_reg);
	outb_p(0xff&((pos-video_mem_start)>>9), video_port_val);
	outb_p(15, video_port_reg);
	outb_p(0xff&((pos-video_mem_start)>>1), video_port_val);
}

void con_write(struct tty_struct * tty)
{
	int nr;
	char c;

	nr = CHARS(tty->write_q);
	while (nr--) {
		GETCH(tty->write_q,c);
		switch(state) {
		case 0:
			if (c>31 && c<127) {
				if (x>=video_num_columns) {
					x -= video_num_columns;
					pos -= video_size_row;
					lf();
				}
				__asm__("movb _attr,%%ah\n\t"
					"movw %%ax,%1\n\t"
					::"a" (c),"m" (*(short *)pos)
					);
				pos += 2;
				x++;
			}
			else if (c==10 || c==11 || c==12)
				lf();
			else if (c==13)
				cr();
		}
	}
	set_cursor();
}

void con_init(void)
{
	register unsigned char a;
	char *display_desc = "????";
	char *display_ptr;

	video_num_columns = ORIG_VIDEO_COLS;
	video_size_row = video_num_columns * 2;
	video_num_lines = ORIG_VIDEO_LINES;
	video_erase_char = 0x0720;
	
	video_mem_start = 0xb8000;
	video_port_reg	= 0x3d4;
	video_port_val	= 0x3d5;
	video_mem_end = 0xbc000;
	display_desc = "EGAc";

	display_ptr = ((char *)video_mem_start) + video_size_row - 8;
	while (*display_desc)
	{
		*display_ptr++ = *display_desc++;
		display_ptr++;
	}
	
	origin	= video_mem_start;
	scr_end	= video_mem_start + video_num_lines * video_size_row;
	top	= 0;
	bottom	= video_num_lines;

	gotoxy(ORIG_X,ORIG_Y);

	outb_p(inb_p(0x21)&0xfd,0x21);
	a=inb_p(0x61);
	outb_p(a|0x80,0x61);
	outb(a,0x61);
}
/* vsprintf.c */
//#include <stdarg.h>

#define is_digit(c)	((c) >= '0' && (c) <= '9')

static int skip_atoi(const char **s)
{
	int i=0;

	while (is_digit(**s))
		i = i*10 + *((*s)++) - '0';
	return i;
}

#define ZEROPAD	1		/* pad with zero */
#define SIGN	2		/* unsigned/signed long */
#define PLUS	4		/* show plus */
#define SPACE	8		/* space if plus */
#define LEFTJ	16		/* left justified */

#define do_div(n,base) ({ \
	int __res; \
	__asm__("divl %4":"=a" (n),"=d" (__res):"0" (n),"1" (0),"r" (base)); \
	__res; })

static char* number(char * str, int num, int base, int size, int precision, int type)
{
	char c,sign,tmp[36];
	const char *digits="0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	int i;

	if (type&LEFTJ)
	 	type &= ~ZEROPAD;
	if (base<2 || base>36)
		return 0;

	i=0;
	while (num!=0)
		tmp[i++]=digits[do_div(num,base)];
	size -= precision;

	while(size-- > 0)
		*str++ = ' ';
	if (sign)
		*str++ = sign;

	while(size-- > 0)
		*str++ = c;

	while(i-- > 0)
		*str++ = tmp[i];

	return str;
}

extern inline int strlen(const char * s)
{
	register int __res ;
	__asm__("cld\n\t"
		"repne\n\t"
		"scasb\n\t"
		"notl %0\n\t"
		"decl %0"
		:"=c" (__res):"D" (s),"a" (0),"0" (0xffffffff));
	return __res;
}

int vsprintf(char *buf, const char *fmt, va_list args)
{
	int len;
	int i;
	char * str;
	char *s;

	int flags;		/* flags to number() */

	int field_width;	/* width of output field */

	for (str=buf ; *fmt ; ++fmt) {
		if (*fmt != '%') {
			*str++ = *fmt;
			continue;
		}
			
		flags = 0;
		repeat:
			++fmt;		/* this also skips first '%' */
			switch (*fmt) {
				case '-': flags |= LEFTJ; goto repeat;
				case '+': flags |= PLUS; goto repeat;
				case ' ': flags |= SPACE; goto repeat;
				case '0': flags |= ZEROPAD; goto repeat;
				}
		
		field_width = skip_atoi(&fmt);

		switch (*fmt) {
		case 's':
			s = va_arg(args, char *);
			len = strlen(s);

			if (!(flags & LEFTJ))
				while (len < field_width--)
					*str++ = ' ';
			for (i = 0; i < len; ++i)
				*str++ = *s++;

			break;

		case 'X':
			str = number(str, va_arg(args, unsigned long), 16, field_width, -1, flags);
			break;

		case 'd':
			str = number(str, va_arg(args, unsigned long), 10, field_width, -1, flags);
			break;

		default:

			break;
		}
	}
	*str = '\0';
	return str-buf;
}
/* printk.c */
//#include <stdarg.h>

static char buf[1024];

extern int vsprintf(char * buf, const char * fmt, va_list args);

int printk(const char *fmt, ...)
{
	va_list args;
	int i;

	va_start(args, fmt);
	i = vsprintf(buf, fmt, args);
	va_end(args);
	__asm__("push %%fs\n\t"
		"push %%ds\n\t"
		"pop %%fs\n\t"
		"pushl %0\n\t"
		"pushl $_buf\n\t"
		"pushl $0\n\t"
		"call _tty_write\n\t"
		"addl $8,%%esp\n\t"
		"popl %0\n\t"
		"pop %%fs"
		::"r" (i):"ax","cx","dx");
	return i;
}

/* main.c */
int printk(const char * fmt, ...);

void start_kernel(void)
{
	tty_init();
	printk("hello %s, %d, %f.\n", "world", 1, 3.5);
	printk("hello %s, %d, %f.\n", "linus ", 2, 4);

	#define ORIG_VIDEO_EGA_BX	(*(unsigned short *)0x9000a)
	printk("ORIG_VIDEO_EGA_BX=%02X\n", ORIG_VIDEO_EGA_BX);
	for(;;);
}
#endif
