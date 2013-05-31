	.section .mdebug.abi32
	.previous
	.file	"ch11_1.bc"
	.text
	.globl	main
	.align	2
	.type	main,@function
	.ent	main                    # @main
main:
	.cfi_startproc
	.frame	$sp,16,$lr
	.mask 	0x00000000,0
	.set	noreorder
	.set	nomacro
# BB#0:                                 # %entry
	addiu	$sp, $sp, -16
$tmp1:
	.cfi_def_cfa_offset 16
	addiu	$3, $zero, 0
	st	$3, 12($sp)
	st	$3, 8($sp)
	addiu	$2, $zero, 1
	st	$2, 4($sp)
	addiu	$4, $zero, 2
	st	$4, 0($sp)
	ld	$4, 8($sp)
	cmp	$sw, $4, $3
	jne	$sw, $BB0_2
	jmp	$BB0_1
$BB0_1:                                 # %if.then
	ld	$4, 8($sp)
	addiu	$4, $4, 1
	st	$4, 8($sp)
$BB0_2:                                 # %if.end
	ld	$4, 4($sp)
	cmp	$sw, $4, $3
	jne	$sw, $BB0_4
	jmp	$BB0_3
$BB0_4:                                 # %if.else
	addiu	$3, $zero, -1
	ld	$4, 4($sp)
	cmp	$sw, $4, $3
	jgt	$sw, $BB0_6
	jmp	$BB0_5
$BB0_3:                                 # %if.then2
	ld	$3, 4($sp)
	ld	$4, 8($sp)
	add	$3, $4, $3
	st	$3, 8($sp)
	jmp	$BB0_6
$BB0_5:                                 # %if.then4
	ld	$3, 8($sp)
	addiu	$4, $3, -1
	st	$4, 8($sp)
	st	$3, 8($sp)
$BB0_6:                                 # %if.end6
	ld	$3, 0($sp)
	cmp	$sw, $3, $2
	jlt	$sw, $BB0_8
	jmp	$BB0_7
$BB0_7:                                 # %if.then8
	ld	$2, 0($sp)
	addiu	$2, $2, 1
	st	$2, 0($sp)
$BB0_8:                                 # %if.end10
	ld	$2, 8($sp)
	addiu	$sp, $sp, 16
	ret	$2
	.set	macro
	.set	reorder
	.end	main
$tmp2:
	.size	main, ($tmp2)-main
	.cfi_endproc


