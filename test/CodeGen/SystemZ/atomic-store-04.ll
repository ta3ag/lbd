; Test 64-bit atomic stores.
;
; RUN: llc < %s -mtriple=s390x-linux-gnu | FileCheck %s

define void @f1(i64 %val, i64 *%src) {
<<<<<<< HEAD
; CHECK: f1:
; CHECK: lg %r0, 0(%r3)
; CHECK: [[LABEL:\.[^:]*]]:
; CHECK: csg %r0, %r2, 0(%r3)
; CHECK: j{{g?}}lh [[LABEL]]
=======
; CHECK-LABEL: f1:
; CHECK: stg %r2, 0(%r3)
; CHECK: bcr 1{{[45]}}, %r0
>>>>>>> llvmtrunk/master
; CHECK: br %r14
  store atomic i64 %val, i64 *%src seq_cst, align 8
  ret void
}
