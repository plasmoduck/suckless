#!/bin/sh

set -e

tmp1=`mktemp`
tmp2=`mktemp`

trap "rm -f file* $tmp1 $tmp2; exit" 0 2 3

############################################################################
#extract 3rd member

cp master.a file.a
ar -xv file.a file3

cat <<EOF > $tmp1
and at the end, this is the last file
that should go at the end of the file,
thus it should go in the third position.
EOF

cmp file3 $tmp1
