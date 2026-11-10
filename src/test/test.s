	.file	"test.cpp"
	.text
	.section	.text$_ZNKSt5ctypeIcE8do_widenEc,"x"
	.linkonce discard
	.align 2
	.p2align 4
	.globl	_ZNKSt5ctypeIcE8do_widenEc
	.def	_ZNKSt5ctypeIcE8do_widenEc;	.scl	2;	.type	32;	.endef
	.seh_proc	_ZNKSt5ctypeIcE8do_widenEc
_ZNKSt5ctypeIcE8do_widenEc:
.LFB2576:
	.seh_endprologue
	movl	%edx, %eax
	ret
	.seh_endproc
	.text
	.p2align 4
	.def	_ZSt4endlIcSt11char_traitsIcEERSt13basic_ostreamIT_T0_ES6_.isra.0;	.scl	3;	.type	32;	.endef
	.seh_proc	_ZSt4endlIcSt11char_traitsIcEERSt13basic_ostreamIT_T0_ES6_.isra.0
_ZSt4endlIcSt11char_traitsIcEERSt13basic_ostreamIT_T0_ES6_.isra.0:
.LFB10980:
	pushq	%rbx
	.seh_pushreg	%rbx
	subq	$48, %rsp
	.seh_stackalloc	48
	.seh_endprologue
	movq	(%rcx), %rax
	movq	%rcx, %rbx
	movq	-24(%rax), %rax
	movq	240(%rcx,%rax), %rcx
	testq	%rcx, %rcx
	je	.L8
	cmpb	$0, 56(%rcx)
	je	.L5
	movsbl	67(%rcx), %edx
.L6:
	movq	%rbx, %rcx
	call	_ZNSo3putEc
	movq	%rax, %rcx
	addq	$48, %rsp
	popq	%rbx
	jmp	_ZNSo5flushEv
.L5:
	movq	%rcx, 40(%rsp)
	call	_ZNKSt5ctypeIcE13_M_widen_initEv
	movq	40(%rsp), %rcx
	leaq	_ZNKSt5ctypeIcE8do_widenEc(%rip), %r8
	movl	$10, %edx
	movq	(%rcx), %rax
	movq	48(%rax), %rax
	cmpq	%r8, %rax
	je	.L6
	movl	$10, %edx
	call	*%rax
	movsbl	%al, %edx
	jmp	.L6
.L8:
	call	_ZSt16__throw_bad_castv
	nop
	.seh_endproc
	.p2align 4
	.globl	_ZN6httpvo14Implementation15is_out_of_rangeILi33ELi126ELi1EEEyPKh
	.def	_ZN6httpvo14Implementation15is_out_of_rangeILi33ELi126ELi1EEEyPKh;	.scl	2;	.type	32;	.endef
	.seh_proc	_ZN6httpvo14Implementation15is_out_of_rangeILi33ELi126ELi1EEEyPKh
_ZN6httpvo14Implementation15is_out_of_rangeILi33ELi126ELi1EEEyPKh:
.LFB10325:
	.seh_endprologue
	movzbl	(%rcx), %eax
	subl	$33, %eax
	cmpb	$93, %al
	seta	%al
	movzbl	%al, %eax
	ret
	.seh_endproc
	.section .rdata,"dr"
	.align 8
