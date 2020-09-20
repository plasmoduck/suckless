	.file	"longjmp"

	.text
	.globl	longjmp,_longjmp
_longjmp:
longjmp:
	mov	%rsi,%rax
	test	%rax,%rax
	jnz	1f
	inc	%rax
1:
	mov	(%rdi),%rbx
	mov	8(%rdi),%rbp
	mov	16(%rdi),%r12
	mov	24(%rdi),%r13
	mov	32(%rdi),%r14
	mov	40(%rdi),%r15
	mov	48(%rdi),%rdx
	mov	%rdx,%rsp
	mov	56(%rdi),%rdx
	jmp	*%rdx
