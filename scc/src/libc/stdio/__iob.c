#include <stdio.h>

FILE __iob[FOPEN_MAX] = {
	{
		.fd = 0,
		.flags = _IOREAD,
	},
	{
		.fd = 1,
		.flags = _IOWRITE | _IOLBF,
	},
	{
		.fd = 2,
		.buf = stderr->unbuf,
		.len = sizeof(stderr->unbuf),
		.flags = _IOWRITE | _IONBF,
		.rp = stderr->unbuf,
		.wp = stderr->unbuf,
	},
};
