	.file	"_cerrno.s"
	.globl	_cerrno

_cerrno:
	cmpl	$0,%eax
	js	1f
	ret

1:	neg	%eax
	mov	%eax,(errno)
	mov	$-1,%eax
	ret
