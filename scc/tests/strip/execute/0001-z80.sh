#!/bin/sh

set -e
trap 'rm -f $tmp1 $tmp2 $tmp3' EXIT HUP INT QUIT TERM

tmp1=tmpfile1
tmp2=tmpfile2
tmp3=tmpfile3

cp z80.out $tmp1
strip $tmp1 > $tmp2
z80-unknown-coff-nm $tmp1 >> $tmp2 2>&1 || true

cat > $tmp3 <<EOF
z80-unknown-coff-nm: $tmp1: no symbols
EOF

diff $tmp2 $tmp3
