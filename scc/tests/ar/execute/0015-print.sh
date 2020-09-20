#!/bin/sh

set -e

tmp1=`mktemp`
tmp2=`mktemp`

trap "rm -f file.a $tmp1 $tmp2; exit" 0 2 3

############################################################################
#print 2nd member with verbose

cp master.a file.a
ar -pv file.a file2 >$tmp1

cat <<! > $tmp2

<file2>

But this other one is the second one,
and it shouldn't go in the first position
because it should go in the second position.
!

cmp $tmp1 $tmp2
