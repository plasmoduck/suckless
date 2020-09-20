#!/bin/sh

set -e

tmp1=`mktemp`
tmp2=`mktemp`

trap "rm -f file* $tmp1 $tmp2; exit" 0 2 3

############################################################################
#Update one member that already exist

echo First > file1

cp master.a file.a
sleep 1
touch file1
ar -ruv file.a file1

ar -p file.a file1 > $tmp1

cat <<EOF > $tmp2
First
EOF

cmp $tmp1 $tmp2

echo Second > file1
touch -t 197001010000 file.1
ar -ruv file.a file1

ar -p file.a file1 > $tmp1

cat <<EOF > $tmp2
First
EOF

cmp $tmp1 $tmp2
