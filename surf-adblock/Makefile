include config.mk

SRC = surf-adblock.c adblock.c
OBJ = ${SRC:.c=.lo}

all: surf-adblock.la

.SUFFIXES: .la .lo .o .c

.c.o:
	${CC} -c ${CFLAGS} $<

.c.lo:
	${LIBTOOL} --mode compile --tag CC ${CC} ${CFLAGS} -c $<

${OBJ}: config.mk

surf-adblock.la: ${OBJ}
	${LIBTOOL} --mode link --tag CC ${CC} ${LIB} ${LDFLAGS} -o $@ \
		${OBJ} -rpath ${DESTDIR}${LIBPREFIX}

clean:
	rm -rf surf-adblock.la .libs ${OBJ} ${OBJ:.lo=.o}

install: all surf-adblock.la
	mkdir -p ${DESTDIR}${PREFIX}/bin
	cp -f surf-adblock-dl ${DESTDIR}${PREFIX}/bin
	chmod 755 ${DESTDIR}${PREFIX}/bin/surf-adblock-dl

	mkdir -p ${DESTDIR}${LIBPREFIX}
	${LIBTOOL} --mode install install -c surf-adblock.la \
		${DESTDIR}${LIBPREFIX}/surf-adblock.la

uninstall:
	rm -f ${DESTDIR}${PREFIX}/bin/surf-adblock-dl
	${LIBTOOL} --mode uninstall rm -f \
		${DESTDIR}${LIBPREFIX}/surf-adblock.la
	rm -df ${DESTDIR}${LIBPREFIX}
	- rm -df ${DESTDIR}${PREFIX}/bin

.PHONY: all clean install uninstall
