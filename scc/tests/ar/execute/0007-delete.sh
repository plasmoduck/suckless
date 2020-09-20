#!/bin/sh


set -e

trap "rm -f file.a; exit" 0 2 3

############################################################################
#delete one member

cp master.a file.a

ar -dv file.a file2

if ar -tv file.a file2
then
	echo file-2 was not deleted >&2
	exit 1
fi
