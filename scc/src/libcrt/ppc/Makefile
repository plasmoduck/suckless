.POSIX:
PROJECTDIR=../../..
include $(PROJECTDIR)/scripts/rules.mk

OBJS = dummy.o
TARGET = $(LIBCDIR)/libcrt.a

all: $(TARGET)

$(TARGET): $(OBJS)
	mklib -o $@ $?
