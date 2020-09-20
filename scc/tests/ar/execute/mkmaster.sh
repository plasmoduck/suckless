#!/bin/sh

uid=`id -u`
gid=`id -g`
time=315532800

cat <<EOF >master.a
!<arch>
file1           `printf "%-12d" $time``printf "%-6d" $uid``printf "%-6d" $gid`100644  79        \`
This is the first file,
and it should go in the
first position in the archive.

file2           `printf "%-12d" $time``printf "%-6d" $uid``printf "%-6d" $gid`100644  125       \`
But this other one is the second one,
and it shouldn't go in the first position
because it should go in the second position.

file3           `printf "%-12d" $time``printf "%-6d" $uid``printf "%-6d" $gid`100644  118       \`
and at the end, this is the last file
that should go at the end of the file,
thus it should go in the third position.
EOF
