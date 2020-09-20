#!/bin/sh

set -e

tmp1=`mktemp`
tmp2=`mktemp`

trap "rm -f file.a $tmp1 $tmp2; exit" 0 2 3

############################################################################
#print 3rd member

cp master.a file.a
ar -p file.a file3 > $tmp1

cat <<! > $tmp2
and at the end, this is the last file
that should go at the end of the file,
thus it should go in the third position.
!

cmp $tmp1 $tmp2
