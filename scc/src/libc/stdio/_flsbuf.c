#include <errno.h>
#include <stdio.h>

#include "../libc.h"
#include "../syscall.h"

int
_flsbuf(FILE *fp)
{
	size_t cnt;

	if (fp->flags&_IOREAD)
		return 0;

	cnt = fp->wp - fp->buf;
	if (cnt > 0 && _write(fp->fd, fp->buf, cnt) != cnt) {
		fp->flags |= _IOERR;
		return EOF;
	}
	fp->wp = fp->buf;

	return 0;
}
