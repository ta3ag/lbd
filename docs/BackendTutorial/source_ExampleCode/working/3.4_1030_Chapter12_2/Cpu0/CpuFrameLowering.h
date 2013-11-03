//===-- CpuFrameLowering.h - Define frame lowering for Cpu ----*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
//
//
//===----------------------------------------------------------------------===//
#ifndef CPU_FRAMEINFO_H
#define CPU_FRAMEINFO_H

#include "Cpu.h"
#include "CpuSubtarget.h"
#include "llvm/Target/TargetFrameLowering.h"

namespace llvm {
  class CpuSubtarget;

class CpuFrameLowering : public TargetFrameLowering {
protected:
  const CpuSubtarget &STI;

public:
  explicit CpuFrameLowering(const CpuSubtarget &sti)
    : TargetFrameLowering(StackGrowsDown, 8, 0),
      STI(sti) {
  }

  /// emitProlog/emitEpilog - These methods insert prolog and epilog code into
  /// the function.
  void emitPrologue(MachineFunction &MF) const;
  void emitEpilogue(MachineFunction &MF, MachineBasicBlock &MBB) const;
  bool hasFP(const MachineFunction &MF) const;
  void eliminateCallFramePseudoInstr(MachineFunction &MF,
                                     MachineBasicBlock &MBB,
                                     MachineBasicBlock::iterator I) const;
  bool spillCalleeSavedRegisters(MachineBasicBlock &MBB,
                                 MachineBasicBlock::iterator MI,
                                 const std::vector<CalleeSavedInfo> &CSI,
                                 const TargetRegisterInfo *TRI) const;
  void processFunctionBeforeCalleeSavedScan(MachineFunction &MF,
                                            RegScavenger *RS) const;
};

} // End llvm namespace

#endif

