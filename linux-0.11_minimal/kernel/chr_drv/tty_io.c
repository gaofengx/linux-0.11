#include <linux/tty.h>

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
