.POSIX:

PROJECTDIR = ../../../..
include $(PROJECTDIR)/scripts/rules.mk

# SYSLST is a list of backend-arch-abi-sys. First
# element of the list becomes the default target

SYSLST  =\
	amd64-sysv-linux-elf\
	z80-scc-none-none\
	i386-sysv-linux-elf\
	amd64-sysv-openbsd-elf\

TARGETS = $(BINDIR)/cc $(BINDIR)/cpp

all: $(TARGETS)

$(BINDIR)/cc: cc.o
	$(CC) $(SCC_LDFLAGS) cc.o -lscc -o $@

$(BINDIR)/cpp: cpp.sh
	trap "rm -f $$$$.sh" 0 2 3;\
	rm -f $@ ;\
	sed "s%@PREFIX@%$(PREFIX)%" < cpp.sh > $$$$.sh && \
	chmod +x $$$$.sh && \
	mv $$$$.sh $@

config.h:
	PREFIX=$(PREFIX) mkconf $(SYSLST)

dep: inc-dep

clean:
	rm -f scc scpp *.o
	rm -f $(TARGETS)
	rm -f config.h

include deps.mk
