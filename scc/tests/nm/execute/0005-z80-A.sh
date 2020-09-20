#!/bin/sh

set -e

tmp1=`mktemp`
tmp2=`mktemp`
trap "rm -f f.out f.a $tmp1 $tmp2; exit" 0 2 3

rm -f f.a
ar -qv f.a z80.out
cp z80.out f.out
ar -qv f.a f.out
nm -A f.a z80.out > $tmp1

cat <<! > $tmp2
f.a[z80.out]: 0000000000000000 b .bss
f.a[z80.out]: 0000000000000000 d .data
f.a[z80.out]: 0000000000000000 t .text
f.a[z80.out]: 0000000000000001 B averylongbss
f.a[z80.out]: 0000000000000001 D averylongdata
f.a[z80.out]: 0000000000000001 T averylongtext
f.a[z80.out]: 0000000000000000 B bss1
f.a[z80.out]: 0000000000000002 b bss3
f.a[z80.out]: 000000000000000a C bss4
f.a[z80.out]: 0000000000000012 C bss5
f.a[z80.out]: 0000000000000000 D data1
f.a[z80.out]: 0000000000000002 d data3
f.a[z80.out]: 000000000000000a C data4
f.a[z80.out]: 0000000000000012 C data5
f.a[z80.out]: 0000000000000000 T text1
f.a[z80.out]: 0000000000000002 t text3
f.a[z80.out]: 000000000000000a C text4
f.a[z80.out]: 0000000000000012 C text5
f.a[z80.out]:                  U text6
f.a[f.out]: 0000000000000000 b .bss
f.a[f.out]: 0000000000000000 d .data
f.a[f.out]: 0000000000000000 t .text
f.a[f.out]: 0000000000000001 B averylongbss
f.a[f.out]: 0000000000000001 D averylongdata
f.a[f.out]: 0000000000000001 T averylongtext
f.a[f.out]: 0000000000000000 B bss1
f.a[f.out]: 0000000000000002 b bss3
f.a[f.out]: 000000000000000a C bss4
f.a[f.out]: 0000000000000012 C bss5
f.a[f.out]: 0000000000000000 D data1
f.a[f.out]: 0000000000000002 d data3
f.a[f.out]: 000000000000000a C data4
f.a[f.out]: 0000000000000012 C data5
f.a[f.out]: 0000000000000000 T text1
f.a[f.out]: 0000000000000002 t text3
f.a[f.out]: 000000000000000a C text4
f.a[f.out]: 0000000000000012 C text5
f.a[f.out]:                  U text6
z80.out: 0000000000000000 b .bss
z80.out: 0000000000000000 d .data
z80.out: 0000000000000000 t .text
z80.out: 0000000000000001 B averylongbss
z80.out: 0000000000000001 D averylongdata
z80.out: 0000000000000001 T averylongtext
z80.out: 0000000000000000 B bss1
z80.out: 0000000000000002 b bss3
z80.out: 000000000000000a C bss4
z80.out: 0000000000000012 C bss5
z80.out: 0000000000000000 D data1
z80.out: 0000000000000002 d data3
z80.out: 000000000000000a C data4
z80.out: 0000000000000012 C data5
z80.out: 0000000000000000 T text1
z80.out: 0000000000000002 t text3
z80.out: 000000000000000a C text4
z80.out: 0000000000000012 C text5
z80.out:                  U text6
!

diff $tmp1 $tmp2
