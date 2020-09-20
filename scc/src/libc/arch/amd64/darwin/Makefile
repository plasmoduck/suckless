.POSIX:
PROJECTDIR =../../../../..
include $(PROJECTDIR)/scripts/rules.mk
include ../../../rules.mk

GENOBJS =\
	__Exit.$O\
	__close.$O\
	__getpid.$O\
	__kill.$O\
	__lseek.$O\
	__open.$O\
	__read.$O\
	__write.$O\
	__sigaction.$O\

GENSRC = $(GENOBJS:.$O=.s)

OBJS  =\
	$(GENOBJS)\
	_getheap.$O\
	_tzone.$O\
	getenv.$O\
	raise.$O\
	signal.$O\
	time.$O\
	_sys_errlist.$O\

# Rules

all: $(LIBC) $(CRT)

$(LIBC): $(OBJS)
	$(MKLST)

crt.$O: ../crt-posix.s

$(GENSRC): syscall.lst
	gensys.sh $(@:.s=)

dep: inc-dep

clean:
	rm -f $(GENSRC)
	rm -f syscall _sys_errlist.c

include deps.mk
