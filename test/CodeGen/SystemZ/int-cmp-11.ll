; Test 64-bit signed comparisons in which the second operand is a constant.
;
; RUN: llc < %s -mtriple=s390x-linux-gnu | FileCheck %s

; Check comparisons with 0.
define double @f1(double %a, double %b, i64 %i1) {
<<<<<<< HEAD
; CHECK: f1:
; CHECK: cghi %r2, 0
; CHECK-NEXT: j{{g?}}l
=======
; CHECK-LABEL: f1:
; CHECK: cgijl %r2, 0
>>>>>>> llvmtrunk/master
; CHECK: ldr %f0, %f2
; CHECK: br %r14
  %cond = icmp slt i64 %i1, 0
  %res = select i1 %cond, double %a, double %b
  ret double %res
}

; Check comparisons with 1.
define double @f2(double %a, double %b, i64 %i1) {
<<<<<<< HEAD
; CHECK: f2:
; CHECK: cghi %r2, 1
; CHECK-NEXT: j{{g?}}l
=======
; CHECK-LABEL: f2:
; CHECK: cgijle %r2, 0
>>>>>>> llvmtrunk/master
; CHECK: ldr %f0, %f2
; CHECK: br %r14
  %cond = icmp slt i64 %i1, 1
  %res = select i1 %cond, double %a, double %b
  ret double %res
}

; Check the high end of the CGHI range.
define double @f3(double %a, double %b, i64 %i1) {
<<<<<<< HEAD
; CHECK: f3:
=======
; CHECK-LABEL: f3:
; CHECK: cgijl %r2, 127
; CHECK: ldr %f0, %f2
; CHECK: br %r14
  %cond = icmp slt i64 %i1, 127
  %res = select i1 %cond, double %a, double %b
  ret double %res
}

; Check the next value up, which must use CGHI instead.
define double @f4(double %a, double %b, i64 %i1) {
; CHECK-LABEL: f4:
; CHECK: cghi %r2, 128
; CHECK-NEXT: jl
; CHECK: ldr %f0, %f2
; CHECK: br %r14
  %cond = icmp slt i64 %i1, 128
  %res = select i1 %cond, double %a, double %b
  ret double %res
}

; Check the high end of the CGHI range.
define double @f5(double %a, double %b, i64 %i1) {
; CHECK-LABEL: f5:
>>>>>>> llvmtrunk/master
; CHECK: cghi %r2, 32767
; CHECK-NEXT: j{{g?}}l
; CHECK: ldr %f0, %f2
; CHECK: br %r14
  %cond = icmp slt i64 %i1, 32767
  %res = select i1 %cond, double %a, double %b
  ret double %res
}

