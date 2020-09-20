#!/bin/sh

#
# This job is very easy because app and kernel ABI are identical
# until the 4th parameter, so we only have to set the syscall
# number in rax

awk 'NF == 2 && $2 == "'$1'" {
	printf("0x%x\t%s\n", 33554432 + $1, $2)
}' syscall.lst |
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

1:	movq	%rax,_errno(%rip)
	movq	\$-1,%rax
	retq
EOF
done
