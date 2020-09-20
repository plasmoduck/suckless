AS = z80-unknown-coff-as
AR = ../../../bin/ar
LD = ../../../bin/ld
RL = ../../../bin/ranlib
SZ = ../../../bin/size

OBJS   = f1.o f2.o
OBJLIB = f3.o

all test: a.out

a.out: $(OBJS) f.a
	$(LD) $(OBJS) f.a

f.a: $(OBJLIB)
	$(AR) -rv $@ $?

size:
	$(SZ) *.o *.a

clean:
	rm -f *.o *.a a.out core* *.dat