; Check the next value up, which must use CGFI.
<<<<<<< HEAD
define double @f4(double %a, double %b, i64 %i1) {
; CHECK: f4:
=======
define double @f6(double %a, double %b, i64 %i1) {
; CHECK-LABEL: f6:
>>>>>>> llvmtrunk/master
; CHECK: cgfi %r2, 32768
; CHECK-NEXT: j{{g?}}l
; CHECK: ldr %f0, %f2
; CHECK: br %r14
  %cond = icmp slt i64 %i1, 32768
  %res = select i1 %cond, double %a, double %b
  ret double %res
}

; Check the high end of the CGFI range.
<<<<<<< HEAD
define double @f5(double %a, double %b, i64 %i1) {
; CHECK: f5:
=======
define double @f7(double %a, double %b, i64 %i1) {
; CHECK-LABEL: f7:
>>>>>>> llvmtrunk/master
; CHECK: cgfi %r2, 2147483647
; CHECK-NEXT: j{{g?}}l
; CHECK: ldr %f0, %f2
; CHECK: br %r14
  %cond = icmp slt i64 %i1, 2147483647
  %res = select i1 %cond, double %a, double %b
  ret double %res
}

; Check the next value up, which must use register comparison.
<<<<<<< HEAD
define double @f6(double %a, double %b, i64 %i1) {
; CHECK: f6:
; CHECK: cgr
; CHECK-NEXT: j{{g?}}l
=======
define double @f8(double %a, double %b, i64 %i1) {
; CHECK-LABEL: f8:
; CHECK: cgrjl
>>>>>>> llvmtrunk/master
; CHECK: ldr %f0, %f2
; CHECK: br %r14
  %cond = icmp slt i64 %i1, 2147483648
  %res = select i1 %cond, double %a, double %b
  ret double %res
}

<<<<<<< HEAD
; Check the high end of the negative CGHI range.
define double @f7(double %a, double %b, i64 %i1) {
; CHECK: f7:
; CHECK: cghi %r2, -1
; CHECK-NEXT: j{{g?}}l
=======
; Check the high end of the negative CGIJ range.
define double @f9(double %a, double %b, i64 %i1) {
; CHECK-LABEL: f9:
; CHECK: cgijl %r2, -1
>>>>>>> llvmtrunk/master
; CHECK: ldr %f0, %f2
; CHECK: br %r14
  %cond = icmp slt i64 %i1, -1
  %res = select i1 %cond, double %a, double %b
  ret double %res
}

<<<<<<< HEAD
; Check the low end of the CGHI range.
define double @f8(double %a, double %b, i64 %i1) {
; CHECK: f8:
=======
; Check the low end of the CGIJ range.
define double @f10(double %a, double %b, i64 %i1) {
; CHECK-LABEL: f10:
; CHECK: cgijl %r2, -128
; CHECK: ldr %f0, %f2
; CHECK: br %r14
  %cond = icmp slt i64 %i1, -128
  %res = select i1 %cond, double %a, double %b
  ret double %res
}

; Check the next value down, which must use CGHI instead.
define double @f11(double %a, double %b, i64 %i1) {
; CHECK-LABEL: f11:
; CHECK: cghi %r2, -129
; CHECK-NEXT: jl
; CHECK: ldr %f0, %f2
; CHECK: br %r14
  %cond = icmp slt i64 %i1, -129
  %res = select i1 %cond, double %a, double %b
  ret double %res
}

; Check the low end of the CGHI range.
define double @f12(double %a, double %b, i64 %i1) {
; CHECK-LABEL: f12:
>>>>>>> llvmtrunk/master
; CHECK: cghi %r2, -32768
; CHECK-NEXT: j{{g?}}l
; CHECK: ldr %f0, %f2
; CHECK: br %r14
  %cond = icmp slt i64 %i1, -32768
  %res = select i1 %cond, double %a, double %b
  ret double %res
}

; Check the next value down, which must use CGFI instead.
<<<<<<< HEAD
define double @f9(double %a, double %b, i64 %i1) {
; CHECK: f9:
=======
define double @f13(double %a, double %b, i64 %i1) {
; CHECK-LABEL: f13:
>>>>>>> llvmtrunk/master
; CHECK: cgfi %r2, -32769
; CHECK-NEXT: j{{g?}}l
; CHECK: ldr %f0, %f2
; CHECK: br %r14
  %cond = icmp slt i64 %i1, -32769
  %res = select i1 %cond, double %a, double %b
  ret double %res
}

; Check the low end of the CGFI range.
<<<<<<< HEAD
define double @f10(double %a, double %b, i64 %i1) {
; CHECK: f10:
=======
define double @f14(double %a, double %b, i64 %i1) {
; CHECK-LABEL: f14:
>>>>>>> llvmtrunk/master
; CHECK: cgfi %r2, -2147483648
; CHECK-NEXT: j{{g?}}l
; CHECK: ldr %f0, %f2
; CHECK: br %r14
  %cond = icmp slt i64 %i1, -2147483648
  %res = select i1 %cond, double %a, double %b
  ret double %res
}

; Check the next value down, which must use register comparison.
<<<<<<< HEAD
define double @f11(double %a, double %b, i64 %i1) {
; CHECK: f11:
; CHECK: cgr
; CHECK-NEXT: j{{g?}}l
=======
define double @f15(double %a, double %b, i64 %i1) {
; CHECK-LABEL: f15:
; CHECK: cgrjl
>>>>>>> llvmtrunk/master
; CHECK: ldr %f0, %f2
; CHECK: br %r14
  %cond = icmp slt i64 %i1, -2147483649
  %res = select i1 %cond, double %a, double %b
  ret double %res
}
