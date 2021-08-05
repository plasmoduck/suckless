# sacc: saccomys gopher client
# See LICENSE file for copyright and license details.
.POSIX:

include config.mk

BIN = sacc
MAN = $(BIN).1
OBJ = $(BIN:=.o) ui_$(UI).o

all: $(BIN)

config.h:
	cp config.def.h config.h

$(BIN): $(OBJ)
	$(CC) $(OBJ) $(LDFLAGS) $(LIBS) -o $@

$(OBJ): config.h config.mk common.h

clean:
	rm -f $(BIN) $(OBJ)

install: $(BIN)
	mkdir -p $(DESTDIR)$(PREFIX)/bin/
	cp -f $(BIN) $(DESTDIR)$(PREFIX)/bin/
	chmod 555 $(DESTDIR)$(PREFIX)/bin/$(BIN)
	mkdir -p $(DESTDIR)$(MANDIR)
	cp -f $(MAN) $(DESTDIR)$(MANDIR)

uninstall:
	rm -f $(DESTDIR)$(PREFIX)/bin/$(BIN) $(DESTDIR)$(MANDIR)/$(MAN)

# Stock FLAGS
SACCCFLAGS = -D_BSD_SOURCE \
             $(CFLAGS)

.c.o:
	$(CC) $(SACCCFLAGS) -c $<
