; RUN: llc -march=cpu0 -relocation-model=static -cpu0-use-small-section=false -filetype=asm < %s | FileCheck %s -check-prefix=STATIC_LARGE
; RUN: llc -march=cpu0 -relocation-model=static -cpu0-use-small-section=true -filetype=asm < %s | FileCheck %s -check-prefix=STATIC_SMALL
; RUN: llc -march=cpu0 -relocation-model=pic -cpu0-use-small-section=false -filetype=asm < %s | FileCheck %s -check-prefix=STATIC_LARGE
; RUN: llc -march=cpu0 -relocation-model=pic -cpu0-use-small-section=true -filetype=asm < %s | FileCheck %s -check-prefix=STATIC_SMALL

; ModuleID = 'ch7_2.bc'
target datalayout = "E-p:32:32:32-i1:8:8-i8:8:32-i16:16:32-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-n32-S64"
target triple = "mips-unknown-linux-gnu"

%struct.Date = type { i16, i8, i8, i8, i8, i8 }

@b = global [4 x i8] c"abc\00", align 1
@_ZZ9test_charvE5date1 = private unnamed_addr constant { i16, i8, i8, i8, i8, i8, i8 } { i16 2012, i8 11, i8 25, i8 9, i8 40, i8 15, i8 undef }, align 2

; Function Attrs: nounwind
define i32 @_Z9test_charv() #0 {
entry:
  %a = alloca i8, align 1
  %c = alloca i8, align 1
  %date1 = alloca %struct.Date, align 2
  %m = alloca i8, align 1
  %s = alloca i8, align 1
  %0 = load i8* getelementptr inbounds ([4 x i8]* @b, i32 0, i32 1), align 1
  store i8 %0, i8* %a, align 1
  %1 = load i8* getelementptr inbounds ([4 x i8]* @b, i32 0, i32 1), align 1
  store i8 %1, i8* %c, align 1
; STATIC_LARGE:  lui	$[[T0:[0-9]+]], %hi(b)
; STATIC_LARGE:  addiu	$[[T1:[0-9]+]], $[[T0]], %lo(b)
; STATIC_LARGE:  lbu	$[[T3:[0-9]+]], 1($[[T2]])
; STATIC_LARGE:  sb	$[[T3]]
; STATIC_LARGE:  lbu	$[[T1:[0-9]+]], 1($[[T0:[0-9]+]])
; STATIC_LARGE:  sb	$[[T1]]
; STATIC_SMALL:  addiu	$[[T0:[0-9]+]], $gp, %gp_rel(b)
; STATIC_SMALL:  lbu	$[[T1:[0-9]+]], 1($[[T0]])
; STATIC_SMALL:  sb	$[[T2]]
; STATIC_SMALL:  lbu	$[[T1:[0-9]+]], 1($[[T0:[0-9]+]])
; STATIC_SMALL:  sb	$[[T1]]
; PIC_LARGE:  .cpload	$t9
; PIC_LARGE:  lui	$[[T0:[0-9]+]], %got_hi(b)
; PIC_LARGE:  addu	$[[T1:[0-9]+]], $[[T0]], $gp
; PIC_LARGE:  ld	$[[T2:[0-9]+]], %got_lo(b)($[[T1]])
; PIC_LARGE:  lbu	$[[T3:[0-9]+]], 1($[[T2]])
; PIC_LARGE:  sb	$[[T3]]
; PIC_LARGE:  lbu	$[[T1:[0-9]+]], 1($[[T0:[0-9]+]])
; PIC_LARGE:  sb	$[[T1]]
; PIC_SMALL:  .cpload	$t9
; PIC_SMALL:  ld	$[[T0:[0-9]+]], %got(b)($gp)
; PIC_SMALL:  lbu	$[[T1:[0-9]+]], 1($[[T0]])
; PIC_SMALL:  sb	$[[T2]]
; PIC_SMALL:  lbu	$[[T1:[0-9]+]], 1($[[T0:[0-9]+]])
; PIC_SMALL:  sb	$[[T1]]
  %2 = bitcast %struct.Date* %date1 to i8*
  call void @llvm.memcpy.p0i8.p0i8.i32(i8* %2, i8* bitcast ({ i16, i8, i8, i8, i8, i8, i8 }* @_ZZ9test_charvE5date1 to i8*), i32 8, i32 2, i1 false)
  %month = getelementptr inbounds %struct.Date* %date1, i32 0, i32 1
  %3 = load i8* %month, align 1
  store i8 %3, i8* %m, align 1
  %second = getelementptr inbounds %struct.Date* %date1, i32 0, i32 5
  %4 = load i8* %second, align 1
  store i8 %4, i8* %s, align 1
  ret i32 0
}

; Function Attrs: nounwind
declare void @llvm.memcpy.p0i8.p0i8.i32(i8* nocapture, i8* nocapture, i32, i32, i1) #1

attributes #0 = { nounwind "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf"="true" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind }
