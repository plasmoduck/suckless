	.file	"_cerrno.s"
	.globl	_cerrno

_cerrno:
	cmp	r0,#0
	blt	1f
	bx	lr

1:
	neg	r0,r0
	ldr	r1,=errno
	str	r0,[r1]
	mov	r0,#-1
	bx	lr
