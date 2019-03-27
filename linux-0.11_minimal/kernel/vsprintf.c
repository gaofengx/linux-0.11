#include <stdarg.h>

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
#define LEFT	16		/* left justified */

#define do_div(n,base) ({ \
	int __res; \
	__asm__("divl %4":"=a" (n),"=d" (__res):"0" (n),"1" (0),"r" (base)); \
	__res; })

static char * number(char * str, int num, int base, int size, int precision, int type)
{
	char c,sign,tmp[36];
	const char *digits="0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	int i;

	if (type&LEFT)
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
				case '-': flags |= LEFT; goto repeat;
				case '+': flags |= PLUS; goto repeat;
				case ' ': flags |= SPACE; goto repeat;
				case '0': flags |= ZEROPAD; goto repeat;
				}
		
		field_width = skip_atoi(&fmt);

		switch (*fmt) {
		case 's':
			s = va_arg(args, char *);
			len = strlen(s);

			if (!(flags & LEFT))
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
