#include <ctype.h>
#include <limits.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <wchar.h>
#undef vfprintf

enum {
	LONG     = 1 << 0,
	LLONG    = 1 << 1,
	SHORT    = 1 << 2,
	CHAR     = 1 << 3,
	SIZET    = 1 << 4,
	PTRDIFF  = 1 << 5,
	INTMAX   = 1 << 6,
	VOIDPTR  = 1 << 7,
	UNSIGNED = 1 << 8,
	ALTFORM  = 1 << 9,
};

#define MAXPREC    50

struct conv {
	int sign;
	int prec;
	char *digs;
	int base;
};

static uintmax_t
getnum(va_list *va, int flags, int *sign)
{
	uintmax_t uval;
	intmax_t val;

	if (flags & CHAR) {
		val = va_arg(*va, int);
		uval = (unsigned char) val;
	} else if (flags & SHORT) {
		val = va_arg(*va, int);
		uval = (unsigned short) val;
	} else if (flags & LONG) {
		val = va_arg(*va, long);
		uval = (unsigned long) val;
	} else if (flags & LLONG) {
		val = va_arg(*va, long long);
		uval = (unsigned long long) val;
	} else if (flags & SIZET) {
		uval = va_arg(*va, size_t);
	} else if (flags & INTMAX) {
		val = va_arg(*va, intmax_t);
		uval = (uintmax_t) val;
	} else if (flags & VOIDPTR) {
		uval = (uintmax_t) va_arg(*va, void *);
	} else {
		val = va_arg(*va, int);
		uval = (unsigned) val;
	}

	if ((flags & UNSIGNED) == 0 && val < 0) {
		*sign = '-';
		uval = -val;
	}
	return uval;
}

static char *
numtostr(uintmax_t val, int flags, struct conv *conv, char *buf)
{
	char *buf0 = buf;
	int base = conv->base, prec = conv->prec;
	uintmax_t oval = val;

	if (prec == -1)
		prec = 1;

	for (*buf = '\0'; val > 0; val /= base)
		*--buf = conv->digs[val % base];
	while (buf0 - buf < prec)
		*--buf = '0';

	if (flags & ALTFORM) {
		if (base == 8 && *buf != '0') {
			*--buf = '0';
		} else if (base == 16 && oval != 0) {
			*--buf = conv->digs[16];
			*--buf = '0';
		}
	}
	if (conv->sign)
		*--buf = conv->sign;

	return buf;
}

static void
savecnt(va_list *va, int flags, int cnt)
{
	if (flags & CHAR)
		*va_arg(*va, char*) = cnt;
	else if (flags & SHORT)
		*va_arg(*va, short*) = cnt;
	else if (flags & LONG)
		*va_arg(*va, long*) = cnt;
	else if (flags & LLONG)
		*va_arg(*va, long long*) = cnt;
	else if (flags & SIZET)
		*va_arg(*va, size_t*) = cnt;
	else if (flags & INTMAX)
		*va_arg(*va, intmax_t*) = cnt;
	else
		*va_arg(*va, int*) = cnt;
}

static size_t
wstrout(wchar_t *ws, size_t len, int width, int fill, FILE * restrict fp)
{
	int left = 0, adjust;
	size_t cnt = 0;
	wchar_t wc;
#if 0

	if (width < 0) {
		left = 1;
		width = -width;
	}

	len *= sizeof(wchar_t);
	adjust = (len < width) ? width - len : 0;
	cnt = adjust + len;
	if (left)
		adjust = -adjust;

	for ( ; adjust > 0; adjust++)
		putc(fill, fp);

	while (wc = *ws++)
		putwc(wc, fp);

	for ( ; adjust < 0; adjust--)
		putc(' ', fp);
#endif
	return cnt;
}

static size_t
strout(char *s, size_t len, int width, int fill, FILE * restrict fp)
{
	int left = 0, adjust, ch, prefix;
	size_t cnt = 0;

	if (width < 0) {
		left = 1;
		width = -width;
	}

	adjust = (len < width) ? width - len : 0;
	cnt = adjust + len;
	if (left)
		adjust = -adjust;

	if (fill == '0') {
		if (*s == '-' || *s == '+')
			prefix = 1;
		else if (*s == '0' && toupper(s[1]) == 'X')
			prefix = 2;
		else
			prefix = 0;
		while (prefix--) {
			putc(*s++, fp);
			--len;
		}
	}

	for ( ; adjust > 0; adjust--)
		putc(fill, fp);

	while (ch = *s++)
		putc(ch, fp);

	for ( ; adjust < 0; adjust++)
		putc(' ', fp);

	return cnt;
}

