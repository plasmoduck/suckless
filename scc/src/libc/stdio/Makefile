.POSIX:
PROJECTDIR =../../..
include $(PROJECTDIR)/scripts/rules.mk
include ../rules.mk

OBJS =\
	__getc.$O\
	__putc.$O\
	_flsbuf.$O\
	_fpopen.$O\
	clearerr.$O\
	fclose.$O\
	feof.$O\
	ferror.$O\
	fgetc.$O\
	fgets.$O\
	fopen.$O\
	fprintf.$O\
	fputc.$O\
	fputs.$O\
	fread.$O\
	freopen.$O\
	fseek.$O\
	ftell.$O\
	fwrite.$O\
	getc.$O\
	getchar.$O\
	gets.$O\
	perror.$O\
	printf.$O\
	putc.$O\
	putchar.$O\
	puts.$O\
	remove.$O\
	rewind.$O\
	setbuf.$O\
	setvbuf.$O\
	snprintf.$O\
	sprintf.$O\
	__iob.$O\
	tmpnam.$O\
	vfprintf.$O\
	vsnprintf.$O\
	vsprintf.$O\
	vprintf.$O\
	_allocbuf.$O\

all: $(LIBC)

$(LIBC): $(OBJS)
	$(MKLST)

dep: inc-dep

include deps.mk
