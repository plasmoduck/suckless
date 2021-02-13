CC = cc
LD = $(CC)
 
PREFIX = /usr/local
MANPREFIX = ${PREFIX}/share/man
 
CFLAGS = -Wall -Wextra -pedantic
LDFLAGS =
  
.PHONY : clean install uninstall
 
skroll: skroll.o
 
clean : rm -f skroll *.o
 
install: skroll skroll.1 install -D -m 0755 skroll ${DESTDIR}${PREFIX}/bin/skroll install -D -m 0644 skroll.1 ${DESTDIR}${MANPREFIX}/man1/skroll.1
 
uninstall:
	${RM} ${DESTDIR}${PREFIX}/bin/skroll
	${RM} ${DESTDIR}${MANPREFIX}/man1/skroll.1
