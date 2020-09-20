.POSIX:

PROJECTDIR = ../..
include $(PROJECTDIR)/scripts/rules.mk

OBJS = debug.o \
       die.o \
       newitem.o \
       xcalloc.o \
       xmalloc.o \
       xrealloc.o \
       xstrdup.o \
       alloc.o \
       casecmp.o \
       genhash.o

TARGET = $(LIBDIR)/libscc.a

all: $(TARGET)

$(TARGET): $(OBJS)
	mklib -o $@ $(OBJS)

dep: inc-dep

include deps.mk
