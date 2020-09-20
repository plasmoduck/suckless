	.comm _environ,4,4

	.text
	.globl	_start
_start:
	stwu	1,-16(1)
	lis	14,_environ@h
	ori	14,14,_environ@l
	stw	5,0(14)
	bl	main
	b	exit
