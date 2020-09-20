.POSIX:
PROJECTDIR=../../..
include $(PROJECTDIR)/scripts/rules.mk

OBJS =	dummy.o		\
	umoddi3.o	\
	udivdi3.o	\

TARGET = $(LIBCDIR)/libcrt.a

all: $(TARGET)

$(TARGET): $(OBJS)
	mklib -o $@ $?
