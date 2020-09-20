.POSIX:
PROJECTDIR =../../../../..
include $(PROJECTDIR)/scripts/rules.mk
include ../../../rules.mk

GENOBJS  =\
	_Exit.$O\
	_close.$O\
	_brk.$O\
	_getpid.$O\
	_kill.$O\
	_lseek.$O\
	_openat.$O\
	_read.$O\
	_write.$O\
	_sigaction.$O\

GENSRC = $(GENOBJS:.$O=.s)

OBJS =\
	$(GENOBJS)\
	getenv.$O\
	raise.$O\
	signal.$O\
	time.$O\
	_tzone.$O\
	_open.$O\
	_getheap.$O\
	_cerrno.$O\
	_sys_errlist.$O\

all: $(LIBC) $(CRT)

$(LIBC): $(OBJS)
	$(MKLST)

crt.$O: ../crt-posix.s

$(GENSRC): syscall.lst
	gensys.sh $(@:.s=)

clean:
	rm -f $(GENSRC)
	rm -f syscall _sys_errlist.c
