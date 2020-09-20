#!/bin/sh

#
# This job is very easy because app and kernel ABI are identical
# until the 4th parameter, so we only have to set the syscall
# number in rax

sed -n "
     s/[ 	]*#.*//
     /$1/p" syscall.lst |
while read num name
do
cat <<EOF > $name.s
	.file	"$name.s"

	.globl	$name
$name:
	movq	\$$num,%rax
	syscall
	jb	1f
	retq

1:	movq	%rax,(errno)
	movq	\$-1,%rax
	retq
EOF
done
