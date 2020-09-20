.POSIX:
PROJECTDIR =../../..
include $(PROJECTDIR)/scripts/rules.mk
include ../rules.mk

OBJS =\
	localeconv.$O\
	setlocale.$O\

all: $(LIBC)

$(LIBC): $(OBJS)
	$(MKLST)

dep: inc-dep

include deps.mk
