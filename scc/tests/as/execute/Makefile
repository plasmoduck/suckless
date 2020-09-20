.POSIX:
ROOT=../../../

all: tests

tests:
	@PATH=$(ROOT)/libexec/scc/:$(ROOT)/bin:$$PATH ./chktest.sh

clean:
	rm -f test.log
