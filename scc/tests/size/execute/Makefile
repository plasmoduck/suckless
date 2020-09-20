.POSIX:
ROOT=../../..

OUT = z80.out

all: tests

tests: $(OUT)
	@PATH=$(ROOT)/bin:$$PATH:. chktest.sh

z80.out: master.s
	z80-unknown-coff-as -o $@ master.s

clean:
	rm -f *.a *.out test.log
