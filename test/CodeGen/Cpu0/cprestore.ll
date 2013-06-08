; DISABLE: llc -march=mipsel < %s | FileCheck %s
; RUN: false
; XFAIL: *

; CHECK: .set macro
; CHECK: .set at
; CHECK-NEXT: .cprestore
; CHECK: .set noat
; CHECK-NEXT: .set nomacro

;%struct.S = type { [16384 x i32] }
%struct.S = type { i32 }

declare void @foo1(%struct.S* byval align 8 %s)

define void @foo2() nounwind {
entry:
;  %s = alloca %struct.S, align 4
;  call void @foo1(%struct.S* byval %s)
  %s = alloca %struct.S, align 8
  call void @foo1(%struct.S* byval align 8 %s)
  ret void
}

;declare void @foo1(%struct.S* byval)
