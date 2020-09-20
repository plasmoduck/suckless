#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include "../libc.h"

int
_allocbuf(FILE *fp)
{
	char *bp;

	if ((bp = malloc(BUFSIZ)) == NULL) {
		fp->flags |= _IOERR;
		errno = ENOMEM;
		return EOF;
	}
	fp->len = BUFSIZ;
	fp->rp = fp->wp = fp->buf = bp;
	fp->lp = bp + BUFSIZ;

	return 0;
}
