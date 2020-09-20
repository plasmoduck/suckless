.POSIX:
PROJECTDIR =../..
include $(PROJECTDIR)/scripts/rules.mk

TARGET = $(LIBDIR)/libmach.a

OBJS =\
	mach.o\
	newobj.o \
	delobj.o\
	objpos.o\
	archive.o\
	armember.o\
	objtype.o\
	readobj.o\
	writeobj.o\
	getsym.o\
	getsec.o\
	rebase.o\
	loadmap.o\
	newmap.o\
	setmap.o\
	findsec.o\
	strip.o\
	pc2line.o\
	pack.o\
	unpack.o\
	setindex.o\
	getindex.o\

DIRS = coff32


all: $(OBJS) $(DIRS) objlst.mk
	+@$(MAKE) -f Makefile.mach

$(DIRS): FORCE
	+@cd $@ && $(MAKE)

objlst.mk: $(OBJS) $(DIRS)
	mklst $@

dep: inc-dep
	$(FORALL)

clean:
	$(FORALL)
	rm -f $(TBLS) objlst.mk


include deps.mk