.LC0:
	.ascii "GET https://www.google.com/search?q=how+to+download+only+a+folder+from+a+github+repo&gs_lcrp=EgZjaHJvbWUqCAgBEAAYFhgeMgYIABBFGDkyCAgBEAAYFhgeMggIAhAAGBYYHjINCAMQABiGAxiABBiKBTINCAQQABiGAxiABBiKBTINCAUQABiGAxiABBiKBTIHCAYQABjvBTIHCAcQABjvBTIKCAgQABiiBBiJBdIBCTE1NDMyajBqNKgCALACAQ&sourceid=chrome&ie=UTF-8&udm=50&fbs=ACAhTvwdifjn2dKXmjtOfBxVFl_k7xB_bJXhWBZp-REbIUHBWOp3CYiB4qKOY0dV7v7979NSd1v4STFn0mDYY346TlrUAoPA9pe4zEGs7jquRZoh90W8-EzpeHhpS29m8tPJSD2nUVKCx-XC-9uF4844JQt0SA7Y_jOo-h1clATovKVqHDSuneMk-mgZgiUhTJKr2E6Ok0PdPwf6TKePjGiixb9Nt_Xo3A&aep=10&ntc=1&sxsrf=AKL5yDgovRdfzIJMZO9WLUQjdyCf2x3ZmA%3A1793737240282&mstk=AUtExfBZPTspMrJWGyybvY6DCM33okrssV9BQehVuMh3GhGqU-9wL7D6blc3FrjUJ0XMfjlDlwezk63o_AxZqAc-MHp__EPUUeLLKU-vQAcMOfKZnuEm1t2rer2gMcRflA4MBffCxERyO_RtuzqbudCXHcca7SSmnJCX5Xu3V2-7yBLYkqN3fmBtwK1LvEt6tVlLaZ9vFAohs23PMv7iRUnkIi50BpJtS2hW6bz5lWfp0vIW90mDdz-A6VQsTs3qAW_wR6G0sgsomyX72MAs_Q_vO-Ph4nxXSEJ1c41_sP_ArF6O6S45fT2w6tKFyn7bbXTSFwe1MQyyjcUVKTcKgIO6r7Epi1eokd3PZN6L761VJb2ZfdwOqdhEjiGHrv0kEgJnHj2BzFpIe536W6SQLo0xS_I5Z9h4WGAY-Q&aioh=3&csuir=1&cs=1&mtid=GELqavywMPKwhbIP8LqW2AQ HTTP/1.1\15\12\0"
	.section	.text.startup,"x"
	.p2align 4
	.globl	main
	.def	main;	.scl	2;	.type	32;	.endef
	.seh_proc	main
main:
.LFB10351:
	pushq	%r12
	.seh_pushreg	%r12
	pushq	%rbp
	.seh_pushreg	%rbp
	pushq	%rdi
	.seh_pushreg	%rdi
	pushq	%rsi
	.seh_pushreg	%rsi
	pushq	%rbx
	.seh_pushreg	%rbx
	subq	$1248, %rsp
	.seh_stackalloc	1248
	vmovups	%xmm6, 1232(%rsp)
	.seh_savexmm	%xmm6, 1232
	.seh_endprologue
	vpxor	%xmm6, %xmm6, %xmm6
	leaq	_ZN6httpvo14ImplementationL9TCHAR_LUTE(%rip), %rsi
	call	__main
	movq	.refptr._ZSt4cout(%rip), %rbx
	movl	$1, %edx
	movq	$0, 112(%rsp)
	vmovdqu	%xmm6, 96(%rsp)
	movq	%rbx, %rcx
	call	_ZNSolsEi
	movq	%rax, %rcx
	call	_ZSt4endlIcSt11char_traitsIcEERSt13basic_ostreamIT_T0_ES6_.isra.0
	vmovdqu	.LC8(%rip), %xmm0
	movb	$0, 144(%rsp)
	leaq	128(%rsp), %rdx
	xorl	%eax, %eax
	vmovdqu	%xmm0, 128(%rsp)
.L11:
	movzbl	(%rdx), %ecx
	cmpb	$3, (%rsi,%rcx)
	jne	.L71
	incq	%rax
	incq	%rdx
	cmpq	$16, %rax
	jne	.L11
	movl	$4294967295, %edx
	xorl	%eax, %eax
	xorl	%r8d, %r8d
	xorl	%r11d, %r11d
	salq	$32, %rdx
	xorl	%edi, %edi
	movq	%r11, 32(%rsp)
	andq	%rdx, %rax
	movl	%edi, 40(%rsp)
	movl	$4294967295, %edx
	movl	%eax, %eax
	movq	%rax, %r9
	movq	%r8, %rax
	orq	%rdx, %rax
	movq	%rax, %r8
	.p2align 4,,10
	.p2align 3
