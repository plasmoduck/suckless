.POSIX:
PROJECTDIR =../../../../..
include $(PROJECTDIR)/scripts/rules.mk
include ../../../rules.mk

GENOBJS =\
	_Exit.$O\
	_read.$O\
	_write.$O\
	_open.$O\
	_close.$O\
	_waitpid.$O\
	_create.$O\
	_link.$O\
	_unlink.$O\
	_getpid.$O\
	_brk.$O\

GENSRC = $(GENOBJS:.$O=.s)

OBJS  = \
	$(GENOBJS)\
	_cerrno.$O\
	_getheap.$O\

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

include deps.mk
