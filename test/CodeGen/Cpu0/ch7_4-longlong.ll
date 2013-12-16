; RUN: llc  -march=cpu0 -mcpu=cpu032I -relocation-model=pic %s -o - | FileCheck %s -check-prefix=cpu032I
; RUN: llc  -march=cpu0 -mcpu=cpu032II -relocation-model=pic %s -o - | FileCheck %s -check-prefix=cpu032II

; ModuleID = 'ch7_4.bc'
target datalayout = "E-p:32:32:32-i1:8:8-i8:8:32-i16:16:32-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-n32-S64"
target triple = "mips-unknown-linux-gnu"

; Function Attrs: nounwind
define i64 @_Z13test_longlongv() #0 {
  %a = alloca i64, align 8
  %b = alloca i64, align 8
  %a1 = alloca i32, align 4
  %b1 = alloca i32, align 4
  %c = alloca i64, align 8
  %d = alloca i64, align 8
  %e = alloca i64, align 8
  %f = alloca i64, align 8
  store i64 12884901890, i64* %a, align 8
  store i64 4294967297, i64* %b, align 8
  store i32 50335744, i32* %a1, align 4
  store i32 33558528, i32* %b1, align 4
  %1 = load i64* %a, align 8
  %2 = load i64* %b, align 8
  %3 = add nsw i64 %1, %2
  store i64 %3, i64* %c, align 8
  
; cpu032I:  lui	$[[T0:[0-9]+]], 768
; cpu032I:  ori	${{[0-9]+}}, $[[T0]], 4096
; cpu032I:  lui	$[[T0:[0-9]+]], 512
; cpu032I:  ori	${{[0-9]+}}, $[[T0]], 4096

; cpu032I:  addu	${{[0-9]+}}, ${{[0-9]+}}, ${{[0-9]+}}

; cpu032I:  cmp	$sw, ${{[0-9]+}}, ${{[0-9]+}}
; cpu032I:  mfsw	$[[T0:[0-9]+]]
; cpu032I:  andi	$[[T1:[0-9]+]], $[[T0]], 1
; cpu032I:  addu	${{[0-9]+}}, ${{[0-9]+}}, ${{[0-9]+}}
; cpu032I:  addu	${{[0-9]+}}, ${{[0-9]+}}, ${{[0-9]+}}

  %4 = load i64* %a, align 8
  %5 = load i64* %b, align 8
  %6 = sub nsw i64 %4, %5
  store i64 %6, i64* %d, align 8
; cpu032I:  cmp	$sw, ${{[0-9]+}}, ${{[0-9]+}}
; cpu032I:  mfsw	$[[T0:[0-9]+]]
; cpu032I:  andi	$[[T1:[0-9]+]], $[[T0]], 1
; cpu032I:  addu	${{[0-9]+}}, ${{[0-9]+}}, ${{[0-9]+}}
; cpu032I:  subu	${{[0-9]+}}, ${{[0-9]+}}, ${{[0-9]+}}

  %7 = load i64* %a, align 8
  %8 = load i64* %b, align 8
  %9 = mul nsw i64 %7, %8
  store i64 %9, i64* %e, align 8
; cpu032I:  multu	${{[0-9]+}}, ${{[0-9]+}}
; cpu032I:  mflo	$t9
; cpu032I:  mfhi	${{[0-9]+}}

; cpu032I:  mul	${{[0-9]+}}, ${{[0-9]+}}, ${{[0-9]+}}
; cpu032I:  addu	${{[0-9]+}}, ${{[0-9]+}}, ${{[0-9]+}}
; cpu032I:  mul	${{[0-9]+}}, ${{[0-9]+}}, ${{[0-9]+}}
; cpu032I:  addu	${{[0-9]+}}, ${{[0-9]+}}, ${{[0-9]+}}
	
  %10 = load i32* %a1, align 4
  %11 = sext i32 %10 to i64
  %12 = load i32* %b1, align 4
  %13 = sext i32 %12 to i64
  %14 = mul nsw i64 %11, %13
  store i64 %14, i64* %f, align 8
; cpu032I:  multu	${{[0-9]+}}, ${{[0-9]+}}
; cpu032I:  mflo	$t9
; cpu032I:  mfhi	${{[0-9]+}}

; cpu032I:  addu	${{[0-9]+}}, ${{[0-9]+}}, ${{[0-9]+}}
; cpu032I:  addu	${{[0-9]+}}, ${{[0-9]+}}, ${{[0-9]+}}
; cpu032I:  addu	${{[0-9]+}}, ${{[0-9]+}}, ${{[0-9]+}}

  %15 = load i64* %c, align 8
  %16 = load i64* %d, align 8
  %17 = add nsw i64 %15, %16
  %18 = load i64* %e, align 8
  %19 = add nsw i64 %17, %18
  %20 = load i64* %f, align 8
  %21 = add nsw i64 %19, %20
  ret i64 %21
}

attributes #0 = { nounwind }
