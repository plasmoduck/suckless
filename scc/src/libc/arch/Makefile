.POSIX:
PROJECTDIR =../../..
include $(PROJECTDIR)/scripts/rules.mk
include ../rules.mk

DIRS =\
	amd64\
	arm64\
	arm\
	i386\
	ppc\

all:
	+@cd $(ARCH) && $(MAKE)

dep clean:
	$(FORALL)
