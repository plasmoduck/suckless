#!/bin/sh

set -e

tmp1=`mktemp`
tmp2=`mktemp`

trap "rm -f file* f1 f2 f3 $tmp1 $tmp2; exit" 0 2 3

###########################################################################
#Append generated files at once to an existing archive

echo First > file-1
echo Second > file-2
echo Third > file-3

cp master.a file.a
ar -qv file.a file-1 file-2 file-3

ar -t file.a file-1 file-2 file-3 > $tmp1

cat <<EOF > $tmp2
file-1
file-2
file-3
EOF

cmp $tmp1 $tmp2

ar -p file.a file-1 file-2 file-3 > $tmp1

cat <<EOF > $tmp2
First
Second
Third
EOF

cmp $tmp1 $tmp2
