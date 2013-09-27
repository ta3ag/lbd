; RUN: llc  < %s -march=cpu0el | FileCheck %s 
; RUN: llc  < %s -march=cpu0   | FileCheck %s

define void @f(i64 %l, i64* nocapture %p) nounwind {
entry:
; CHECK: lui ${{[0-9]+}}, 37035
; CHECK: ori ${{[0-9]+}}, ${{[0-9]+}}, 52719
;	CHECK: ld	${{[0-9]+}}, 0($sp)
;	CHECK: addu	
; CHECK: cmp $sw,  ${{[0-9]+}}, ${{[0-9]+}}
; CHECK: andi ${{[0-9]+}}, $sw, 1
; CHECK: lui ${{[0-9]+}}, 4660
; CHECK: ori ${{[0-9]+}}, ${{[0-9]+}}, 22136
; CHECK: addu
;	CHECK: ld	$[[R0:[0-9]+]], 4($sp)
;	CHECK: addu
  %add = add i64 %l, 1311768467294899695
  store i64 %add, i64* %p, align 4 
  ret void
}

