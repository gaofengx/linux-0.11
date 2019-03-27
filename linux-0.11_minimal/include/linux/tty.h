#ifndef _TTY_H
#define _TTY_H

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

#endif
