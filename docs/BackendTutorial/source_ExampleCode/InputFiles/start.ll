; ModuleID = 'start.bc'
target datalayout = "E-p:32:32:32-i1:8:8-i8:8:32-i16:16:32-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-n32-S64"
target triple = "mips-unknown-linux-gnu"

define void @_start() #0 {
entry:
  call void asm sideeffect "lui   $$1,  0x7", ""() #1, !srcloc !0
  call void asm sideeffect "ori   $$1,  $$1, 0xfff0", ""() #1, !srcloc !1
  call void asm sideeffect "ld    $$gp, 0($$1)", ""() #1, !srcloc !2
  call void asm sideeffect "addiu $$1,  $$ZERO, 0x10", ""() #1, !srcloc !3
  call void asm sideeffect "st    $$1,  0($$gp)", ""() #1, !srcloc !4
  call void asm sideeffect "addiu $$1,\09$$ZERO, 0", ""() #1, !srcloc !5
  call void asm sideeffect "addiu $$2,\09$$ZERO, 0", ""() #1, !srcloc !6
  call void asm sideeffect "addiu $$3,\09$$ZERO, 0", ""() #1, !srcloc !7
  call void asm sideeffect "addiu $$4,\09$$ZERO, 0", ""() #1, !srcloc !8
  call void asm sideeffect "addiu $$5,\09$$ZERO, 0", ""() #1, !srcloc !9
  call void asm sideeffect "addiu $$6,\09$$ZERO, 0", ""() #1, !srcloc !10
  call void asm sideeffect "addiu $$7,\09$$ZERO, 0", ""() #1, !srcloc !11
  call void asm sideeffect "addiu $$8,\09$$ZERO, 0", ""() #1, !srcloc !12
  call void asm sideeffect "addiu $$9,\09$$ZERO, 0", ""() #1, !srcloc !13
  call void asm sideeffect "addiu $$11, $$ZERO, 0", ""() #1, !srcloc !14
  call void asm sideeffect "addiu $$12, $$ZERO, 0", ""() #1, !srcloc !15
  call void asm sideeffect "addiu $$sp, $$zero, 0x6ffc", ""() #1, !srcloc !16
  %call = call i32 @main()
  call void asm sideeffect "addiu $$14, $$ZERO, -1", ""() #1, !srcloc !17
  call void asm sideeffect "ret $$14", ""() #1, !srcloc !18
  ret void
}

declare i32 @main() #0

attributes #0 = { "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf"="true" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind }

!0 = metadata !{i32 966}
!1 = metadata !{i32 991}
!2 = metadata !{i32 1023}
!3 = metadata !{i32 1086}
!4 = metadata !{i32 1119}
!5 = metadata !{i32 1178}
!6 = metadata !{i32 1207}
!7 = metadata !{i32 1236}
!8 = metadata !{i32 1265}
!9 = metadata !{i32 1294}
!10 = metadata !{i32 1323}
!11 = metadata !{i32 1352}
!12 = metadata !{i32 1381}
!13 = metadata !{i32 1410}
!14 = metadata !{i32 1439}
!15 = metadata !{i32 1469}
!16 = metadata !{i32 1502}
!17 = metadata !{i32 1547}
!18 = metadata !{i32 1578}
