/*
 *	pxe.S		Copyright (C) 2012 Michael Brown <mcb30@ipxe.org>
 *
 * pxe.S is loaded at 0x7c00 by the PXE ROM.  It copies the 32-bit
 * portion to 0x10000 and jumps into setup.S.
 */

#include "defs.h"

#define RMSIZE (0x200 + 0x200*SETUPSECS)

#define PXENV_ENTRY 0x0a
#define PXENV_UNDI_SHUTDOWN 0x0005

.code16
.section ".bootsect", "ax", @progbits
.org 0
_pxe:

.globl	_main
_main:
	/* Canonicalise addresses */
	ljmp	$BOOTSEG, $pxe_start

pxe_start:
	/* Store PXENV+ entry point */
	movl	%es:PXENV_ENTRY(%bx), %eax
	movl	%eax, %cs:pxenv_vector

	/* Copy 32-bit portion to TSTLOAD:0000.  Perform copy in
	 * reverse in 1kB blocks, since regions will overlap and we
	 * need to copy more than the 64kB real-mode segment limit.
	 */
	movw	$_syssize, %bx	/* Length is _syssize (paragraphs) */
	addw	$63, %bx
	andw	$~63, %bx	/* Round up to nearest kB */
	movw	$(BOOTSEG + (RMSIZE >> 4)), %ax
	addw	%bx, %ax
	movw	%ax, %ds
	movw	$TSTLOAD, %ax
	addw	%bx, %ax
	movw	%ax, %es
1:	movw	%ds, %ax	/* Decrement %ds and %es by 1kB */
	subw	$64, %ax
	movw	%ax, %ds
	movw	%es, %ax
	subw	$64, %ax
	movw	%ax, %es
	movw	$256, %cx	/* Copy 1kB block */
	xorw	%si, %si
	xorw	%di, %di
	cld
	rep movsl
	subw	$64, %bx
	jnz	1b

	/* Set up %ds and %es for access to local variables */
	movw	%cs, %ax
	movw	%ax, %ds
	movw	%ax, %es

	/* Shutdown NIC */
	movw	$PXENV_UNDI_SHUTDOWN, %bx
	movw	$pxenv_undi_shutdown, %di
	lcall	*pxenv_vector

	/* Jump to setup.S */
	ljmp	$(BOOTSEG + 0x20), $0

pxenv_vector:
	.word	0,0

pxenv_undi_shutdown:
	.word	0		/* Status */

.org 512
_epxe:
