#!/bin/sh


set -e

trap "rm -f file.a; exit" 0 2 3

############################################################################
#delete two members, 2nd and 3rd

cp master.a file.a
ar -dv  file.a file2 file3

if ar -tv file.a file2 file3
then
	echo file-2 file-3 were not deleted >&2
	exit 1
fi
