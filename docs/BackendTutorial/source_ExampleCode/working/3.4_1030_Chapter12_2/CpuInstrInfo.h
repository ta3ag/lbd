//===-- CpuInstrInfo.h - Cpu Instruction Information ----------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains the Cpu implementation of the TargetInstrInfo class.
//
//===----------------------------------------------------------------------===//

#ifndef CPUINSTRUCTIONINFO_H
#define CPUINSTRUCTIONINFO_H

#include "Cpu.h"
#include "CpuRegisterInfo.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Target/TargetInstrInfo.h"

#define GET_INSTRINFO_HEADER
#include "CpuGenInstrInfo.inc"

namespace llvm {

class CpuInstrInfo : public CpuGenInstrInfo {
  CpuTargetMachine &TM;
  const CpuRegisterInfo RI;
public:
  explicit CpuInstrInfo(CpuTargetMachine &TM);

  /// getRegisterInfo - TargetInstrInfo is a superset of MRegister info.  As
  /// such, whenever a client has an instance of instruction info, it should
  /// always be able to get register info as well (through this method).
  ///
  virtual const CpuRegisterInfo &getRegisterInfo() const;

public:
  virtual void copyPhysReg(MachineBasicBlock &MBB,
                           MachineBasicBlock::iterator MI, DebugLoc DL,
                           unsigned DestReg, unsigned SrcReg,
                           bool KillSrc) const;

  virtual void storeRegToStackSlot(MachineBasicBlock &MBB,
                                   MachineBasicBlock::iterator MBBI,
                                   unsigned SrcReg, bool isKill, int FrameIndex,
                                   const TargetRegisterClass *RC,
                                   const TargetRegisterInfo *TRI) const;

  virtual void loadRegFromStackSlot(MachineBasicBlock &MBB,
                                    MachineBasicBlock::iterator MBBI,
                                    unsigned DestReg, int FrameIndex,
                                    const TargetRegisterClass *RC,
                                    const TargetRegisterInfo *TRI) const;

  virtual MachineInstr* emitFrameIndexDebugValue(MachineFunction &MF,
                                                 int FrameIx, uint64_t Offset,
                                                 const MDNode *MDPtr,
                                                 DebugLoc DL) const;
  /// Expand Pseudo instructions into real backend instructions
  virtual bool expandPostRAPseudo(MachineBasicBlock::iterator MI) const;

private:
  void ExpandRetLR(MachineBasicBlock &MBB, MachineBasicBlock::iterator I,
                   unsigned Opc) const;
};
}

#endif
