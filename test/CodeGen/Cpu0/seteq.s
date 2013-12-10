	.section .mdebug.abi32
	.previous
	.file	"seteq.ll"
	.text
	.globl	test
	.align	2
	.type	test,@function
	.ent	test                    # @test
test:
	.frame	$sp,0,$lr
	.mask 	0x00000000,0
	.set	noreorder
	.cpload	$t9
	.set	nomacro
# BB#0:                                 # %entry
	lui	$2, %got_hi(k)
	addu	$2, $2, $gp
	ld	$2, %got_lo(k)($2)
	ld	$2, 0($2)
	lui	$3, %got_hi(i)
	addu	$3, $3, $gp
	ld	$3, %got_lo(i)($3)
	ld	$3, 0($3)
	xor	$2, $3, $2
	sltiu	$2, $2, 1
	lui	$3, %got_hi(r1)
	addu	$3, $3, $gp
	ld	$3, %got_lo(r1)($3)
	st	$2, 0($3)
	ret	$lr
	.set	macro
	.set	reorder
	.end	test
$tmp0:
	.size	test, ($tmp0)-test

	.type	i,@object               # @i
	.data
	.globl	i
	.align	2
i:
	.4byte	1                       # 0x1
	.size	i, 4

	.type	j,@object               # @j
	.globl	j
	.align	2
j:
	.4byte	10                      # 0xa
	.size	j, 4

	.type	k,@object               # @k
	.globl	k
	.align	2
k:
	.4byte	1                       # 0x1
	.size	k, 4

	.type	r1,@object              # @r1
	.comm	r1,4,4
	.type	r2,@object              # @r2
	.comm	r2,4,4

