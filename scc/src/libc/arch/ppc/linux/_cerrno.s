	.file	"_cerrno.s"
	.globl	_cerrno
_cerrno:
	cmpwi	0,0
	bne	err
	blr
err:
	lis	14,errno@h
	ori	14,14,errno@l
	stw	3,0(14)
	xor	3,3,3
	addi	3,3,-1
	blr
