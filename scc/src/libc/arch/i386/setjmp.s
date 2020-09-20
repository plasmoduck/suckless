	.file	"setjmp.s"

	.text
	.globl	setjmp,_setjmp
_setjmp:
setjmp:
	movl    4(%esp),%eax
	movl    %ebx,(%eax)
	movl    %ecx,4(%eax)
	movl    %esi,8(%eax)
	movl    %edi,12(%eax)
	movl    %ebp,16(%eax)
	movl    %esp,20(%eax)
	pushl   (%esp)
	popl	24(%eax)
	xor     %eax,%eax
	ret
