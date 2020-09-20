TOOLCFLAGS = -std=c99

COMP = gcc
ASM = as
LINKER = ld
RANLIB = ranlib
ARCHIVE = ar

ARCHIVEFLAGS = -U
NOPIE_CFLAGS = -nopie
NOPIE_LDFLAGS = -nopie
TOOLCFLAGS = -std=c99 -fno-stack-protector -static
