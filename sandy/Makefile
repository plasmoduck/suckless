# sandy - simple ncurses editor
# See LICENSE file for copyright and license details.

include config.mk

.POSIX:
.SUFFIXES: .c .o

HDR = arg.h

SRC = sandy.c

OBJ = $(SRC:.c=.o)

all: options sandy

options:
	@echo sandy build options:
	@echo "CFLAGS   = $(CFLAGS)"
	@echo "LDFLAGS  = $(LDFLAGS)"
	@echo "CC       = $(CC)"

$(OBJ): config.h config.mk

.o:
	@echo LD $@
	@$(LD) -o $@ $< $(LDFLAGS)

.c.o:
	@echo CC $<
	@$(CC) -c -o $@ $< $(CFLAGS)

config.h:
	@echo creating $@ from config.def.h
	@cp config.def.h $@

sandy: $(OBJ)
	@echo CC -o $@
	@$(CC) -o $@ $(OBJ) $(LDFLAGS)

clean:
	@echo cleaning
	@rm -f sandy $(OBJ) sandy-$(VERSION).tar.gz

dist: clean
	@echo creating dist tarball
	@mkdir -p sandy-$(VERSION)
	@cp -R LICENSE Makefile config.mk config.def.h \
		README TODO sandy.1 $(HDR) $(SRC) sandy-$(VERSION)
	@tar -cf sandy-$(VERSION).tar sandy-$(VERSION)
	@gzip sandy-$(VERSION).tar
	@rm -rf sandy-$(VERSION)

install: all
	@echo installing executable file to $(DESTDIR)$(PREFIX)/bin
	@mkdir -p $(DESTDIR)$(PREFIX)/bin
	@cp -f sandy $(DESTDIR)$(PREFIX)/bin
	@chmod 755 $(DESTDIR)$(PREFIX)/bin/sandy
	@echo installing manual page to $(DESTDIR)$(MANPREFIX)/man1
	@mkdir -p $(DESTDIR)$(MANPREFIX)/man1
	@sed "s/VERSION/$(VERSION)/g" < sandy.1 > $(DESTDIR)$(MANPREFIX)/man1/sandy.1
	@chmod 644 $(DESTDIR)$(MANPREFIX)/man1/sandy.1

uninstall:
	@echo removing executable file from $(DESTDIR)$(PREFIX)/bin
	@rm -f $(DESTDIR)$(PREFIX)/bin/sandy
	@echo removing manual page from $(DESTDIR)$(MANPREFIX)/man1
	@rm -f $(DESTDIR)$(MANPREFIX)/man1/sandy.1

.PHONY: all options clean dist install uninstall