.L18:
	movl	%r8d, %eax
	movq	%r9, %rdi
	testl	%r8d, %r8d
	jne	.L27
	movq	128(%rsp,%r9), %rax
	movabsq	$3328493621662205952, %rdx
	movq	%rax, %r12
	salq	$8, %rax
	shrq	$56, %r12
	xorl	$48, %r12d
	cmpq	%rdx, %rax
	sete	%al
	cmpb	$9, %r12b
	setbe	%dl
	andl	%edx, %eax
	leaq	-5(%r9), %rdx
	cmpq	$3, %rdx
	jbe	.L28
	cmpw	$2573, 136(%rsp,%r9)
	setne	%dl
.L29:
	xorb	$1, %al
	jne	.L57
	testb	%dl, %dl
	jne	.L57
	movq	96(%rsp), %rbp
	movq	%rbx, %rcx
	movq	%rbp, %rdx
	movq	%rbp, %rdi
	call	_ZNSo9_M_insertIyEERSoT_
	notq	%rdi
	movq	%rax, %rcx
	call	_ZSt4endlIcSt11char_traitsIcEERSt13basic_ostreamIT_T0_ES6_.isra.0
	movq	%rbp, %r8
	leaq	128(%rsp), %rdx
	movq	%rbx, %rcx
	call	_ZSt16__ostream_insertIcSt11char_traitsIcEERSt13basic_ostreamIT_T0_ES6_PKS3_x
	movq	%rax, %rcx
	call	_ZSt4endlIcSt11char_traitsIcEERSt13basic_ostreamIT_T0_ES6_.isra.0
	addq	104(%rsp), %rdi
	movq	%rbx, %rcx
	movq	%rdi, %rdx
	call	_ZNSo9_M_insertIyEERSoT_
	movq	%rax, %rcx
	call	_ZSt4endlIcSt11char_traitsIcEERSt13basic_ostreamIT_T0_ES6_.isra.0
	movq	%rdi, %r8
	leaq	129(%rsp,%rbp), %rdx
	movq	%rbx, %rcx
	call	_ZSt16__ostream_insertIcSt11char_traitsIcEERSt13basic_ostreamIT_T0_ES6_PKS3_x
	movq	%rax, %rcx
	call	_ZSt4endlIcSt11char_traitsIcEERSt13basic_ostreamIT_T0_ES6_.isra.0
	movzbl	%r12b, %edx
	movq	%rbx, %rcx
	call	_ZNSolsEi
	movq	%rax, %rcx
	call	_ZSt4endlIcSt11char_traitsIcEERSt13basic_ostreamIT_T0_ES6_.isra.0
	leaq	.LC0(%rip), %rdx
	movl	$1101, %r8d
	leaq	128(%rsp), %rcx
	call	memcpy
	leaq	128(%rsp), %rdx
	xorl	%eax, %eax
	vmovdqu	%xmm6, 96(%rsp)
.L30:
	movzbl	(%rdx), %ecx
	cmpb	$3, (%rsi,%rcx)
	jne	.L72
	incq	%rax
	incq	%rdx
	cmpq	$1100, %rax
	jne	.L30
	movl	$4294967295, %edx
	xorl	%eax, %eax
	xorl	%r8d, %r8d
	xorl	%ecx, %ecx
	salq	$32, %rdx
	movl	%r8d, 40(%rsp)
	xorl	%r8d, %r8d
	andq	%rdx, %rax
	movq	%rcx, 32(%rsp)
	movl	$4294967295, %edx
	movl	%eax, %eax
	movq	%rax, %r9
	movq	%r8, %rax
	orq	%rdx, %rax
	movq	%rax, %r8
	.p2align 4,,10
	.p2align 3
.L37:
	movl	%r8d, %eax
	movq	%r9, %rsi
	testl	%r8d, %r8d
	jne	.L54
	movq	128(%rsp,%r9), %rax
	movabsq	$3328493621662205952, %rdx
	movq	%rax, %rbp
	salq	$8, %rax
	shrq	$56, %rbp
	xorl	$48, %ebp
	cmpq	%rdx, %rax
	sete	%al
	cmpb	$9, %bpl
	setbe	%dl
	andl	%edx, %eax
	leaq	-1089(%r9), %rdx
	cmpq	$3, %rdx
	jbe	.L55
	cmpw	$2573, 136(%rsp,%r9)
	setne	%dl
