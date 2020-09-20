#!/bin/sh

set -e

tmp1=`mktemp`
tmp2=`mktemp`

trap "rm -f file.a $tmp1 $tmp2; exit" 0 2 3

############################################################################
#print all members

cp master.a file.a
ar -p file.a file1 file2 file3 >$tmp1

cat <<! > $tmp2
This is the first file,
and it should go in the
first position in the archive.
But this other one is the second one,
and it shouldn't go in the first position
because it should go in the second position.
and at the end, this is the last file
that should go at the end of the file,
thus it should go in the third position.
!

cmp $tmp1 $tmp2
