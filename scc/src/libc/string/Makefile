.POSIX:
PROJECTDIR =../../..
include $(PROJECTDIR)/scripts/rules.mk
include ../rules.mk

OBJS = \
	strcat.$O\
	strchr.$O\
	strcmp.$O\
	strcoll.$O\
	strcpy.$O\
	strcspn.$O\
	strerror.$O\
	strlen.$O\
	strncat.$O\
	strncmp.$O\
	strncpy.$O\
	strpbrk.$O\
	strrchr.$O\
	strspn.$O\
	strstr.$O\
	strtok.$O\
	strxfrm.$O\

all: $(LIBC)

$(LIBC): $(OBJS)
	$(MKLST)

dep: inc-dep

include deps.mk
