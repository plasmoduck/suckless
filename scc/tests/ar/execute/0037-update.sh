#!/bin/sh

set -e

tmp1=`mktemp`
tmp2=`mktemp`

trap "rm -f file* $tmp1 $tmp2; exit" 0 2 3

############################################################################
#Update one member that doesn't exist and using -a

echo First > file-1

cp master.a file.a
ar -rv -a file1 file.a file-1

ar -p file.a file-1 > $tmp1

cat <<EOF > $tmp2
First
EOF

cmp $tmp1 $tmp2

ar -t file.a > $tmp1

cat <<EOF > $tmp2
file1
file-1
file2
file3
EOF

cmp $tmp1 $tmp2