.L56:
	xorb	$1, %al
	jne	.L58
	testb	%dl, %dl
	jne	.L58
	movq	96(%rsp), %rdi
	movq	%rbx, %rcx
	movq	%rdi, %rdx
	movq	%rdi, %rsi
	call	_ZNSo9_M_insertIyEERSoT_
	notq	%rsi
	movq	%rax, %rcx
	call	_ZSt4endlIcSt11char_traitsIcEERSt13basic_ostreamIT_T0_ES6_.isra.0
	movq	%rdi, %r8
	leaq	128(%rsp), %rdx
	movq	%rbx, %rcx
	call	_ZSt16__ostream_insertIcSt11char_traitsIcEERSt13basic_ostreamIT_T0_ES6_PKS3_x
	movq	%rax, %rcx
	call	_ZSt4endlIcSt11char_traitsIcEERSt13basic_ostreamIT_T0_ES6_.isra.0
	addq	104(%rsp), %rsi
	movq	%rbx, %rcx
	movq	%rsi, %rdx
	call	_ZNSo9_M_insertIyEERSoT_
	movq	%rax, %rcx
	call	_ZSt4endlIcSt11char_traitsIcEERSt13basic_ostreamIT_T0_ES6_.isra.0
	movq	%rsi, %r8
	leaq	129(%rsp,%rdi), %rdx
	movq	%rbx, %rcx
	call	_ZSt16__ostream_insertIcSt11char_traitsIcEERSt13basic_ostreamIT_T0_ES6_PKS3_x
	movq	%rax, %rcx
	call	_ZSt4endlIcSt11char_traitsIcEERSt13basic_ostreamIT_T0_ES6_.isra.0
	movzbl	%bpl, %edx
	movq	%rbx, %rcx
	call	_ZNSolsEi
	movq	%rax, %rcx
	call	_ZSt4endlIcSt11char_traitsIcEERSt13basic_ostreamIT_T0_ES6_.isra.0
	nop
	vmovups	1232(%rsp), %xmm6
	xorl	%eax, %eax
	addq	$1248, %rsp
	popq	%rbx
	popq	%rsi
	popq	%rdi
	popq	%rbp
	popq	%r12
	ret
.L71:
	cmpb	$32, 128(%rsp,%rax)
	jne	.L73
	leaq	-5(%rax), %rdx
	xorl	%r8d, %r8d
	movq	%rax, 96(%rsp)
	movq	%rax, %r9
	cmpq	$11, %rdx
	setbe	%r8b
	jbe	.L18
	leaq	129(%rsp,%rax), %rcx
	cmpb	$32, (%rcx)
	je	.L68
	leaq	136(%rsp), %r8
	cmpq	%r8, %rcx
	jnb	.L20
	movabsq	$9187201950435737471, %rdi
	movabsq	$-6872316419617283936, %r11
	movabsq	$72340172838076673, %r10
	movabsq	$-9187201950435737472, %r9
	jmp	.L23
	.p2align 6
	.p2align 4,,10
	.p2align 3
.L21:
	addq	$8, %rcx
	cmpq	%r8, %rcx
	jnb	.L20
.L23:
	movq	(%rcx), %rax
	movq	%r11, %rbp
	movq	%rax, %rdx
	andq	%rdi, %rdx
	subq	%rdx, %rbp
	addq	%r10, %rdx
	orq	%rbp, %rdx
	andn	%rdx, %rax, %rax
	andq	%r9, %rax
	tzcntq	%rax, %rax
	sarl	$3, %eax
	cltq
	cmpq	$7, %rax
	ja	.L21
	leaq	(%rcx,%rax), %r8
.L22:
	leaq	144(%rsp), %rdx
	cmpq	%rdx, %r8
	je	.L74
	cmpb	$32, (%r8)
	jne	.L75
	leaq	1(%r8), %rcx
	cmpq	%rdx, %rcx
	jnb	.L26
	cmpb	$32, 1(%r8)
	je	.L68
.L26:
	subq	%rcx, %rdx
	movq	%rcx, %rax
	leaq	128(%rsp), %rdi
	subq	%rdi, %rax
	cmpq	$9, %rdx
	setbe	%dl
	movq	%rax, 104(%rsp)
	movq	%rax, %r9
	movzbl	%dl, %edx
	movq	%rdx, %r8
	jmp	.L18
