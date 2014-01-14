; RUN: llc -march=mipsel < %s | FileCheck %s

; "D": Second word of double word. This works for any memory element
; double or single.
; CHECK: #APP
; CHECK-NEXT: lw ${{[0-9]+}},4(${{[0-9]+}});
; CHECK-NEXT: #NO_APP

; No "D": First word of double word. This works for any memory element 
; double or single.
; CHECK: #APP
; CHECK-NEXT: lw ${{[0-9]+}},0(${{[0-9]+}});
; CHECK-NEXT: #NO_APP

;int b[8] = {0,1,2,3,4,5,6,7};
;int main()
;{
;  int i;
; 
;  // The first word. Notice, no 'D'
;  { asm (
;    "lw    %0,%1;\n"
;    : "=r" (i) : "m" (*(b+4)));}
; 
;  // The second word
;  { asm (
;    "lw    %0,%D1;\n"
;    : "=r" (i) "m" (*(b+4)));}
;}

@b = common global [20 x i32] zeroinitializer, align 4

define void @main() {
entry:
  tail call void asm sideeffect "    ld    $0,${1};", "r,*m,~{$11}"(i32 undef, i32* getelementptr inbounds ([20 x i32]* @b, i32 0, i32 3))
  ret void
}

attributes #0 = { nounwind }

