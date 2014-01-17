
; /// start
; The @_start() exist to prevent lld linker error.
; Real entry (first instruction) is from cpu0BootAtomContent of 
; Cpu0RelocationPass.cpp jump to asm("start:") of start.cpp.
define void @_start() nounwind {
entry:
  ret void
}

define void @__stack_chk_fail() nounwind {
entry:
  ret void
}

define void @__stack_chk_guard() nounwind {
entry:
  ret void
}
