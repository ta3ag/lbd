; Test 32-bit atomic stores.
;
; RUN: llc < %s -mtriple=s390x-linux-gnu | FileCheck %s

define void @f1(i32 %val, i32 *%src) {
<<<<<<< HEAD
; CHECK: f1:
; CHECK: l %r0, 0(%r3)
; CHECK: [[LABEL:\.[^:]*]]:
; CHECK: cs %r0, %r2, 0(%r3)
; CHECK: j{{g?}}lh [[LABEL]]
=======
; CHECK-LABEL: f1:
; CHECK: st %r2, 0(%r3)
; CHECK: bcr 1{{[45]}}, %r0
>>>>>>> llvmtrunk/master
; CHECK: br %r14
  store atomic i32 %val, i32 *%src seq_cst, align 4
  ret void
}
