; RUN: llc -march=cpu0 < %s | FileCheck %s

define i32 @f1(double %d) nounwind readnone {
entry:
; CHECK: %call24(__fixdfsi)
  %conv = fptosi double %d to i32
  ret i32 %conv
}
