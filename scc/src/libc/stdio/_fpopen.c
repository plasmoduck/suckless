#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys.h>
#include "../syscall.h"
#include "../libc.h"
#undef fopen

FILE *
_fpopen(const char * restrict fname,
        const char * restrict mode,
        FILE * restrict fp)
{
	int i, flags, fd, rw, bin;

	flags = rw = bin = 0;

	if (mode[0] == '\0')
		goto einval;

	for (i = 1; mode[i]; ++i) {
		switch (mode[i]) {
		case '+':
			if (rw)
				goto einval;
			rw = 1;
			break;
		case 'b':
			if (bin)
				goto einval;
			bin = 1;
			break;
		default:
			goto einval;
		}
	}

	switch (mode[0]) {
	case 'a':
		flags |= O_APPEND | O_CREAT;
		goto wrflags;
	case 'w':
		flags |= O_TRUNC | O_CREAT;
	wrflags:
		flags |= (rw) ? O_RDWR : O_WRONLY;
		break;
	case 'r':
		flags = (rw) ? O_RDWR : O_RDONLY;
		break;
	default:
	einval:
		errno = EINVAL;
		return NULL;
	}

	if ((fd = _open(fname, flags, 0666)) < 0)
		return NULL;

	fp->buf = NULL;
	fp->fd = fd;

	if (!bin)
		fp->flags |= _IOTXT;

	if (flags & O_RDWR)
		fp->flags |= _IORW;
	else if (flags & O_RDONLY)
		fp->flags |= _IOREAD;
	else
		fp->flags |= _IOWRITE;

	fp->lp = fp->rp = fp->wp = NULL;

	return fp;
}
