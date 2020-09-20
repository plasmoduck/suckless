.POSIX:
PROJECTDIR =../../..
include $(PROJECTDIR)/scripts/rules.mk
include ../rules.mk

OBJS =\
	ctype.$O\
	isalnum.$O\
	isalpha.$O\
	isascii.$O\
	isblank.$O\
	iscntrl.$O\
	isdigit.$O\
	isgraph.$O\
	islower.$O\
	isprint.$O\
	ispunct.$O\
	isspace.$O\
	isupper.$O\
	isxdigit.$O\
	toascii.$O\
	tolower.$O\
	toupper.$O\

all: $(LIBC)

$(LIBC): $(OBJS)
	$(MKLST)

dep: inc-dep

include deps.mk
