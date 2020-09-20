#!/bin/sh

set -e
trap 'rm -f $tmp1 $tmp2' EXIT HUP INT QUIT TERM

tmp1=`mktemp`
tmp2=`mktemp`

size -t z80.out z80.out>$tmp1

cat > $tmp2 <<EOF
text	data	bss	dec	hex	filename
5	3	3	11	b	z80.out
5	3	3	11	b	z80.out
10	6	6	22	16	(TOTALS)
EOF

diff $tmp1 $tmp2
