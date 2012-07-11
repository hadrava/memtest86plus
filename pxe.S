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
#define PXENV_FILE_CMDLINE 0x00e8

#define CMDLINE_MAGIC 0xa33f

.code16
.section ".bootsect", "ax", @progbits
.org 0
_pxe:

.globl	_main
_main:
	/* Canonicalise addresses */
	ljmp	$BOOTSEG, $pxe_start

	/* Command line magic */
.org 0x20
cmdline_magic:
	.word	0
cmdline_offset:
	.word	0

pxe_start:
	/* Preserve registers, %ss:%esp, and magic marker on PXE stack */
	pushfl
	pushal
	pushw	%gs
	pushw	%fs
	pushw	%es
	pushw	%ds
	pushl	%esp
	pushw	%ss
	pushw	%ax	/* Padding */
	pushl	$EXIT_MAGIC

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

	/* Check for a command line */
	movw	$PXENV_FILE_CMDLINE, %bx
	movw	$pxenv_file_cmdline, %di
	lcall	*pxenv_vector
	testw	%ax, %ax
	jnz	no_cmdline
	movw	pxenv_file_cmdline + 2, %ax
	testw	%ax, %ax
	jz	no_cmdline
	movw	$CMDLINE_MAGIC, cmdline_magic
	movw	$RMSIZE, cmdline_offset
no_cmdline:

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

pxenv_file_cmdline:
	.word	0		/* Status */
	.word	0xffff		/* 64kB max length of command line */
	.word	RMSIZE, BOOTSEG

.org 512
_epxe:
