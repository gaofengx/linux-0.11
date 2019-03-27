#include <linux/tty.h>

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

