	.file   "setjmp.s"

	.text
	.globl	setjmp
setjmp:
	// IHI0042F_aapcs.pdf 5.1.1 callee saved registers
	stmia	r0, {r4-r11, sp, lr}
	mov	r0, #0
	bx	lr
