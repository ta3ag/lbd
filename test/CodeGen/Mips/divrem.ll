<<<<<<< HEAD
; RUN: llc -march=mips < %s | FileCheck %s

; CHECK: div $zero,
=======
; RUN: llc -march=mips -verify-machineinstrs < %s |\
; RUN: FileCheck %s -check-prefix=TRAP
; RUN: llc -march=mips -mno-check-zero-division < %s |\
; RUN: FileCheck %s -check-prefix=NOCHECK

; TRAP-LABEL: sdiv1:
; TRAP: div $zero, ${{[0-9]+}}, $[[R0:[0-9]+]]
; TRAP: teq $[[R0]], $zero, 7
; TRAP: mflo

; NOCHECK-LABEL: sdiv1:
; NOCHECK-NOT: teq
; NOCHECK: .end sdiv1

@g0 = common global i32 0, align 4
@g1 = common global i32 0, align 4

>>>>>>> llvmtrunk/master
define i32 @sdiv1(i32 %a0, i32 %a1) nounwind readnone {
entry:
  %div = sdiv i32 %a0, %a1
  ret i32 %div
}

<<<<<<< HEAD
; CHECK: div $zero,
=======
; TRAP-LABEL: srem1:
; TRAP: div $zero, ${{[0-9]+}}, $[[R0:[0-9]+]]
; TRAP: teq $[[R0]], $zero, 7
; TRAP: mfhi

>>>>>>> llvmtrunk/master
define i32 @srem1(i32 %a0, i32 %a1) nounwind readnone {
entry:
  %rem = srem i32 %a0, %a1
  ret i32 %rem
}

<<<<<<< HEAD
; CHECK: divu $zero,
=======
; TRAP-LABEL: udiv1:
; TRAP: divu $zero, ${{[0-9]+}}, $[[R0:[0-9]+]]
; TRAP: teq $[[R0]], $zero, 7
; TRAP: mflo

>>>>>>> llvmtrunk/master
define i32 @udiv1(i32 %a0, i32 %a1) nounwind readnone {
entry:
  %div = udiv i32 %a0, %a1
  ret i32 %div
}

<<<<<<< HEAD
; CHECK: divu $zero,
=======
; TRAP-LABEL: urem1:
; TRAP: divu $zero, ${{[0-9]+}}, $[[R0:[0-9]+]]
; TRAP: teq $[[R0]], $zero, 7
; TRAP: mfhi

>>>>>>> llvmtrunk/master
define i32 @urem1(i32 %a0, i32 %a1) nounwind readnone {
entry:
  %rem = urem i32 %a0, %a1
  ret i32 %rem
}

; CHECK: div $zero,
define i32 @sdivrem1(i32 %a0, i32 %a1, i32* nocapture %r) nounwind {
entry:
  %rem = srem i32 %a0, %a1
  store i32 %rem, i32* %r, align 4
  %div = sdiv i32 %a0, %a1
  ret i32 %div
}

; CHECK: divu $zero,
define i32 @udivrem1(i32 %a0, i32 %a1, i32* nocapture %r) nounwind {
entry:
  %rem = urem i32 %a0, %a1
  store i32 %rem, i32* %r, align 4
  %div = udiv i32 %a0, %a1
  ret i32 %div
}

define i32 @killFlags() {
entry:
  %0 = load i32* @g0, align 4
  %1 = load i32* @g1, align 4
  %div = sdiv i32 %0, %1
  ret i32 %div
}
