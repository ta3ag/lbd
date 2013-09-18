; RUN: llc -march=cpu0 < %s | FileCheck %s

define i32 @test_sext_inreg_from_32(i32 %in) {
; CHECK: test_sext_inreg_from_32:

  %small = trunc i32 %in to i1
  %ext = sext i1 %small to i32

  ; Different registers are of course, possible, though suboptimal. This is
  ; making sure that a 64-bit "(sext_inreg (anyext GPR32), i1)" uses the 64-bit
  ; sbfx rather than just 32-bits.
; CHECK: sbfx x0, x0, #0, #1
  ret i32 %ext
}

