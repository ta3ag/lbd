; RUN: llc -march=cpu0 < %s | FileCheck %s
; RUN: llc -march=cpu0 -mcpu=cpu032II < %s | FileCheck %s

define i64 @add64(i64 %u, i64 %v) nounwind  {
entry:
; CHECK: addu
; CHECK: cmp
; CHECK-NEXT: mfsw
; cpu032II: sltu 
; CHECK: addu
; CHECK: addu
; cpu032II: addu
; cpu032II: addu
  %tmp2 = add i64 %u, %v  
  ret i64 %tmp2
}

define i64 @sub64(i64 %u, i64 %v) nounwind  {
entry:
; CHECK: sub64
; CHECK: subu
; CHECK: cmp
; CHECK-NEXT: mfsw
; cpu032II: sltu 
; CHECK: addu
; CHECK: subu
; cpu032II: addu
; cpu032II: subu
  %tmp2 = sub i64 %u, %v
  ret i64 %tmp2
}
