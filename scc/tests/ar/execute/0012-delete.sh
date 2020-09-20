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


############################################################################
#delete two members, 1st and 2nd

cp master.a file.a

ar -dv file.a file1 file2

if ar -tv file.a file1 file2
then
	echo file-1 or file-2 were not deleted >&2
	exit 1
fi


############################################################################
#delete two members, 2nd and 3rd

cp master.a file.a
ar -dv  file.a file2 file3

if ar -tv file.a file2 file3
then
	echo file-2 file-3 were not deleted >&2
	exit 1
fi

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

############################################################################
#special cases

#no members
cp master.a file.a

last=`ls -l file.a | awk '{print $6,$7,$8}'`

if ! ar -dv file.a
then
	echo ar returned with error when no members
	exit 1
fi

now=`ls -l file.a | awk '{print $6,$7,$8}'`
if test "$now" != "$last"
then
	echo empty ar -d modified the archive >&2
	exit 1
fi

#delete not existing member
cp master.a file.a

if ar -dv file.a badfile
then
	echo ar returned ok deleting a not existing member >&2
	exit 1
fi
