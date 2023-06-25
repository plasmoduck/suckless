.POSIX:

NAME = sfeed
VERSION = 1.8

# curses theme, see themes/ directory.
SFEED_THEME = newsboat

# paths
PREFIX = /usr/local
MANPREFIX = ${PREFIX}/man
DOCPREFIX = ${PREFIX}/share/doc/${NAME}

RANLIB = ranlib

# use system flags.
SFEED_CFLAGS = ${CFLAGS}
SFEED_LDFLAGS = ${LDFLAGS}
SFEED_CPPFLAGS = -D_DEFAULT_SOURCE -D_XOPEN_SOURCE=700 -D_BSD_SOURCE

# uncomment for conservative locked I/O.
#SFEED_CPPFLAGS = -D_DEFAULT_SOURCE -D_XOPEN_SOURCE=700 -D_BSD_SOURCE \
#	-DGETNEXT=getchar

# set $SFEED_CURSES to empty to not build sfeed_curses.
SFEED_CURSES = sfeed_curses
SFEED_CURSES_CFLAGS = ${CFLAGS}
SFEED_CURSES_CPPFLAGS = -D_DEFAULT_SOURCE -D_XOPEN_SOURCE=700 -D_BSD_SOURCE \
	 -DSFEED_THEME=\"themes/${SFEED_THEME}.h\"
SFEED_CURSES_LDFLAGS = ${LDFLAGS} -lcurses

# Linux: some distros use ncurses and require -lncurses.
#SFEED_CURSES_LDFLAGS = ${LDFLAGS} -lncurses

# Gentoo Linux: some distros might also require -ltinfo and -D_DEFAULT_SOURCE
# to prevent warnings about feature test macros.
#SFEED_CURSES_LDFLAGS = ${LDFLAGS} -lcurses -ltinfo

# FreeBSD: unset feature test macros for SIGWINCH etc.
#SFEED_CURSES_CPPFLAGS =

# use minicurses with hardcoded escape sequences (not the system curses).
#SFEED_CURSES_CPPFLAGS = -D_DEFAULT_SOURCE -D_XOPEN_SOURCE=700 -D_BSD_SOURCE \
#	-DSFEED_THEME=\"themes/${SFEED_THEME}.h\" -DSFEED_MINICURSES
#SFEED_CURSES_LDFLAGS = ${LDFLAGS}

BIN = \
	sfeed\
	sfeed_atom\
	${SFEED_CURSES}\
	sfeed_frames\
	sfeed_gopher\
	sfeed_html\
	sfeed_json\
	sfeed_mbox\
	sfeed_opml_import\
	sfeed_plain\
	sfeed_twtxt\
	sfeed_web\
	sfeed_xmlenc
SCRIPTS = \
	sfeed_content\
	sfeed_markread\
	sfeed_opml_export\
	sfeed_update

SRC = ${BIN:=.c}
HDR = \
	minicurses.h\
	util.h\
	xml.h

LIBUTIL = libutil.a
LIBUTILSRC = \
	util.c
LIBUTILOBJ = ${LIBUTILSRC:.c=.o}

LIBXML = libxml.a
LIBXMLSRC = \
	xml.c
LIBXMLOBJ = ${LIBXMLSRC:.c=.o}

COMPATSRC = \
	strlcat.c\
	strlcpy.c
COMPATOBJ =\
	strlcat.o\
	strlcpy.o

LIB = ${LIBUTIL} ${LIBXML} ${COMPATOBJ}

MAN1 = ${BIN:=.1}\
	${SCRIPTS:=.1}
MAN5 = \
	sfeed.5\
	sfeedrc.5
DOC = \
	LICENSE\
	README\
	README.xml

all: ${BIN}

${BIN}: ${LIB} ${@:=.o}

OBJ = ${SRC:.c=.o} ${LIBXMLOBJ} ${LIBUTILOBJ} ${COMPATOBJ}

${OBJ}: ${HDR}

.o:
	${CC} -o $@ $< ${LIB} ${SFEED_LDFLAGS}

.c.o:
	${CC} -o $@ -c $< ${SFEED_CFLAGS} ${SFEED_CPPFLAGS}

sfeed_curses.o: sfeed_curses.c themes/${SFEED_THEME}.h
	${CC} -o $@ -c sfeed_curses.c ${SFEED_CURSES_CFLAGS} ${SFEED_CURSES_CPPFLAGS}

sfeed_curses: ${LIB} sfeed_curses.o
	${CC} -o $@ sfeed_curses.o ${LIB} ${SFEED_CURSES_LDFLAGS}

${LIBUTIL}: ${LIBUTILOBJ}
	${AR} -rc $@ $?
	${RANLIB} $@

${LIBXML}: ${LIBXMLOBJ}
	${AR} -rc $@ $?
	${RANLIB} $@

dist:
	rm -rf "${NAME}-${VERSION}"
	mkdir -p "${NAME}-${VERSION}"
	cp -fR ${MAN1} ${MAN5} ${DOC} ${HDR} \
		${SRC} ${LIBXMLSRC} ${LIBUTILSRC} ${COMPATSRC} ${SCRIPTS} \
		themes Makefile \
		sfeedrc.example style.css \
		"${NAME}-${VERSION}"
	# make tarball
	tar cf - "${NAME}-${VERSION}" | \
		gzip -c > "${NAME}-${VERSION}.tar.gz"
	rm -rf "${NAME}-${VERSION}"

clean:
	rm -f ${BIN} ${OBJ} ${LIB}

install: all
	# installing executable files and scripts.
	mkdir -p "${DESTDIR}${PREFIX}/bin"
	cp -f ${BIN} ${SCRIPTS} "${DESTDIR}${PREFIX}/bin"
	for f in ${BIN} ${SCRIPTS}; do chmod 755 "${DESTDIR}${PREFIX}/bin/$$f"; done
	# installing example files.
	mkdir -p "${DESTDIR}${DOCPREFIX}"
	cp -f sfeedrc.example\
		style.css\
		README\
		README.xml\
		"${DESTDIR}${DOCPREFIX}"
	# installing manual pages for general commands: section 1.
	mkdir -p "${DESTDIR}${MANPREFIX}/man1"
	cp -f ${MAN1} "${DESTDIR}${MANPREFIX}/man1"
	for m in ${MAN1}; do chmod 644 "${DESTDIR}${MANPREFIX}/man1/$$m"; done
	# installing manual pages for file formats: section 5.
	mkdir -p "${DESTDIR}${MANPREFIX}/man5"
	cp -f ${MAN5} "${DESTDIR}${MANPREFIX}/man5"
	for m in ${MAN5}; do chmod 644 "${DESTDIR}${MANPREFIX}/man5/$$m"; done

uninstall:
	# removing executable files and scripts.
	for f in ${BIN} ${SCRIPTS}; do rm -f "${DESTDIR}${PREFIX}/bin/$$f"; done
	# removing example files.
	rm -f \
		"${DESTDIR}${DOCPREFIX}/sfeedrc.example"\
		"${DESTDIR}${DOCPREFIX}/style.css"\
		"${DESTDIR}${DOCPREFIX}/README"\
		"${DESTDIR}${DOCPREFIX}/README.xml"
	-rmdir "${DESTDIR}${DOCPREFIX}"
	# removing manual pages.
	for m in ${MAN1}; do rm -f "${DESTDIR}${MANPREFIX}/man1/$$m"; done
	for m in ${MAN5}; do rm -f "${DESTDIR}${MANPREFIX}/man5/$$m"; done

.PHONY: all clean dist install uninstall
