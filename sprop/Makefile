# sprop - simple X property utility

include config.mk

SRC = sprop.c
OBJ = ${SRC:.c=.o}

all: options sprop

options:
	@echo sprop build options:
	@echo "CFLAGS   = ${CFLAGS}"
	@echo "LDFLAGS  = ${LDFLAGS}"
	@echo "CC       = ${CC}"

.c.o:
	@echo CC $<
	@${CC} -c ${CFLAGS} $<

${OBJ}: config.mk

sprop: ${OBJ}
	@echo CC -o $@
	@${CC} -o $@ ${OBJ} ${LDFLAGS}

clean:
	@echo cleaning
	@rm -f sprop ${OBJ} sprop-${VERSION}.tar.gz

dist: clean
	@echo creating dist tarball
	@mkdir -p sprop-${VERSION}
	@cp -R LICENSE Makefile README config.mk sprop.1 ${SRC} sprop-${VERSION}
	@tar -cf sprop-${VERSION}.tar sprop-${VERSION}
	@gzip sprop-${VERSION}.tar
	@rm -rf sprop-${VERSION}

install: all
	@echo installing executable file to ${DESTDIR}${PREFIX}/bin
	@mkdir -p ${DESTDIR}${PREFIX}/bin
	@cp -f sprop ${DESTDIR}${PREFIX}/bin
	@chmod 755 ${DESTDIR}${PREFIX}/bin/sprop
	@echo installing manual page to ${DESTDIR}${MANPREFIX}/man1
	@mkdir -p ${DESTDIR}${MANPREFIX}/man1
	@sed "s/VERSION/${VERSION}/g" < sprop.1 > ${DESTDIR}${MANPREFIX}/man1/sprop.1
	@chmod 644 ${DESTDIR}${MANPREFIX}/man1/sprop.1

uninstall:
	@echo removing executable file from ${DESTDIR}${PREFIX}/bin
	@rm -f ${DESTDIR}${PREFIX}/bin/sprop
	@echo removing manual page from ${DESTDIR}${MANPREFIX}/man1
	@rm -f ${DESTDIR}${MANPREFIX}/man1/sprop.1

.PHONY: all options clean dist install uninstall
