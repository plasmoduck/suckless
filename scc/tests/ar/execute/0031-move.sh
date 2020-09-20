#!/bin/sh

set -e

tmp1=`mktemp`
tmp2=`mktemp`

trap "rm -f file* $tmp1 $tmp2; exit" 0 2 3

############################################################################
#and now, test without parameters

cp master.a file.a
ar -mv file.a
ar -t file.a > $tmp1

cat <<EOF > $tmp2
file1
file2
file3
EOF

cmp $tmp1 $tmp2
