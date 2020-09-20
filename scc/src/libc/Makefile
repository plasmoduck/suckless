.POSIX:
PROJECTDIR =../..
include $(PROJECTDIR)/scripts/rules.mk
include rules.mk

DIRS =\
	arch\
	assert\
	ctype\
	locale\
	stdio\
	stdlib\
	string\
	time\

all: $(LIBC)

$(LIBC): $(DIRS)
	mklib -o $@ `cat $(LIBCLST)`;\

$(DIRS): rm-lst
	+@cd $@ && $(MAKE)

rm-lst: FORCE
	rm -f $(LIBCLST)

dep:
	$(FORALL)

clean: rm-lst
	$(FORALL)
