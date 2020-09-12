.POSIX:

NAME = xscreenshot
VERSION = 1.0

# paths
PREFIX = /usr/local
MANPREFIX = ${PREFIX}/man

INCS = -I/usr/local/include
LIBS = -L/usr/local/lib -lX11

# use system flags.
XSCREENSHOT_CFLAGS = ${CFLAGS} ${INCS}
XSCREENSHOT_LDFLAGS = ${LDFLAGS} ${LIBS}
XSCREENSHOT_CPPFLAGS = -D_DEFAULT_SOURCE -D_BSD_SOURCE

BIN = ${NAME}
SRC = ${BIN:=.c}
MAN1 = ${BIN:=.1}
OBJ = ${SRC:.c=.o}
DOC = \
	LICENSE\
	README

all: ${BIN}

${OBJ}:

.o:
	${CC} ${XSCREENSHOT_LDFLAGS} -o $@ $<

.c.o:
	${CC} ${XSCREENSHOT_CFLAGS} ${XSCREENSHOT_CPPFLAGS} -o $@ -c $<

dist:
	rm -rf "${NAME}-${VERSION}"
	mkdir -p "${NAME}-${VERSION}"
	cp -f ${MAN1} ${DOC} ${SRC} Makefile \
		"${NAME}-${VERSION}"
	# make tarball
	tar -cf - "${NAME}-${VERSION}" | \
		gzip -c > "${NAME}-${VERSION}.tar.gz"
	rm -rf "${NAME}-${VERSION}"

clean:
	rm -f ${BIN} ${OBJ}

install: all
	mkdir -p ${DESTDIR}${PREFIX}/bin
	cp -f ${BIN} ${DESTDIR}${PREFIX}/bin
	chmod 755 ${DESTDIR}${PREFIX}/bin/xscreenshot
	mkdir -p ${DESTDIR}${MANPREFIX}/man1
	cp -f ${MAN1} ${DESTDIR}${MANPREFIX}/man1
	chmod 644 ${DESTDIR}${MANPREFIX}/man1/xscreenshot.1

uninstall:
	rm -f ${DESTDIR}${PREFIX}/bin/xscreenshot
	rm -f ${DESTDIR}${MANPREFIX}/man1/xscreenshot.1

.PHONY: all clean dist install uninstall
