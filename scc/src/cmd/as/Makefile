.POSIX:

PROJECTDIR = ../../..
include $(PROJECTDIR)/scripts/rules.mk

MORECFLAGS = -I$(INCDIR)/$(STD)

OBJS = \
	main.o\
	symbol.o\
	ins.o\
	parser.o\
	expr.o\

TARGET =\
	$(LIBEXEC)/as-powerpc64\
	$(LIBEXEC)/as-powerpc\
	$(LIBEXEC)/as-amd64\
	$(LIBEXEC)/as-i386\
	$(LIBEXEC)/as-i286\
	$(LIBEXEC)/as-z80 \

all: $(TARGET)

$(TARGET): $(LIBDIR)/libscc.a

dep: inc-dep

clean:
	rm -f target/*/*.o target/*/*tbl.c

include target/powerpc/powerpc64.mk
include target/powerpc/powerpc.mk
include target/x86/amd64.mk
include target/x86/i386.mk
include target/x86/i286.mk
include target/x80/z80.mk
include deps.mk
