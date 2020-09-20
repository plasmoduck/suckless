#include <stdio.h>
#include "../syscall.h"
#undef ftell

long
ftell(FILE *fp)
{
	long off;
	unsigned char *p;

	if (fp->flags & _IOERR)
		return EOF;

	if ((off = _lseek(fp->fd, 0, SEEK_CUR)) < 0) {
		fp->flags |= _IOERR;
		return EOF;
	}

	if (fp->flags & _IOREAD)
		return off - (fp->wp - fp->rp);

	if (fp->flags & _IOWRITE) {
		p = (fp->flags & _IOLBF) ? fp->lp : fp->wp;
		return off + (p - fp->buf);
	}
	return off;
}
