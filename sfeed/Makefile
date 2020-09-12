.POSIX:

NAME = sfeed
VERSION = 0.9.18

# paths
PREFIX = /usr/local
MANPREFIX = ${PREFIX}/man
DOCPREFIX = ${PREFIX}/share/doc/${NAME}

RANLIB = ranlib

# use system flags.
SFEED_CFLAGS = ${CFLAGS}
SFEED_LDFLAGS = ${LDFLAGS}
SFEED_CPPFLAGS = -D_POSIX_C_SOURCE=200809L -D_XOPEN_SOURCE=700 -D_BSD_SOURCE

BIN = \
	sfeed\
	sfeed_atom\
	sfeed_frames\
	sfeed_gopher \
	sfeed_html\
	sfeed_mbox\
	sfeed_opml_import\
	sfeed_plain\
	sfeed_twtxt\
	sfeed_web\
	sfeed_xmlenc
SCRIPTS = \
	sfeed_opml_export\
	sfeed_update

SRC = ${BIN:=.c}
HDR = \
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
	${CC} ${SFEED_LDFLAGS} -o $@ $< ${LIB}

.c.o:
	${CC} ${SFEED_CFLAGS} ${SFEED_CPPFLAGS} -o $@ -c $<

${LIBUTIL}: ${LIBUTILOBJ}
	${AR} -rc $@ $?
	${RANLIB} $@

${LIBXML}: ${LIBXMLOBJ}
	${AR} rc $@ $?
	${RANLIB} $@

dist:
	rm -rf "${NAME}-${VERSION}"
	mkdir -p "${NAME}-${VERSION}"
	cp -f ${MAN1} ${MAN5} ${DOC} ${HDR} \
		${SRC} ${LIBXMLSRC} ${LIBUTILSRC} ${COMPATSRC} ${SCRIPTS} \
		Makefile \
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
