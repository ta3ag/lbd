<<<<<<< HEAD
;RUN: llc < %s -march=r600 -mcpu=redwood | FileCheck %s

;CHECK: TRUNC * T{{[0-9]+\.[XYZW], T[0-9]+\.[XYZW]}}
=======
; RUN: llc < %s -march=r600 -mcpu=redwood | FileCheck --check-prefix=R600-CHECK %s
; RUN: llc < %s -march=r600 -mcpu=verde -verify-machineinstrs | FileCheck --check-prefix=SI-CHECK %s

; R600-CHECK: @amdgpu_trunc
; R600-CHECK: TRUNC T{{[0-9]+\.[XYZW]}}, KC0[2].Z
; SI-CHECK: @amdgpu_trunc
; SI-CHECK: V_TRUNC_F32
>>>>>>> llvmtrunk/master

define void @test() {
   %r0 = call float @llvm.R600.load.input(i32 0)
   %r1 = call float @llvm.AMDGPU.trunc( float %r0)
   call void @llvm.AMDGPU.store.output(float %r1, i32 0)
   ret void
}

declare float @llvm.R600.load.input(i32) readnone

declare void @llvm.AMDGPU.store.output(float, i32)

declare float @llvm.AMDGPU.trunc(float ) readnone
