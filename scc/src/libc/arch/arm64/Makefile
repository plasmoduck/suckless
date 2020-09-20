.POSIX:
PROJECTDIR =../../../..
include $(PROJECTDIR)/scripts/rules.mk
include ../../rules.mk

OBJS =\
	longjmp.$O\
	setjmp.$O\
	memchr.$O\
	memcmp.$O\
	memcpy.$O\
	memmove.$O\
	memset.$O\

DIRS = linux

all: $(LIBC) $(SYS)

$(LIBC): $(OBJS)
	$(MKLST)

$(SYS): FORCE
	+@cd $@ && $(MAKE)

dep: inc-dep

clean:
	$(FORALL)

include deps.mk
