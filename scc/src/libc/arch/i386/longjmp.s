	.file	"longjmp.s"

	.text
	.globl	longjmp,_longjmp
_longjmp:
longjmp:
	mov  	4(%esp),%edx
	mov  	8(%esp),%eax
	test    %eax,%eax
	jnz 	1f
	inc     %eax
1:
	movl   	(%edx),%ebx
	movl  	4(%edx),%ecx
	movl 	8(%edx),%esi
	movl 	12(%edx),%edi
	movl	16(%edx),%ebp
	movl	20(%edx),%esp
	pushl   24(%edx)
	popl	%edx
	jmp 	*%edx
