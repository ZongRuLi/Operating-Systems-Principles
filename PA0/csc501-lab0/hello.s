	.file	"hello.c"
	.section	.rodata
.LC0:
	.string	"zfunction(0xaabbccdd)=%lx\n"
	.text
	.globl	main
	.type	main, @function
main:
.LFB0:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$16, %rsp
	movl	$2864434397, %eax
	movq	%rax, -8(%rbp)
	movq	-8(%rbp), %rax
	movq	%rax, %rdi
	call	zfunction
	movq	%rax, %rsi
	movl	$.LC0, %edi
	movl	$0, %eax
	call	printf
	movl	$0, %eax
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE0:
	.size	main, .-main
	.globl	zfunction
	.type	zfunction, @function
zfunction:
.LFB1:
	pushl	%ebp
	movl	%esp, %ebp
	movl	%edi, -8(%ebp)
	movl	-8(%ebp), %eax
	sall	$4, %eax
	andl	$4227989488, %eax
	popl	%ebp
	ret
.LFE1:
	.size	zfunction, .-zfunction
	.ident	"GCC: (GNU) 4.8.5 20150623 (Red Hat 4.8.5-44)"
	.section	.note.GNU-stack,"",@progbits
