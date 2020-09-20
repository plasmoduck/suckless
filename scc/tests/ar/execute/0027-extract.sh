#!/bin/sh

set -e

tmp1=`mktemp`
tmp2=`mktemp`

trap "rm -f file* $tmp1 $tmp2; exit" 0 2 3

############################################################################
#extract without parameters

cp master.a file.a
ar -xv file.a

cat <<EOF > $tmp1
This is the first file,
and it should go in the
first position in the archive.
But this other one is the second one,
and it shouldn't go in the first position
because it should go in the second position.
and at the end, this is the last file
that should go at the end of the file,
thus it should go in the third position.
EOF

cat file1 file2 file3 > $tmp2

cmp $tmp1 $tmp2

if test `ls file? | wc -l` -ne 3
then
	echo some error extracting files
	exit
fi