.L72:
	cmpb	$32, 128(%rsp,%rax)
	jne	.L76
	leaq	-1089(%rax), %rdx
	xorl	%r8d, %r8d
	movq	%rax, 96(%rsp)
	movq	%rax, %r9
	cmpq	$11, %rdx
	setbe	%r8b
	jbe	.L37
	leaq	129(%rsp,%rax), %rdx
	subq	$1068, %rax
	movzbl	(%rdx), %ecx
	cmpq	$31, %rax
	jbe	.L38
	cmpb	$32, %cl
	je	.L69
	leaq	1196(%rsp), %rcx
	cmpq	%rcx, %rdx
	jnb	.L40
	vmovdqu	.LC6(%rip), %ymm2
	vmovdqu	.LC7(%rip), %ymm1
	jmp	.L43
	.p2align 6
	.p2align 4,,10
	.p2align 3
.L41:
	addq	$32, %rdx
	cmpq	%rcx, %rdx
	jnb	.L40
.L43:
	vpaddb	(%rdx), %ymm2, %ymm0
	vpminsb	%ymm1, %ymm0, %ymm0
	vpcmpeqb	%ymm0, %ymm1, %ymm0
	vpmovmskb	%ymm0, %eax
	tzcntq	%rax, %rax
	cltq
	cmpq	$31, %rax
	ja	.L41
	leaq	(%rdx,%rax), %rcx
.L42:
	leaq	1228(%rsp), %rdx
	cmpq	%rdx, %rcx
	je	.L77
	cmpb	$32, (%rcx)
	jne	.L78
	leaq	1(%rcx), %r8
	cmpq	%rdx, %r8
	jnb	.L46
	cmpb	$32, 1(%rcx)
	je	.L79
.L46:
	movq	%r8, %rax
	leaq	128(%rsp), %rdi
	subq	%r8, %rdx
	subq	%rdi, %rax
	cmpq	$9, %rdx
	movq	%rax, 104(%rsp)
	setbe	%dl
	movq	%rax, %r9
	movzbl	%dl, %edx
	movq	%rdx, %r8
	vzeroupper
	jmp	.L37
.L20:
	movq	136(%rsp), %rax
	movabsq	$9187201950435737471, %rdx
	movabsq	$-6872316419617283936, %rcx
	movabsq	$72340172838076673, %r9
	andq	%rax, %rdx
	subq	%rdx, %rcx
	addq	%r9, %rdx
	orq	%rcx, %rdx
	andn	%rdx, %rax, %rax
	movabsq	$-9187201950435737472, %rdx
	andq	%rdx, %rax
	tzcntq	%rax, %rax
	sarl	$3, %eax
	cltq
	addq	%rax, %r8
	jmp	.L22
.L40:
	vmovdqu	.LC6(%rip), %ymm0
	vpaddb	1196(%rsp), %ymm0, %ymm0
	vpcmpb	$5, .LC7(%rip), %ymm0, %k0
	kmovd	%k0, %eax
	tzcntq	%rax, %rax
	cltq
	addq	%rax, %rcx
	jmp	.L42
.L57:
	movl	$1, %eax
.L27:
	movl	%eax, %edx
	movq	%rbx, %rcx
	call	_ZNSolsEi
	movl	$32, %edx
	movq	%rax, %rcx
	call	_ZStlsISt11char_traitsIcEERSt13basic_ostreamIcT_ES5_c
	movq	%rdi, %rdx
.L70:
	movq	%rax, %rcx
	call	_ZNSo9_M_insertIyEERSoT_
	movq	%rax, %rcx
	call	_ZSt4endlIcSt11char_traitsIcEERSt13basic_ostreamIT_T0_ES6_.isra.0
	movl	$-1, %ecx
	call	exit
.L28:
	cmpb	$13, 136(%rsp,%r9)
	setne	%dl
	cmpb	$10, 137(%rsp,%r9)
	setne	%cl
	orl	%ecx, %edx
	jmp	.L29
.L58:
	movl	$1, %eax
.L54:
	movl	%eax, %edx
	movq	%rbx, %rcx
	call	_ZNSolsEi
	movl	$32, %edx
	movq	%rax, %rcx
	call	_ZStlsISt11char_traitsIcEERSt13basic_ostreamIcT_ES5_c
	movq	%rsi, %rdx
	jmp	.L70
