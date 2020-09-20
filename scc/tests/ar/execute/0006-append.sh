#!/bin/sh

set -e

tmp1=`mktemp`
tmp2=`mktemp`

trap "rm -f file.a f1 f2 f3 $tmp1 $tmp2; exit" 0 2 3

###########################################################################
#empty file list

rm -f file.a
ar -qv file.a file.a
