//===-- CpuCallingConv.td - Calling Conventions for Cpu --*- tablegen -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
// This describes the calling conventions for Cpu architecture.
//===----------------------------------------------------------------------===//

/// CCIfSubtarget - Match if the current subtarget has a feature F.
class CCIfSubtarget<string F, CCAction A>:
  CCIf<!strconcat("State.getTarget().getSubtarget<CpuSubtarget>().", F), A>;

def RetCC_CpuEABI : CallingConv<[
  // i32 are returned in registers V0, V1, A0, A1
  CCIfType<[i32], CCAssignToReg<[V0, V1, A0, A1]>>
]>;

//===----------------------------------------------------------------------===//
// Cpu EABI Calling Convention
//===----------------------------------------------------------------------===//

def CC_CpuEABI : CallingConv<[
  // Promote i8/i16 arguments to i32.
  CCIfType<[i8, i16], CCPromoteToType<i32>>,
  // Integer values get stored in stack slots that are 4 bytes in
  // size and 4-byte aligned.
  CCIfType<[i32], CCAssignToStack<4, 4>>
]>;


//===----------------------------------------------------------------------===//
// Cpu Calling Convention Dispatch
//===----------------------------------------------------------------------===//

def CC_Cpu : CallingConv<[
  CCDelegateTo<CC_CpuEABI>
]>;


def RetCC_Cpu : CallingConv<[
  CCDelegateTo<RetCC_CpuEABI>
]>;

def CSR_O32 : CalleeSavedRegs<(add LR, FP,
                                   (sequence "S%u", 2, 0))>;

