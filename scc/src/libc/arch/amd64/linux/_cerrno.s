	.file	"_cerrno.s"
	.globl	_cerrno

_cerrno:
	cmpq	$0,%rax
	js	1f
	retq

1:	neg	%rax
	mov	%eax,(errno)
	mov	$-1,%eax
	retq