.L55:
	cmpb	$13, 136(%rsp,%r9)
	setne	%dl
	cmpb	$10, 137(%rsp,%r9)
	setne	%cl
	orl	%ecx, %edx
	jmp	.L56
.L68:
	movl	$4, %r8d
	movl	$16, %r9d
	jmp	.L18
.L73:
	movl	$4294967295, %edx
	xorl	%eax, %eax
	xorl	%r9d, %r9d
	xorl	%r8d, %r8d
	salq	$32, %rdx
	movq	%r9, 32(%rsp)
	xorl	%r10d, %r10d
	andq	%rdx, %rax
	movl	%r10d, 40(%rsp)
	movl	%eax, %eax
	movq	%rax, %r9
	movq	%r8, %rax
	andq	%rdx, %rax
	orq	$3, %rax
	movq	%rax, %r8
	jmp	.L18
.L76:
	xorl	%edx, %edx
	xorl	%eax, %eax
	xorl	%r8d, %r8d
	movl	%edx, 40(%rsp)
	movl	$4294967295, %edx
	salq	$32, %rdx
	movq	%rax, 32(%rsp)
	xorl	%eax, %eax
	andq	%rdx, %rax
	movl	%eax, %eax
	movq	%rax, %r9
	movq	%r8, %rax
	andq	%rdx, %rax
	orq	$3, %rax
	movq	%rax, %r8
	jmp	.L37
.L69:
	movl	$4, %r8d
	movl	$1100, %r9d
	jmp	.L37
.L77:
	movl	$4294967295, %r8d
	movl	$1100, %r9d
	vzeroupper
	jmp	.L37
.L74:
	movl	$4294967295, %r8d
	movl	$16, %r9d
	jmp	.L18
.L78:
	leaq	128(%rsp), %rax
	movl	$3, %r8d
	subq	%rax, %rcx
	movq	%rcx, %r9
	vzeroupper
	jmp	.L37
.L75:
	leaq	128(%rsp), %rax
	subq	%rax, %r8
	movq	%r8, %r9
	movl	$3, %r8d
	jmp	.L18
.L38:
	leaq	1220(%rsp), %rax
	cmpb	$32, %cl
	je	.L69
	movabsq	$9187201950435737471, %rsi
	movabsq	$72340172838076673, %r10
	movabsq	$-9187201950435737472, %r9
	jmp	.L47
	.p2align 6
.L50:
	movq	(%rdx), %rcx
	movabsq	$-6872316419617283936, %rdi
	movq	%rcx, %r8
	andq	%rsi, %r8
	subq	%r8, %rdi
	addq	%r10, %r8
	orq	%rdi, %r8
	andn	%r8, %rcx, %rcx
	andq	%r9, %rcx
	tzcntq	%rcx, %rcx
	sarl	$3, %ecx
	movslq	%ecx, %rcx
	cmpq	$7, %rcx
	jbe	.L80
	addq	$8, %rdx
.L47:
	cmpq	%rax, %rdx
	jb	.L50
	movq	1220(%rsp), %rdx
	movabsq	$9187201950435737471, %rcx
	movabsq	$-6872316419617283936, %r8
	movabsq	$72340172838076673, %r9
	andq	%rdx, %rcx
	subq	%rcx, %r8
	addq	%r9, %rcx
	orq	%r8, %rcx
	andn	%rcx, %rdx, %rdx
	movabsq	$-9187201950435737472, %rcx
	andq	%rcx, %rdx
	tzcntq	%rdx, %rdx
	sarl	$3, %edx
	movslq	%edx, %rdx
	addq	%rdx, %rax
.L49:
	leaq	1228(%rsp), %rdx
	movl	$4294967295, %r8d
	movl	$1100, %r9d
	cmpq	%rdx, %rax
	je	.L37
	cmpb	$32, (%rax)
	jne	.L81
	leaq	1(%rax), %rcx
	cmpq	%rdx, %rcx
	jnb	.L53
	cmpb	$32, 1(%rax)
	je	.L69
