.POSIX:

PROJECTDIR = ../../../..
include $(PROJECTDIR)/scripts/rules.mk

MORECFLAGS = -I$(INCDIR)/$(STD)

OBJS = main.o \
       parser.o \
       peep.o \
       symbol.o \
       node.o \
       code.o \
       optm.o \

TARGET  = $(LIBEXEC)/cc2-amd64-sysv \
          $(LIBEXEC)/cc2-i386-sysv \
          $(LIBEXEC)/cc2-qbe_amd64-sysv \
          $(LIBEXEC)/cc2-z80-scc \

all: $(TARGET)

$(TARGET): error.h

error.h: cc2.h
	rm -f $@;\
	trap 'r=$?;rm -f $$$$.h;exit $r' EXIT INT QUIT ;\
	awk -f generror.awk cc2.h > $$$$.h && mv $$$$.h $@

dep: inc-dep

clean:
	rm -f target/*/*.o error.h

include target/amd64-sysv/target.mk
include target/i386-sysv/target.mk
include target/qbe_amd64-sysv/target.mk
include target/qbe_arm64-sysv/target.mk
include target/z80-scc/target.mk
include deps.mk
