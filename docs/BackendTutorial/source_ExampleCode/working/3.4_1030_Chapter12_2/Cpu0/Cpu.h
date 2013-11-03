//===-- Cpu.h - Top-level interface for Cpu representation ----*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains the entry points for global functions defined in
// the LLVM Cpu back-end.
//
//===----------------------------------------------------------------------===//

#ifndef TARGET_CPU_H
#define TARGET_CPU_H

#include "MCTargetDesc/CpuMCTargetDesc.h"
#include "llvm/Target/TargetMachine.h"

//#define CPU_REDESIGN_INSTRUCTION

namespace llvm {
  class CpuTargetMachine;
  class FunctionPass;

  FunctionPass *createCpuISelDag(CpuTargetMachine &TM);
  FunctionPass *createCpuEmitGPRestorePass(CpuTargetMachine &TM);
  FunctionPass *createCpuDelJmpPass(CpuTargetMachine &TM);

} // end namespace llvm;

#endif
