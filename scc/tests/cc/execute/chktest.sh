#!/bin/sh

file=${1?' empty input file'}
trap "rm -f a.out; exit" 0 1 2 3 15
ulimit -c 0
rm -f test.log

SYS=`uname | tr A-Z a-z`
FORMAT=elf
ABI=sysv
ARCH=amd64

export SYS FORMAT ABI ARCH

while read i state
do
	echo $i >>test.log
	state=${state:-""}
	rm -f a.out

	(SCCPREFIX=$SCCPREFIX $CC -Isysinclude $CFLAGS "$i" && ./a.out) 2>>test.log &&
		printf '[PASS]' || printf '[FAIL]'
	printf '\t%s\t%s\n' $i $state
done < $file
