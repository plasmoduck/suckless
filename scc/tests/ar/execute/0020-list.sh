#!/bin/sh

set -e

tmp1=`mktemp`
tmp2=`mktemp`

trap "rm -f file.a $tmp1 $tmp2; exit" 0 2 3

############################################################################
#print 2nd member with verbose

cp master.a file.a
ar -tv file.a file2 >$tmp1

cat <<! > $tmp2
rw-r--r-- `id -u`/`id -g`	Tue Jan  1 00:00:00 1980 file2
!

cmp $tmp1 $tmp2
