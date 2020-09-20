#!/bin/sh

set -e

tmp1=`mktemp`
tmp2=`mktemp`

trap "rm -f file.a $tmp1 $tmp2; exit" 0 2 3

############################################################################
#print all members

cp master.a file.a
ar -t file.a file1 file2 file3 >$tmp1

cat <<! > $tmp2
file1
file2
file3
!

cmp $tmp1 $tmp2
