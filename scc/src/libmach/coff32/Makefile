.POSIX:
PROJECTDIR =../../..
include $(PROJECTDIR)/scripts/rules.mk

OBJS = coff32.o \
       coff32del.o \
       coff32new.o \
       coff32probe.o \
       coff32read.o \
       coff32strip.o \
       coff32write.o \
       coff32setidx.o \
       coff32xsetidx.o \
       coff32getidx.o \
       coff32xgetidx.o \
       coff32setidx.o \
       coff32getidx.o \
       coff32pc2line.o \
       coff32getsym.o \
       coff32getsec.o \
	coff32loadmap.o\

all: $(OBJS)

dep: inc-dep

include deps.mk
