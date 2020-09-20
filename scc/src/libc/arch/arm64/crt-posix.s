	.globl	_environ
	.comm	_environ,8,8

	.text
	.globl	_start
_start:
	ldr	x0,[sp]		/* argc */
	add	x1,sp,#8	/* argv */
	add	x2,x1,x0,lsl #3	/* argv = argc + 8*argc + 8 */
	add	x2,x2,#8
	adr	x3,_environ
	str	x2,[x3]
	bl	main
	b	exit
