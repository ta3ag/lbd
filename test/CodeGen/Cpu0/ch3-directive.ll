; RUN: llc -march=cpu0el < %s | FileCheck %s

define i32 @main() #0 {
entry:
; CHECK: 	.section .mdebug.abi32
; CHECK: 	.previous
; CHECK: 	.file	"<stdin>"
; CHECK: 	.text
; CHECK: 	.globl	main
; CHECK: 	.align	2
; CHECK: 	.type	main,@function
; CHECK: 	.ent	main                    # @main
; CHECK: main:
; CHECK: 	.cfi_startproc
; CHECK: 	.frame	$sp,8,$lr
; CHECK: 	.mask 	0x00000000,0
; CHECK: 	.set	noreorder
; CHECK: 	.set	nomacro
; CHECK: 	.cfi_def_cfa_offset 8
; CHECK: 	.size	main, ($tmp2)-main
; CHECK: 	.cfi_endproc

  %retval = alloca i32, align 4
  store i32 0, i32* %retval
  ret i32 0
}

