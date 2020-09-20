.POSIX:
PROJECTDIR =../../../../..
include $(PROJECTDIR)/scripts/rules.mk
include ../../../rules.mk

GENOBJS =\
	_Exit.$O\
	_close.$O\
	_getpid.$O\
	_kill.$O\
	_lseek.$O\
	_open.$O\
	_read.$O\
	_write.$O\
	_brk.$O\
	_sigaction.$O\

GENSRC = $(GENOBJS:.$O=.s)

OBJS =\
	$(GENOBJS)\
	_getheap.$O\
	_tzone.$O\
	getenv.$O\
	raise.$O\
	signal.$O\
	time.$O\
	_sys_errlist.$O\

all: $(LIBC) $(CRT)

$(LIBC): $(OBJS)
	$(MKLST)

crt.$O: ../crt-posix.s ../openbsd/crt.s

$(GENSRC): syscall.lst
	gensys.sh $(@:.s=)

dep: inc-dep

clean:
	rm -f $(GENSRC)
	rm -f syscall _sys_errlist.c

include deps.mk
