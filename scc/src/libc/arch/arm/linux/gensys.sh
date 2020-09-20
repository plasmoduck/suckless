#!/bin/sh

sed -n "
     s/[ 	]*#.*//
     /$1/p" syscall.lst |
while read num name
do
cat <<EOF > $name.s
	.file	"$name.s"

	.globl	$name
	.arm
$name:
	push	{r7,lr}
	ldr	r7,=$num
	swi	0
	pop	{r7,lr}
	b	_cerrno
EOF
done
