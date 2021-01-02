VERSION = 0.1

# Customize below to fit your system

# paths
PREFIX = /usr/local
MANPREFIX = ${PREFIX}/share/man
LIBPREFIX = ${PREFIX}/lib/surf

X11INC = /usr/local/include
X11LIB = /usr/local/lib

WEBEXTINC = `pkg-config --cflags gtk+-3.0 webkit2gtk-4.0 \
             webkit2gtk-web-extension-4.0`
WEBEXTLIB = `pkg-config --libs gtk+-3.0 webkit2gtk-4.0 \
             webkit2gtk-web-extension-4.0`

# includes and libs
INCS = -I. -I/usr/include -I${X11INC} ${WEBEXTINC}
LIBS = -L/usr/lib -lc -L${X11LIB} -lX11 ${WEBEXTLIB} -lgthread-2.0

# flags
CPPFLAGS = -DVERSION=\"${VERSION}\" -DWEBEXTDIR=\"${LIBPREFIX}\" \
           -D_DEFAULT_SOURCE -DWEBEXTDIR=\"${LIBPREFIX}\"
CFLAGS = -std=c99 -pedantic -Wall -O2 ${INCS} ${CPPFLAGS} ${WEBEXTINC}
LDFLAGS = -s ${LIBS} -module -avoid-version

# compiler and linker
CC = cc
LIBTOOL = libtool --quiet
