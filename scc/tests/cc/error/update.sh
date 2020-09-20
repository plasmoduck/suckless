#!/bin/sh

for i
do
	(echo '/^PATTERN/+;/^\./-c'
	 cc $CFLAGS -w -c $i 2>&1
	 printf ".\nw\n"
	 echo w) |
	ed -s $i
done
