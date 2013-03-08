	.section .mdebug.abi32
	.previous
	.file	"ch8_3.bc"
	.text
	.globl	_Z5sum_iiz
	.align	2
	.type	_Z5sum_iiz,@function
	.ent	_Z5sum_iiz              # @_Z5sum_iiz
_Z5sum_iiz:
	.frame	$sp,24,$lr
	.mask 	0x00000000,0
	.set	noreorder
	.set	nomacro
# BB#0:
	addiu	$sp, $sp, -24
	ld	$2, 24($sp)
	st	$2, 20($sp)
	addiu	$2, $zero, 0
	st	$2, 16($sp)
	st	$2, 12($sp)
	st	$2, 8($sp)
	addiu	$3, $sp, 28
	st	$3, 4($sp)
	st	$2, 16($sp)
$BB0_1:                                 # =>This Inner Loop Header: Depth=1
	ld	$2, 20($sp)
	ld	$3, 16($sp)
	slt	$2, $3, $2
	beq	$2, $zero, $BB0_4
# BB#2:                                 #   in Loop: Header=BB0_1 Depth=1
	ld	$2, 4($sp)
	addiu	$3, $2, 4
	st	$3, 4($sp)
	ld	$2, 0($2)
	st	$2, 12($sp)
	ld	$3, 8($sp)
	addu	$2, $3, $2
	st	$2, 8($sp)
# BB#3:                                 #   in Loop: Header=BB0_1 Depth=1
	ld	$2, 16($sp)
	addiu	$2, $2, 1
	st	$2, 16($sp)
	jmp	$BB0_1
$BB0_4:
	ld	$2, 8($sp)
	addiu	$sp, $sp, 24
	ret	$lr
	.set	macro
	.set	reorder
	.end	_Z5sum_iiz
$tmp1:
	.size	_Z5sum_iiz, ($tmp1)-_Z5sum_iiz

	.globl	main
	.align	2
	.type	main,@function
	.ent	main                    # @main
main:
	.frame	$sp,88,$lr
	.mask 	0x00004000,-4
	.set	noreorder
	.cpload	$t9
	.set	nomacro
# BB#0:
	addiu	$sp, $sp, -88
	st	$lr, 84($sp)            # 4-byte Folded Spill
	.cprestore	32
	addiu	$2, $zero, 0
	st	$2, 80($sp)
	addiu	$3, $zero, 5
	st	$3, 24($sp)
	addiu	$3, $zero, 4
	st	$3, 20($sp)
	addiu	$3, $zero, 3
	st	$3, 16($sp)
	addiu	$3, $zero, 2
	st	$3, 12($sp)
	addiu	$3, $zero, 1
	st	$3, 8($sp)
	st	$2, 4($sp)
	addiu	$2, $zero, 6
	st	$2, 0($sp)
	ld	$6, %call24(_Z5sum_iiz)($gp)
	jalr	$6
	ld	$gp, 32($sp)
	st	$2, 76($sp)
	ld	$lr, 84($sp)            # 4-byte Folded Reload
	addiu	$sp, $sp, 88
	ret	$lr
	.set	macro
	.set	reorder
	.end	main
$tmp4:
	.size	main, ($tmp4)-main


