#!/bin/sh


set -e

trap "rm -f file.a; exit" 0 2 3

############################################################################
#delete two members, 1st and 2nd

cp master.a file.a

ar -dv file.a file1 file2

if ar -tv file.a file1 file2
then
	echo file-1 or file-2 were not deleted >&2
	exit 1
fi
