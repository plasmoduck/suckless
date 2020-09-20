.POSIX:

PROJECTDIR = ../../../..
include $(PROJECTDIR)/scripts/rules.mk

OBJS =\
	types.o \
	decl.o \
	lex.o \
	error.o \
	symbol.o \
	main.o \
	expr.o \
	code.o \
	stmt.o \
	cpp.o \
	fold.o \
	init.o \
	builtin.o \
	arch.o\
	amd64-sysv.o\
	arm64-sysv.o \
	i386-sysv.o \
	z80-scc.o\

TARGET   = $(LIBEXEC)/cc1

all: $(TARGET)

$(TARGET): $(LIBDIR)/libscc.a $(OBJS)
	$(CC) $(SCC_LDFLAGS) $(OBJS) -lscc -o $@

dep: inc-dep

include deps.mk
