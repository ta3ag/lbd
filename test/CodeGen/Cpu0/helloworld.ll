; RUN: llc  -march=cpu0el -relocation-model=pic < %s | FileCheck %s -check-prefix=PIC
; RUN: llc  -march=mipsel -mcpu=mips16 -relocation-model=pic -O3 < %s | FileCheck %s -check-prefix=C2
; RUN: llc  -march=mipsel -mcpu=mips16 -relocation-model=pic -O3 < %s | FileCheck %s -check-prefix=PE
; RUN: llc  -march=mipsel -mcpu=mips16 -relocation-model=static -O3 < %s | FileCheck %s -check-prefix=ST1
; RUN: llc  -march=mipsel -mcpu=mips16 -relocation-model=static -O3 < %s | FileCheck %s -check-prefix=ST2
;
; RUN: llc  -march=mipsel -mcpu=mips16 -relocation-model=pic -O3 < %s | FileCheck %s -check-prefix=SR
; RUN: llc  -march=mipsel -mcpu=mips32  -relocation-model=pic -O3 < %s | FileCheck %s -check-prefix=SR32

; check got .str as well as save/restore $lr

@.str = private unnamed_addr constant [13 x i8] c"hello world\0A\00", align 1

define i32 @main() nounwind {
entry:
  %call = call i32 (i8*, ...)* @printf(i8* getelementptr inbounds ([13 x i8]* @.str, i32 0, i32 0))
  ret i32 0

; PIC: .ent main
; PIC: .set noreorder
; PIC: .cpload $t9
; PIC: .set nomacro
; PIC: st $lr, [[FS:[0-9]+]]($sp)
; PIC: ld	$[[R0:[0-9]+]], %got($.str)($gp)
; PIC: addiu	${{[0-9]+}}, $[[R0:[0-9]+]], %lo($.str)
; PIC: ld	$t9, %call24(printf)($gp)
; PIC: jalr $t9
; PIC: ld $lr, [[FS]]($sp)
}

; PIC: .set macro
; PIC: .set reorder
; PIC: .end main
declare i32 @printf(i8*, ...)
