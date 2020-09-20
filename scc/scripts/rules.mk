CONF=amd64-linux
TOOL=unix
HOST=unix
include $(PROJECTDIR)/config/conf/$(CONF).mk
include $(PROJECTDIR)/config/tool/$(TOOL).mk
include $(PROJECTDIR)/config/host/$(HOST).mk

LIBDIR     = $(PROJECTDIR)/lib/scc
SCRIPTDIR  = $(PROJECTDIR)/scripts
INCDIR     = $(PROJECTDIR)/include

BINDIR     = $(PROJECTDIR)/bin
LIBEXEC    = $(PROJECTDIR)/libexec/scc
CRTDIR     = $(PROJECTDIR)/lib/scc
LIBCDIR    = $(CRTDIR)/$(ARCH)-$(SYS)
ENVIRON    = $(SCRIPTDIR)/env.sh

INCLUDE    = -I$(INCDIR)/scc

STD = c99
CC  = $(CROSS_COMPILE)$(COMP)
AS  = $(CROSS_COMPILE)$(ASM)
LD  = $(CROSS_COMPILE)$(LINKER)
RL  = $(CROSS_COMPILE)$(RANLIB)
AR  = $(CROSS_COMPILE)$(ARCHIVE)

SCC_CFLAGS =\
	$(MORECFLAGS)\
	$(TOOLCFLAGS)\
	$(HOSTCFLAGS)\
	$(SYSCFLAGS)\
	$(INCLUDE)\
	-g\
	$(CFLAGS)

SCC_LDFLAGS =\
	$(MORELFLAGS)\
	$(TOOLLDFLAGS)\
	$(HOSTLDFLAGS)\
	$(SYSLDFLAGS)\
	-L$(LIBDIR)\
	-g \
	$(LDFLAGS)

SCC_ASFLAGS =\
	$(MOREASFLAGS)\
	$(TOOLASFLAGS)\
	$(HOSTASFLAGS)\
	$(SYSASFLAGS)\
	$(ASFLAGS)

# helper macro to run over all the directories
FORALL = +@set -e ;\
	pwd=$$PWD; \
	. $(ENVIRON); \
	for i in $(DIRS); \
	do \
		cd $$i; \
		$(MAKE) $@; \
		cd $$pwd; \
	done

.o:
	$(CC) $(SCC_LDFLAGS) -o $@ $< $(LIBS)

.s.o:
	$(AS) $(SCC_ASFLAGS) $< -o $@

.c.o:
	$(CC) $(SCC_CFLAGS) -o $@ -c $<

all:

dep:

distclean:

inc-dep: FORCE
	mkdep

clean: clean-helper

clean-helper:
	rm -f *.o $(OBJS) $(TARGET)

FORCE:
