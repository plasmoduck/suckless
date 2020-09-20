.POSIX:

PROJECTDIR = ../../..
include $(PROJECTDIR)/scripts/rules.mk

SYSHDR =\
	ldflags.h\
	syscrts.h\
	sysincludes.h\
	syslibs.h\
	cstd.h\

all: $(SYSHDR)

cstd.h: cstd-$(STD).h
	cp cstd-$(STD).h $@

ldflags.h:
	cp ldflags.def.h $@

syscrts.h:
	cp syscrts.def.h $@

sysincludes.h:
	cp sysincludes.def.h $@

syslibs.h:
	cp syslibs.def.h $@

distclean:
	rm -f $(SYSHDR)