static size_t
strnlen(const char *s, size_t maxlen)
{
	size_t n;

	for (n = 0; n < maxlen && *s++; ++n)
		;
	return n;
}

int
vfprintf(FILE * restrict fp, const char * restrict fmt, va_list va)
{
	int ch, n, flags, width, left, fill, cnt = 0;
	size_t inc, len;
	char *s;
	wchar_t *ws;
	struct conv conv;
	char buf[MAXPREC+1];
	wchar_t wbuf[2];
	va_list va2;

	va_copy(va2, va);
	for (cnt = 0; ch = *fmt++; cnt += inc) {
		if (ch != '%') {
			putc(ch, fp);
			inc = 1;
			continue;
		}

		fill = ' ';
		left = flags = width =  0;
		conv.prec = -1;
		conv.base = 10;
		conv.sign = '\0';
		conv.digs = "0123456789ABCDEFX";

flags:
		switch (*fmt++) {
		case ' ':
			if (conv.sign == '\0')
				conv.sign = ' ';
			goto flags;
		case '+':
			conv.sign = '+';
			goto flags;
		case '#':
			flags |= ALTFORM;
			goto flags;
		case '.':
			if (*fmt == '*') {
				fmt++;
				n = va_arg(va2, int);
			} else {
				for (n = 0; isdigit(ch = *fmt); fmt++)
					n = n * 10 + ch - '0';
			}
			if (n > MAXPREC)
				n = MAXPREC;
			if (n > 0)
				conv.prec = n;
			goto flags;
		case '*':
			width = va_arg(va2, int);
			goto flags;
		case '-':
			left = 1;
			++fmt;
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
			--fmt;
			for (n = 0; isdigit(ch = *fmt); ++fmt)
				n = n * 10 + ch - '0';
			if (left)
				n = -n;
			width = n;
			goto flags;
		case '0':
			fill = '0';
			goto flags;
		case 'l':
			flags += LONG;
			goto flags;
		case 'h':
			flags += SHORT;
			goto flags;
		case '%':
			ch = '%';
			goto cout;
		case 'c':
			if (flags & LONG) {
				wbuf[0] = va_arg(va2, wint_t);
				wbuf[1] = L'\0';
				ws = wbuf;
				len = 1;
				goto wstrout;
			}
			ch = va_arg(va2, int);
		cout:
			buf[0] = ch;
			buf[1] = '\0';
			s = buf;
			len = 1;
			goto strout;
		case 'j':
			flags |= INTMAX;
			goto flags;
		case 't':
			flags |= PTRDIFF;
			goto flags;
		case 'z':
			flags |= SIZET;
			goto flags;
		case 'u':
			flags |= UNSIGNED;
		case 'i':
		case 'd':
			conv.base = 10;
			goto numeric;
		case 'p':
			flags |= VOIDPTR | ALTFORM;
			goto numeric16;
		case 'x':
			conv.digs = "0123456789abcdefx";
		case 'X':
		numeric16:
			conv.base = 16;
			flags |= UNSIGNED;
			goto numeric;
		case 'o':
			conv.base = 8;
			flags |= UNSIGNED;
		numeric:
			if (conv.prec != -1)
				fill = ' ';
			s = numtostr(getnum(&va2, flags, &conv.sign),
			             flags,
			             &conv,
			             &buf[MAXPREC]);
			len = &buf[MAXPREC] - s;
			goto strout;
		case 'L':
		case 'a':
		case 'A':
		case 'e':
		case 'E':
		case 'f':
		case 'g':
		case 'G':
			/* TODO */
		case 's':
			if (flags & LONG) {
				ws = va_arg(va2, wchar_t *);
				/* len = wcsnlen(ws, conv.prec); */
				goto wstrout;
			} else {
				s = va_arg(va2, char *);
				if ((len = strlen(s)) > conv.prec)
					len = conv.prec;
				goto strout;
			}
		wstrout:
			inc = wstrout(ws, len, width, fill, fp);
			break;
		strout:
			inc = strout(s, len, width, fill, fp);
			break;
		case 'n':
			savecnt(&va2, flags, cnt);
			break;
		case '\0':
			goto out_loop;
		}
	}

out_loop:
	return (ferror(fp)) ? EOF : cnt;
}
