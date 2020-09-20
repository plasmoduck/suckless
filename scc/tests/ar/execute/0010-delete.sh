#!/bin/sh


set -e

trap "rm -f file.a; exit" 0 2 3

############################################################################
#remove all the members

cp master.a file.a
ar -dv file.a file1 file2 file3

if ar -tv file.a file2 file3
then
	echo file-1 file2 file were not deleted >&2
	exit 1
fi

if test `ar -t file.a | wc -l` -ne 0
then
	echo file.a is not empty after deleting all the members >&2
	exit 1
fi
