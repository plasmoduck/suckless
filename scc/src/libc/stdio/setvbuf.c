#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#include "../libc.h"
#undef setvbuf

int
setvbuf(FILE * restrict fp, char * restrict buf, int mode, size_t size)
{
	int flags;
	char *p;
	size_t l;

	if (_flsbuf(fp) == EOF)
		return EOF;

	if (buf)
		p = buf, l = size;
	else
		p = fp->buf, l = fp->len;

	switch (mode) {
	case _IONBF:
		l = sizeof(fp->unbuf);
		p = fp->unbuf;
	case _IOLBF:
	case _IOFBF:
		fp->rp = fp->wp = p;
		fp->lp = p + l;
		break;
	default:
		errno = EINVAL;
		return EOF;
	}

	flags = fp->flags;
	if (flags&_IOALLOC && (buf || mode == _IONBF)) {
		free(fp->buf);
		flags &= ~_IOALLOC;
	}

	fp->buf = p;
	fp->len = l;
	flags &= ~(_IONBF | _IOLBF | _IOFBF);
	flags |= mode;
	fp->flags = flags;

	return 0;
}
