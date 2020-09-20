#!/bin/sh

set -e
trap 'rm -f $tmp1 $tmp2' EXIT HUP INT QUIT TERM

tmp1=`mktemp`
tmp2=`mktemp`

size z80.out >$tmp1

cat > $tmp2 <<EOF
text	data	bss	dec	hex	filename
5	3	3	11	b	z80.out
EOF

diff $tmp1 $tmp2
