#!/bin/sh

set -e

tmp1=`mktemp`
tmp2=`mktemp`

trap "rm -f file* $tmp1 $tmp2; exit" 0 2 3

############################################################################
#move 1st without specifier

cp master.a file.a
ar -mv file.a file1
ar -t file.a > $tmp1

cat <<EOF > $tmp2
file2
file3
file1
EOF

cmp $tmp1 $tmp2
