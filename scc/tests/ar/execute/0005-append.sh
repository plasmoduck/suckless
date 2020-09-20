#!/bin/sh

set -e

tmp1=`mktemp`
tmp2=`mktemp`

trap "rm -f file.a f1 f2 f3 $tmp1 $tmp2; exit" 0 2 3

###########################################################################
#empty file list

rm -f file.a
if ar -qv file.a badfile.a
then
	echo "ar -q failed to detect missed file" >&2
	exit 1
fi
