	.file	"udivdi3.s"

	.text
	.globl	__udivdi3
__udivdi3:
	pushl	%ebp
	movl	%esp,%ebp
	addl	$-16,%esp

	movl	8(%ebp),%ecx
	movl	%ecx,-16(%ebp)
	movl	16(%ebp),%ecx
	movl	%ecx,-8(%ebp)
	movl	-16(%ebp),%eax
	xor	%edx,%edx
	divl	-8(%ebp)
	movl	%ebp,%esp
	popl	%ebp
	ret
