.POSIX:

PROJECTDIR = .
include $(PROJECTDIR)/scripts/rules.mk

PREFIX= /usr/local
DIRS  = src src/libc include/scc/scc tests

all:
	+@$(MAKE) `$(SCRIPTDIR)/config` toolchain
	+@$(MAKE) `$(SCRIPTDIR)/config` CONF=amd64-linux libc
	+@$(MAKE) `$(SCRIPTDIR)/config` CONF=amd64-openbsd libc
	+@$(MAKE) `$(SCRIPTDIR)/config` CONF=amd64-netbsd libc
	+@$(MAKE) `$(SCRIPTDIR)/config` CONF=amd64-dragonfly libc

toolchain: dirs src

libc: dirs src/libc

src: include/scc/scc

dirs: $(SCRIPTDIR)/libc-proto
	xargs mkdir -p < $(SCRIPTDIR)/libc-proto
	touch dirs

$(DIRS): $(ENVIRON) FORCE
	+@. $(ENVIRON) && cd $@ && $(MAKE)

$(ENVIRON):
	@rm -f $@; \
	trap 'r=$?;rm -f $$$$.tmp;exit $r' EXIT HUP INT QUIT TERM; \
	echo PATH=$$PATH:$$PWD/$(SCRIPTDIR):. > $$$$.tmp && \
	echo PREFIX=\"$(PREFIX)\" >> $$$$.tmp && \
	echo NM=\"$(NM)\" >> $$$$.tmp && \
	echo AR=\"$(AR)\" >> $$$$.tmp && \
	echo RL=\"$(RL)\" >> $$$$.tmp && \
	echo STD=\"$(STD)\" >> $$$$.tmp && \
	echo ARFLAGS=\"$(ARFLAGS)\" >> $$$$.tmp && \
	echo RLFLAGS=\"$(RLFLAGS)\" >> $$$$.tmp && \
	echo export PATH STD ARFLAGS RLFLAGS NM AR RL >> $$$$.tmp && \
	mv $$$$.tmp $@

dep:
	$(FORALL)

install: all
	$(SCRIPTDIR)/install $(PREFIX)

distclean: clean
	$(MAKE) $(ENVIRON)
	$(FORALL)
	rm -f $(ENVIRON)

clean: $(ENVIRON)
	$(FORALL)
	xargs rm -rf < $(SCRIPTDIR)/libc-proto
	rm -f dirs $(ENVIRON)

tests: all
