include config.mk

NAME = lel
SRC = lel.c
OBJ = ${SRC:.c=.o}
# paths
PREFIX = /usr/local
MANPREFIX = ${PREFIX}/man

INCS = -I/usr/local/include
LIBS = -L/usr/local/lib -lX11


all: lel

options:
	@echo ${NAME} build options:
	@echo "CFLAGS   = ${CFLAGS}"
	@echo "LDFLAGS  = ${LDFLAGS}"
	@echo "CC       = ${CC}"

.c.o:
	${CC} -c ${CFLAGS} $<

${OBJ}: config.mk

lel: lel.o
	${CC} -o $@ lel.o ${LDFLAGS}

clean:
	rm -f lel ${OBJ}

install: all
	@echo installing executable file to ${DESTDIR}${PREFIX}/bin
	@mkdir -p ${DESTDIR}${PREFIX}/bin
	@cp -f lel ${DESTDIR}${PREFIX}/bin
	@cp -f lel-open ${DESTDIR}${PREFIX}/bin
	@chmod 755 ${DESTDIR}${PREFIX}/bin/lel
	@chmod 755 ${DESTDIR}${PREFIX}/bin/lel-open
	@echo installing manual pages to ${DESTDIR}${MANPREFIX}/man1
	@mkdir -p ${DESTDIR}${MANPREFIX}/man1
	@cp -f lel.1 ${DESTDIR}${MANPREFIX}/man1
	@chmod 644 ${DESTDIR}${MANPREFIX}/man1/lel.1

uninstall:
	@echo removing executable file from ${DESTDIR}${PREFIX}/bin
	@rm -f ${DESTDIR}${PREFIX}/bin/lel
	@rm -f ${DESTDIR}${PREFIX}/bin/lel-open
	@echo removing manual pages from ${DESTDIR}${MANPREFIX}/man1
	@rm -f ${DESTDIR}${MANPREFIX}/man1/lel.1

.PHONY: all options clean dist install uninstall
