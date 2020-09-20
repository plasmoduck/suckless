#include <sys/types.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <string.h>
#include <unistd.h>
#include <utime.h>

#include <limits.h>

#include "sys.h"

const char invalidchars[] = " ";

time_t
totime(long long t)
{
	return t;
}

char *
canonical(char *path)
{
	char *name = strrchr(path, '/');
	return (name && name[1]) ? name+1 : path;
}

int
getstat(char *fname, struct fprop *prop)
{
	struct stat st;

	if (stat(fname, &st) < 0)
		return -1;
	if (st.st_size > LONG_MAX)
		return -1;
	prop->uid = st.st_uid;
	prop->gid = st.st_gid;
	prop->mode = st.st_mode;
	prop->time = st.st_mtime;
	prop->size = st.st_size;

	return 0;
}

int
setstat(char *fname, struct fprop *prop)
{
	struct utimbuf ut = {prop->time, prop->time};

	if (chown(fname, prop->uid, prop->gid) < 0) {
		if (chown(fname, getuid(), getgid()) < 0)
			return -1;
	}
	if (chmod(fname, prop->mode) < 0)
		return -1;
	if (utime(fname, &ut) < 0)
		return -1;
	return 0;
}
