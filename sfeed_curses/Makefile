.POSIX:

NAME = sfeed_curses
VERSION = 0.9.6

# paths
PREFIX = /usr/local
MANPREFIX = ${PREFIX}/man
DOCPREFIX = ${PREFIX}/share/doc/${NAME}

# use system flags.
SFEED_CFLAGS = ${CFLAGS}
SFEED_LDFLAGS = ${LDFLAGS} -lcurses
SFEED_CPPFLAGS = -D_BSD_SOURCE

# Linux: some distros use ncurses and require -lncurses.
#SFEED_LDFLAGS = ${LDFLAGS} -lncurses

# Gentoo Linux: some distros might also require -ltinfo and -D_DEFAULT_SOURCE
# to prevent warnings about feature macros.
#SFEED_LDFLAGS = ${LDFLAGS} -lcurses -ltinfo
#SFEED_CPPFLAGS = -D_DEFAULT_SOURCE -D_POSIX_C_SOURCE=200809L -D_XOPEN_SOURCE=700 -D_BSD_SOURCE

BIN = sfeed_curses
SCRIPTS = sfeed_content sfeed_markread

SRC = ${BIN:=.c}
HDR = minicurses.h

MAN1 = ${BIN:=.1}\
	${SCRIPTS:=.1}
DOC = \
	LICENSE\
	README

all: ${BIN}

${BIN}: ${@:=.o}

OBJ = ${SRC:.c=.o}

${OBJ}:

.o:
	${CC} -o $@ $< ${SFEED_LDFLAGS}

.c.o:
	${CC} ${SFEED_CFLAGS} ${SFEED_CPPFLAGS} -o $@ -c $<

dist:
	rm -rf "${NAME}-${VERSION}"
	mkdir -p "${NAME}-${VERSION}"
	cp -f ${MAN1} ${DOC} ${HDR} ${SRC} ${SCRIPTS} Makefile \
		"${NAME}-${VERSION}"
	# make tarball
	tar cf - "${NAME}-${VERSION}" | \
		gzip -c > "${NAME}-${VERSION}.tar.gz"
	rm -rf "${NAME}-${VERSION}"

clean:
	rm -f ${BIN} ${OBJ}

install: all
	# installing executable files and scripts.
	mkdir -p "${DESTDIR}${PREFIX}/bin"
	cp -f ${BIN} ${SCRIPTS} "${DESTDIR}${PREFIX}/bin"
	for f in ${BIN} ${SCRIPTS}; do chmod 755 "${DESTDIR}${PREFIX}/bin/$$f"; done
	# installing example files.
	mkdir -p "${DESTDIR}${DOCPREFIX}"
	cp -f README\
		"${DESTDIR}${DOCPREFIX}"
	# installing manual pages for general commands: section 1.
	mkdir -p "${DESTDIR}${MANPREFIX}/man1"
	cp -f ${MAN1} "${DESTDIR}${MANPREFIX}/man1"
	for m in ${MAN1}; do chmod 644 "${DESTDIR}${MANPREFIX}/man1/$$m"; done

uninstall:
	# removing executable files and scripts.
	for f in ${BIN} ${SCRIPTS}; do rm -f "${DESTDIR}${PREFIX}/bin/$$f"; done
	# removing example files.
	rm -f \
		"${DESTDIR}${DOCPREFIX}/README"
	-rmdir "${DESTDIR}${DOCPREFIX}"
	# removing manual pages.
	for m in ${MAN1}; do rm -f "${DESTDIR}${MANPREFIX}/man1/$$m"; done

.PHONY: all clean dist install uninstall
