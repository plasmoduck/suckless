#!/bin/sh

set -e

tmp1=`mktemp`
tmp2=`mktemp`
trap "rm -f $tmp1 $tmp2; exit" 0 2 3

nm -g z80.out > $tmp1

cat <<! > $tmp2
0000000000000001 B averylongbss
0000000000000001 D averylongdata
0000000000000001 T averylongtext
0000000000000000 B bss1
000000000000000a C bss4
0000000000000012 C bss5
0000000000000000 D data1
000000000000000a C data4
0000000000000012 C data5
0000000000000000 T text1
000000000000000a C text4
0000000000000012 C text5
                 U text6
!

diff $tmp1 $tmp2