.L53:
	subq	%rcx, %rdx
	movq	%rcx, %rax
	leaq	128(%rsp), %rsi
	subq	%rsi, %rax
	cmpq	$9, %rdx
	setbe	%dl
	movq	%rax, 104(%rsp)
	movq	%rax, %r9
	movzbl	%dl, %edx
	movq	%rdx, %r8
	jmp	.L37
.L79:
	movl	$4, %r8d
	movl	$1100, %r9d
	vzeroupper
	jmp	.L37
.L80:
	leaq	(%rdx,%rcx), %rax
	jmp	.L49
.L81:
	leaq	128(%rsp), %rsi
	movl	$3, %r8d
	subq	%rsi, %rax
	movq	%rax, %r9
	jmp	.L37
	.seh_endproc
	.section .rdata,"dr"
	.align 32
_ZN6httpvo14ImplementationL9TCHAR_LUTE:
	.ascii "\0\0\0\0\0\0\0\0\2\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\2\2\2\2\2\2\2\2\3\2\3\3\3\3\3\2\2\3\3\2\3\3\2\3\3\3\3\3\3\3\3\3\3\2\2\2\2\2\2\2\3\3\3\3\3\3\3\3\3\3\3\3\3\3\3\3\3\3\3\3\3\3\3\3\3\3\2\2\2\3\3\3\3\3\3\3\3\3\3\3\3\3\3\3\3\3\3\3\3\3\3\3\3\3\3\3\3\3\2\3\2\0\2\2\2\2\2\2\2\2\2\2\2\2\2\2\2\2\2\2\2\2\2\2\2\2\2\2\2\2\2\2\2\2\2\2\2\2\2\2\2\2\2\2\2\2\2\2\2\2\2\2\2\2\2\2\2\2\2\2\2\2\2\2\2\2\2\2\2\2\2\2\2\2\2\2\2\2\2\2\2\2\2\2\2\2\2\2\2\2\2\2\2\2\2\2\2\2\2\2\2\2\2\2\2\2\2\2\2\2\2\2\2\2\2\2\2\2\2\2\2\2\2\2\2\2\2\2\2\2\2"
	.globl	_ZN6httpvo4passE
	.bss
_ZN6httpvo4passE:
	.space 1
	.section .rdata,"dr"
	.align 32
.LC6:
	.long	1600085855
	.long	1600085855
	.long	1600085855
	.long	1600085855
	.long	1600085855
	.long	1600085855
	.long	1600085855
	.long	1600085855
	.align 32
.LC7:
	.long	-555819298
	.long	-555819298
	.long	-555819298
	.long	-555819298
	.long	-555819298
	.long	-555819298
	.long	-555819298
	.long	-555819298
	.align 16
.LC8:
	.quad	6073139479992091975
	.quad	724289189539958868
	.def	__main;	.scl	2;	.type	32;	.endef
	.ident	"GCC: (Rev4, Built by MSYS2 project) 16.2.0"
	.def	_ZNSo3putEc;	.scl	2;	.type	32;	.endef
	.def	_ZNSo5flushEv;	.scl	2;	.type	32;	.endef
	.def	_ZNKSt5ctypeIcE13_M_widen_initEv;	.scl	2;	.type	32;	.endef
	.def	_ZSt16__throw_bad_castv;	.scl	2;	.type	32;	.endef
	.def	_ZNSolsEi;	.scl	2;	.type	32;	.endef
	.def	_ZNSo9_M_insertIyEERSoT_;	.scl	2;	.type	32;	.endef
	.def	_ZSt16__ostream_insertIcSt11char_traitsIcEERSt13basic_ostreamIT_T0_ES6_PKS3_x;	.scl	2;	.type	32;	.endef
	.def	memcpy;	.scl	2;	.type	32;	.endef
	.def	_ZStlsISt11char_traitsIcEERSt13basic_ostreamIcT_ES5_c;	.scl	2;	.type	32;	.endef
	.def	exit;	.scl	2;	.type	32;	.endef
	.section	.rdata$.refptr._ZSt4cout, "dr"
	.p2align	3, 0
	.globl	.refptr._ZSt4cout
	.linkonce	discard
.refptr._ZSt4cout:
	.quad	_ZSt4cout
