.POSIX:

PROJECTDIR = ..
include $(PROJECTDIR)/scripts/rules.mk

LIBS  = libcrt libscc libmach
DIRS  = cmd $(LIBS)

all: cmd

cmd: $(LIBS)

$(DIRS): FORCE
	+@cd $@ && $(MAKE)

dep clean:
	$(FORALL)
