; ModuleID = 'start.bc'
target datalayout = "E-p:32:32:32-i1:8:8-i8:8:32-i16:16:32-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-n32-S64"
target triple = "mips-unknown-linux-gnu"

define void @_Z5startv() #0 {
entry:
  call void asm sideeffect "lui   $$1,  0x7", ""() #1, !srcloc !0
  call void asm sideeffect "ori   $$1,  $$1, 0xfff0", ""() #1, !srcloc !1
  call void asm sideeffect "ld    $$gp, 0($$1)", ""() #1, !srcloc !2
  call void asm sideeffect "addiu $$1,\09$$ZERO, 0", ""() #1, !srcloc !3
  call void asm sideeffect "addiu $$2,\09$$ZERO, 0", ""() #1, !srcloc !4
  call void asm sideeffect "addiu $$3,\09$$ZERO, 0", ""() #1, !srcloc !5
  call void asm sideeffect "addiu $$4,\09$$ZERO, 0", ""() #1, !srcloc !6
  call void asm sideeffect "addiu $$5,\09$$ZERO, 0", ""() #1, !srcloc !7
  call void asm sideeffect "addiu $$6,\09$$ZERO, 0", ""() #1, !srcloc !8
  call void asm sideeffect "addiu $$7,\09$$ZERO, 0", ""() #1, !srcloc !9
  call void asm sideeffect "addiu $$8,\09$$ZERO, 0", ""() #1, !srcloc !10
  call void asm sideeffect "addiu $$9,\09$$ZERO, 0", ""() #1, !srcloc !11
  call void asm sideeffect "addiu $$10,\09$$ZERO, 0", ""() #1, !srcloc !12
  call void asm sideeffect "addiu $$fp, $$ZERO, 0", ""() #1, !srcloc !13
  call void asm sideeffect "addiu $$sp, $$zero, 0x6ffc", ""() #1, !srcloc !14
  %call = call i32 @main()
  call void asm sideeffect "addiu $$lr, $$ZERO, -1", ""() #1, !srcloc !15
  call void asm sideeffect "ret $$lr", ""() #1, !srcloc !16
  ret void
}

declare i32 @main() #0

attributes #0 = { "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf"="true" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind }

!0 = metadata !{i32 1097}
!1 = metadata !{i32 1122}
!2 = metadata !{i32 1154}
!3 = metadata !{i32 -2147476750}
!4 = metadata !{i32 -2147476719}
!5 = metadata !{i32 -2147476688}
!6 = metadata !{i32 -2147476657}
!7 = metadata !{i32 -2147476626}
!8 = metadata !{i32 -2147476595}
!9 = metadata !{i32 -2147476564}
!10 = metadata !{i32 -2147476533}
!11 = metadata !{i32 -2147476502}
!12 = metadata !{i32 -2147476471}
!13 = metadata !{i32 -2147476439}
!14 = metadata !{i32 1231}
!15 = metadata !{i32 1276}
!16 = metadata !{i32 1307}
