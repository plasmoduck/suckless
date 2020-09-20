.POSIX:

PROJECTDIR = ../..
include $(PROJECTDIR)/scripts/rules.mk

TARGET =\
	$(BINDIR)/nm\
	$(BINDIR)/ar\
	$(BINDIR)/strip\
	$(BINDIR)/size\
	$(BINDIR)/ranlib\
	$(BINDIR)/objdump\
	$(BINDIR)/objcopy\
	$(BINDIR)/addr2line\

DIRS   = ld as cc

LIBMACH = $(LIBDIR)/libmach.a
LIBSCC  = $(LIBDIR)/libscc.a

all: $(TARGET) $(DIRS)

$(DIRS): FORCE
	+@cd $@ && $(MAKE)

$(BINDIR)/nm: nm.o $(LIBMACH) $(LIBSCC)
	$(CC) $(SCC_LDFLAGS) nm.o -lmach -lscc -o $@

$(BINDIR)/strip: strip.o $(LIBMACH) $(LIBSCC)
	$(CC) $(SCC_LDFLAGS) strip.o -lmach -lscc -o $@

$(BINDIR)/size: size.o $(LIBMACH) $(LIBSCC)
	$(CC) $(SCC_LDFLAGS) size.o -lmach -lscc -o $@

$(BINDIR)/ranlib: ranlib.o $(DRIVER).o $(LIBMACH) $(LIBSCC)
	$(CC) $(SCC_LDFLAGS) ranlib.o $(DRIVER).o -lmach -lscc -o $@

$(BINDIR)/objdump: objdump.o $(LIBMACH)
	$(CC) $(SCC_LDFLAGS) objdump.o -lmach -o $@

$(BINDIR)/objcopy: objcopy.o $(LIBMACH)
	$(CC) $(SCC_LDFLAGS) objcopy.o -lmach -o $@

$(BINDIR)/addr2line: addr2line.o $(LIBMACH) $(LIBSCC)
	$(CC) $(SCC_LDFLAGS) addr2line.o -lmach -lscc -o $@

$(BINDIR)/ar: ar.o $(DRIVER).o
	$(CC) $(SCC_LDFLAGS) ar.o $(DRIVER).o -o $@

dep: inc-dep

dep clean:
	$(FORALL)

include deps.mk
