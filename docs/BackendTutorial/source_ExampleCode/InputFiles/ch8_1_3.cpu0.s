	.section .mdebug.abi32
	.previous
	.file	"ch8_1_3.bc"
	.text
	.globl	main
	.align	2
	.type	main,@function
	.ent	main                    # @main
main:
	.cfi_startproc
	.frame	$sp,48,$lr
	.mask 	0x00004000,-4
	.set	noreorder
	.cpload	$t9
	.set	nomacro
# BB#0:
	addiu	$sp, $sp, -48
$tmp2:
	.cfi_def_cfa_offset 48
	st	$lr, 44($sp)            # 4-byte Folded Spill
$tmp3:
	.cfi_offset 14, -4
	.cprestore	8
	addiu	$2, $zero, 0
	st	$2, 40($sp)
	addiu	$2, $zero, 1
	st	$2, 36($sp)
	addiu	$2, $zero, 2
	st	$2, 32($sp)
	ld	$3, 36($sp)
	st	$2, 4($sp)
	st	$3, 0($sp)
	ld	$6, %call24(_Z5sum_iii)($gp)
	jalr	$6
	ld	$gp, 8($sp)
	st	$2, 28($sp)
	ld	$lr, 44($sp)            # 4-byte Folded Reload
	addiu	$sp, $sp, 48
	ret	$lr
	.set	macro
	.set	reorder
	.end	main
$tmp4:
	.size	main, ($tmp4)-main
	.cfi_endproc


