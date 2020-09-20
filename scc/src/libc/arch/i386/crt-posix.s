	.globl	_environ
	.comm	_environ,4,4

	.globl	_start
	.text
_start:
	movl	%esp,%ebp

	leal	16(%ebp,%edi,8),%edx
	movl	%edx,_environ
	pushl	%edx
	leal	8(%ebp),%esi
	pushl	%esi
	movl	(%ebp),%edi
	pushl	%edi

	call 	main
	movl	%eax,%edi
	jmp	exit
