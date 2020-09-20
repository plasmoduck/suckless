.POSIX:

PROJECTDIR = ../../..
include $(PROJECTDIR)/scripts/rules.mk

DIRS = cc1 cc2 $(DRIVER)

all: $(DIRS)

$(DIRS): FORCE
	+@cd $@ && $(MAKE)

dep clean:
	$(FORALL)
