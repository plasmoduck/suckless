.POSIX:
.SUFFIXES:
PREFIX ?= /usr/local
MAN     = $(PREFIX)/share/man
NAME    = stw
CC      = cc
PKG_CONFIG = pkg-config
X11INC = /usr/local/include
INCS = -I $(X11INC) \
       `$(PKG_CONFIG) --cflags freetype2` \
       `$(PKG_CONFIG) --cflags freetype2`
X11LIB = /usr/local/lib
LIBS = -L$(X11LIB)  -lX11 -lfontconfig -lXft -lXrender \
       `$(PKG_CONFIG) --libs fontconfig` \
       `$(PKG_CONFIG) --libs freetype2`
CFLAGS  = -g -std=c99 -pedantic -Wall -Werror -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_POSIX_C_SOURCE=2 $(INCS)
LDFLAGS = $(LIBS)
SRC     = $(NAME).c
OBJ     = ${SRC:.c=.o}

all: $(NAME)

$(NAME): $(OBJ)
	$(CC) $(LDFLAGS) -o $@ $(OBJ)

.SUFFIXES: .c .o
$(OBJ):
.c.o:
	$(CC) $(CFLAGS) -c $<

install: all
	mkdir -p $(PREFIX)/bin
	cp -f $(NAME) $(PREFIX)/bin
	chmod 755 ${DESTDIR}${PREFIX}/bin/$(NAME)
	mkdir -p ${DESTDIR}${MAN}/man1
	cp stw.1 ${DESTDIR}${MAN}/man1/stw.1

clean:
	rm -f -- $(NAME) $(OBJ)

.PHONY: clean
