
	.text
	.globl	_Exit
	.globl	_setcontext

_setcontext:
	movq	%r15,%rdi
	movq	$0x134,%rax
	syscall

	# Something was wrong, finish the program. We can't call
	# abort here because it could generate a loop
	movq	$-1,%rdi
	jmp	_Exit
