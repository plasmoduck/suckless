#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include <scc/mach.h>
#include "libmach.h"

static int
lunpack(unsigned char *src, char *fmt, va_list va)
{
	unsigned char *bp, *cp;
	unsigned short *sp;
	unsigned s;
	unsigned long *lp, l;
	unsigned long long *qp, q;
	int n;

	bp = src;
	while (*fmt) {
		switch (*fmt++) {
		case '\'':
			n = atoi(fmt);
			while (isdigit(*fmt))
				fmt++;
			cp = va_arg(va, unsigned char *);
			while (n--)
				*cp++ = *bp++;
			break;
		case 'c':
			cp = va_arg(va, unsigned char *);
			*cp = *bp++;
			break;
		case 's':
			sp = va_arg(va, unsigned short *);
			s =  (unsigned) *bp++;
			s |= (unsigned) *bp++ << 8;
			*sp = s;
			break;
		case 'l':
			lp = va_arg(va, unsigned long *);
			l = (unsigned long) *bp++;
			l |= (unsigned long) *bp++ << 8;
			l |= (unsigned long) *bp++ << 16;
			l |= (unsigned long) *bp++ << 24;
			*lp = l;
			break;
		case 'q':
			qp = va_arg(va, unsigned long long *);
			q = (unsigned long long) *bp++;
			q |= (unsigned long long) *bp++ << 8;
			q |= (unsigned long long) *bp++ << 16;
			q |= (unsigned long long) *bp++ << 24;
			q |= (unsigned long long) *bp++ << 32;
			q |= (unsigned long long) *bp++ << 40;
			q |= (unsigned long long) *bp++ << 48;
			q |= (unsigned long long) *bp++ << 56;
			*qp = q;
			break;
		default:
			va_end(va);
			return -1;
		}
	}

	return bp - src;
}

static int
bunpack(unsigned char *src, char *fmt, va_list va)
{
	unsigned char *bp, *cp;
	unsigned short *sp;
	unsigned s;
	unsigned long *lp, l;
	unsigned long long *qp, q;
	int n;

	bp = src;
	while (*fmt) {
		switch (*fmt++) {
		case '\'':
			n = atoi(fmt);
			while (isdigit(*fmt))
				fmt++;
			cp = va_arg(va, unsigned char *);
			while (n--)
				*cp++ = *bp++;
			break;
		case 'c':
			cp = va_arg(va, unsigned char *);
			*cp = *bp++;
			break;
		case 's':
			sp = va_arg(va, unsigned short *);
			s =  (unsigned) *bp++ << 8;
			s |= (unsigned) *bp++;
			*sp = s;
			break;
		case 'l':
			lp = va_arg(va, unsigned long *);
			l =  (unsigned long) *bp++ << 24;
			l |= (unsigned long) *bp++ << 16;
			l |= (unsigned long) *bp++ << 8;
			l |= (unsigned long) *bp++;
			*lp = l;
			break;
		case 'q':
			qp = va_arg(va, unsigned long long *);
			q =  (unsigned long long) *bp++ << 56;
			q |= (unsigned long long) *bp++ << 48;
			q |= (unsigned long long) *bp++ << 40;
			q |= (unsigned long long) *bp++ << 32;
			q |= (unsigned long long) *bp++ << 24;
			q |= (unsigned long long) *bp++ << 16;
			q |= (unsigned long long) *bp++ << 8;
			q |= (unsigned long long) *bp++;
			*qp = q;
			break;
		default:
			va_end(va);
			return -1;
		}
	}

	return bp - src;
}

int
unpack(int order, unsigned char *src, char *fmt, ...)
{
	int r;
        int (*fn)(unsigned char *dst, char *fmt, va_list va);
        va_list va;

        va_start(va, fmt);
        fn = (order == LITTLE_ENDIAN) ? lunpack : bunpack;
        r = (*fn)(src, fmt, va);
        va_end(va);

        return r;
}
