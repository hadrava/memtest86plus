/*
 *	exit.S		Copyright (C) 2012 Michael Brown <mcb30@ipxe.org>
 *
 */

#include "defs.h"

.text
.code32
.globl exit
exit:
	/* Exit status to %eax */
	movl	4(%esp), %eax

	/* Calculate real-mode exit %ss:esp in %dx:%edi */
	xorw	%dx, %dx
	xorl	%edi, %edi
	movl	exit_esp, %ebp
	cmpl	$EXIT_MAGIC, 0(%ebp)
	jne	1f
	movl	8(%ebp), %edi
	movw	6(%ebp), %dx
1:
	/* Load IDT and GDT and switch to 16-bit code segment */
	cli
	lidt	idt_descr
	lgdt	gdt_descr
	ljmp	$REAL_CS, $(1f - LOW_TEST_ADR)
1:
	.code16
	/* Load 16-bit limits for other segment registers */
	movw	$REAL_DS, %bx
	movw	%bx, %ds
	movw	%bx, %es
	movw	%bx, %fs
	movw	%bx, %gs
	movw	%bx, %ss
	movl	$LOW_TEST_ADR, %esp

	/* Switch to real mode */
	movl	%cr0, %ebx
	andb	$~0x01, %bl
	movl	%ebx, %cr0
	ljmp	$(LOW_TEST_ADR >> 4), $1f
1:
	/* Load real-mode segment registers */
	xorw	%bx, %bx
	movw	%bx, %ds
	movw	%bx, %es
	movw	%bx, %fs
	movw	%bx, %gs
	movw	%bx, %ss

	/* Reenable interrupts */
	sti

	/* If we have a real-mode exit stack, restore registers
	 * (except exit status) and return
	 */
	testl	%edi, %edi
	jz	reset
	movw	%dx, %ss
	movl	%edi, %esp
	movw	%sp, %bp
	movl	%eax, 36(%bp)
	popw	%ds
	popw	%es
	popw	%fs
	popw	%gs
	popal
	popfl
	lret

reset:
	/* Perform a warm reset */
	movw	$0x1234, %ax
	movw	%ax, 0x472
	movb	$0xfe, %al
	outb	%al, $0x64

	/* If reset failed, halt the CPU */
	cli
	hlt

.align 16
gdt:
	/* Dummy */
	.word	0,0,0,0
	/* Unused */
	.word	0,0,0,0
	/* Unused */
	.word	0,0,0,0
	/* Unused */
	.word	0,0,0,0
	/* 16 bit real mode code segment */
	.word	0xffff, ( LOW_TEST_ADR & 0xffff )
	.word	( 0x9b00 | ( LOW_TEST_ADR >> 16 ) ), 0
	/* 16 bit real mode data segment */
	.word	0xffff, 0, 0x9300, 0
gdt_end:
gdt_descr:
	.word	gdt_end - gdt - 1
	.long	gdt

.align 16
idt_descr:
	.word	0xffff
	.long	0

.globl exit_esp
exit_esp:
	.long 0
