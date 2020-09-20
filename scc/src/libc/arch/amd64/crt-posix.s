	.globl	_environ
	.comm	_environ,8,8

	.text
	.globl	_start
_start:
	movq	%rsp,%rbp

	/* load argc, argv, envp from stack */
	movq	(%rbp),%rdi             /* argc */
	leaq	8(%rbp),%rsi            /* argv */
	leaq	16(%rbp,%rdi,8),%rdx    /* envp = argv + 8*argc + 8 */
	movq	%rdx,_environ(%rip)

	call	main
	movl	%eax,%edi
	jmp	exit
