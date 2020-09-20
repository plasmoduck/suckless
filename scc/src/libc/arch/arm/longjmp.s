	.file   "longjmp.s"

	.text
	.globl	longjmp
longjmp:
	ldmia	r0, {r4-r11, sp, lr}

	// If r1 is 0 return 1 instead
	movs	r0, r1
	moveq	r0, #1
	bx	lr
