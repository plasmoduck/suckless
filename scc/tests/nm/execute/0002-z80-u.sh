#!/bin/sh

set -e

tmp1=`mktemp`
tmp2=`mktemp`
trap "rm -f $tmp1 $tmp2; exit" 0 2 3

nm -u z80.out > $tmp1

cat <<! > $tmp2
                 U text6
!

diff $tmp1 $tmp2
