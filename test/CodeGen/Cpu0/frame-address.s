	.section .mdebug.abi32
	.previous
	.file	"frame-address.ll"
	.text
	.globl	f
	.align	2
	.type	f,@function
	.set	nomips16                # @f
	.ent	f
f:
	.frame	$fp,8,$ra
	.mask 	0x40000000,-4
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
# BB#0:                                 # %entry
	addiu	$sp, $sp, -8
	sw	$fp, 4($sp)             # 4-byte Folded Spill
	move	$fp, $sp
	move	$2, $fp
	move	$sp, $fp
	lw	$fp, 4($sp)             # 4-byte Folded Reload
	jr	$ra
	addiu	$sp, $sp, 8
	.set	at
	.set	macro
	.set	reorder
	.end	f
$tmp3:
	.size	f, ($tmp3)-f


