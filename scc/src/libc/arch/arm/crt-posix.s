	.globl	_environ
	.comm	_environ,4,4

	.text
	.globl	_start
_start:
	ldr	r0,[sp]		/* argc */
	add	r1,sp,#4	/* argv */
	add	r2,r1,r0,lsl #2	/* argv = argc + 4*argc + 4 */
	add	r2,r2,#4
	ldr	r3,=_environ
	str	r2,[r3]
	bl	main
	b	exit
