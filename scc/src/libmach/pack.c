#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include <scc/mach.h>
#include "libmach.h"

static int
lpack(unsigned char *dst, char *fmt, va_list va)
{
	unsigned char *bp, *cp;
	unsigned s;
	unsigned long l;
	unsigned long long q;
	int n;

	bp = dst;
	while (*fmt) {
		switch (*fmt++) {
		case '\'':
			n = atoi(fmt);
			while (isdigit(*fmt))
				fmt++;
			cp = va_arg(va, unsigned char *);
			while (n--)
				*bp++ = *cp++;
			break;
		case 'c':
			*bp++ = va_arg(va, unsigned);
			break;
		case 's':
			s = va_arg(va, unsigned);
			*bp++ = s;
			*bp++ = s >> 8;
			break;
		case 'l':
			l = va_arg(va, unsigned long);
			*bp++ = l;
			*bp++ = l >> 8;
			*bp++ = l >> 16;
			*bp++ = l >> 24;
			break;
		case 'q':
			q = va_arg(va, unsigned long long);
			*bp++ = q;
			*bp++ = q >> 8;
			*bp++ = q >> 16;
			*bp++ = q >> 24;
			*bp++ = q >> 32;
			*bp++ = q >> 40;
			*bp++ = q >> 48;
			*bp++ = q >> 56;
			break;
		default:
			va_end(va);
			return -1;
		}
	}

	return bp - dst;
}

static int
bpack(unsigned char *dst, char *fmt, va_list va)
{
	unsigned char *bp, *cp;
	unsigned s;
	unsigned long l;
	unsigned long long q;
	int n;

	bp = dst;
	while (*fmt) {
		switch (*fmt++) {
		case '\'':
			n = atoi(fmt);
			while (isdigit(*fmt))
				fmt++;
			cp = va_arg(va, unsigned char *);
			while (n--)
				*bp++ = *cp++;
			break;
		case 'c':
			*bp++ = va_arg(va, unsigned);
			break;
		case 's':
			s = va_arg(va, unsigned);
			*bp++ = s >> 8;
			*bp++ = s;
			break;
		case 'l':
			l = va_arg(va, unsigned long);
			*bp++ = l >> 24;
			*bp++ = l >> 16;
			*bp++ = l >> 8;
			*bp++ = l;
			break;
		case 'q':
			q = va_arg(va, unsigned long long);
			*bp++ = q >> 56;
			*bp++ = q >> 48;
			*bp++ = q >> 40;
			*bp++ = q >> 32;
			*bp++ = q >> 24;
			*bp++ = q >> 16;
			*bp++ = q >> 8;
			*bp++ = q;
			break;
		default:
			va_end(va);
			return -1;
		}
	}

	return bp - dst;
}

int
pack(int order, unsigned char *dst, char *fmt, ...)
{
	int r;
	int (*fn)(unsigned char *dst, char *fmt, va_list va);
	va_list va;

	va_start(va, fmt);
	fn = (order == LITTLE_ENDIAN) ? lpack : bpack;
	r = (*fn)(dst, fmt, va);
	va_end(va);

	return r;
}
