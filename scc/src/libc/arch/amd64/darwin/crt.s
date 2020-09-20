	.bss
	.globl	__environ
__environ:
	.quad	0

	.text
	.global	start
start:
	movq	%rsp,%rbp

	/* load argc, argv, envp from stack */
	movq	(%rbp),%rdi             /* argc */
	leaq	8(%rbp),%rsi            /* argv */
	leaq	16(%rbp,%rdi,8),%rdx    /* envp = argv + 8*argc + 8 */
	movq	%rdx,__environ(%rip)

	call	_main
	movl	%eax,%edi
	jmp	_exit
