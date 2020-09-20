	.file 	"memcpy.s"
	.text
	.globl	memcpy,_memcpy

memcpy:
_memcpy:
	mov	%rdi,%rax
	mov	%rdx,%rcx
	cld
	rep
	movsb
	ret
