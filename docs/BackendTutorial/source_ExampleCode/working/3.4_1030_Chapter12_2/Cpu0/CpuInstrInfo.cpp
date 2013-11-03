//===-- CpuInstrInfo.cpp - Cpu Instruction Information ------------------===//
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

#include "CpuInstrInfo.h"
#include "CpuTargetMachine.h"
#include "CpuMachineFunction.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#define GET_INSTRINFO_CTOR
#include "CpuGenInstrInfo.inc"

using namespace llvm;

CpuInstrInfo::CpuInstrInfo(CpuTargetMachine &tm)
  : CpuGenInstrInfo(Cpu::ADJCALLSTACKDOWN, Cpu::ADJCALLSTACKUP),
    TM(tm),
    RI(*TM.getSubtargetImpl(), *this) {}

const CpuRegisterInfo &CpuInstrInfo::getRegisterInfo() const {
  return RI;
}

// CpuInstrInfo::copyPhysReg()
void CpuInstrInfo::
copyPhysReg(MachineBasicBlock &MBB,
            MachineBasicBlock::iterator I, DebugLoc DL,
            unsigned DestReg, unsigned SrcReg,
            bool KillSrc) const {
  unsigned Opc = 0, ZeroReg = 0;

  if (Cpu::CPURegsRegClass.contains(DestReg)) { // Copy to CPU Reg.
    if (Cpu::CPURegsRegClass.contains(SrcReg))
      Opc = Cpu::ADD, ZeroReg = Cpu::ZERO;
    else if (SrcReg == Cpu::HI)
      Opc = Cpu::MFHI, SrcReg = 0;
    else if (SrcReg == Cpu::LO)
      Opc = Cpu::MFLO, SrcReg = 0;
#ifndef CPU_REDESIGN_INSTRUCTION
    else if (SrcReg == Cpu::SW)
      Opc = Cpu::MFSW, SrcReg = 0;
#endif // CPU_REDESIGN_INSTRUCTION
  }
  else if (Cpu::CPURegsRegClass.contains(SrcReg)) { // Copy from CPU Reg.
    if (DestReg == Cpu::HI)
      Opc = Cpu::MTHI, DestReg = 0;
    else if (DestReg == Cpu::LO)
      Opc = Cpu::MTLO, DestReg = 0;
#ifndef CPU_REDESIGN_INSTRUCTION
    // Only possibility in (DestReg==SW, SrcReg==CPURegs) is 
    //  cmp $SW, $ZERO, $rc
    else if (DestReg == Cpu::SW)
      Opc = Cpu::MTSW, DestReg = 0;
#endif // CPU_REDESIGN_INSTRUCTION
  }

  assert(Opc && "Cannot copy registers");

  MachineInstrBuilder MIB = BuildMI(MBB, I, DL, get(Opc));

  if (DestReg)
    MIB.addReg(DestReg, RegState::Define);

  if (ZeroReg)
    MIB.addReg(ZeroReg);

  if (SrcReg)
    MIB.addReg(SrcReg, getKillRegState(KillSrc));
}

static MachineMemOperand* GetMemOperand(MachineBasicBlock &MBB, int FI,
                                        unsigned Flag) {
  MachineFunction &MF = *MBB.getParent();
  MachineFrameInfo &MFI = *MF.getFrameInfo();
  unsigned Align = MFI.getObjectAlignment(FI);

  return MF.getMachineMemOperand(MachinePointerInfo::getFixedStack(FI), Flag,
                                 MFI.getObjectSize(FI), Align);
}

//- st SrcReg, MMO(FI)
void CpuInstrInfo::
storeRegToStackSlot(MachineBasicBlock &MBB, MachineBasicBlock::iterator I,
                    unsigned SrcReg, bool isKill, int FI,
                    const TargetRegisterClass *RC,
                    const TargetRegisterInfo *TRI) const {
  DebugLoc DL;
  if (I != MBB.end()) DL = I->getDebugLoc();
  MachineMemOperand *MMO = GetMemOperand(MBB, FI, MachineMemOperand::MOStore);

  unsigned Opc = 0;

  if (Cpu::CPURegsRegClass.hasSubClassEq(RC))
    Opc = Cpu::ST;
  assert(Opc && "Register class not handled!");
  BuildMI(MBB, I, DL, get(Opc)).addReg(SrcReg, getKillRegState(isKill))
    .addFrameIndex(FI).addImm(0).addMemOperand(MMO);
}

void CpuInstrInfo::
loadRegFromStackSlot(MachineBasicBlock &MBB, MachineBasicBlock::iterator I,
                     unsigned DestReg, int FI,
                     const TargetRegisterClass *RC,
                     const TargetRegisterInfo *TRI) const
{
  DebugLoc DL;
  if (I != MBB.end()) DL = I->getDebugLoc();
  MachineMemOperand *MMO = GetMemOperand(MBB, FI, MachineMemOperand::MOLoad);
  unsigned Opc = 0;

  if (Cpu::CPURegsRegClass.hasSubClassEq(RC))
    Opc = Cpu::LD;
  assert(Opc && "Register class not handled!");
  BuildMI(MBB, I, DL, get(Opc), DestReg).addFrameIndex(FI).addImm(0)
    .addMemOperand(MMO);
}

MachineInstr*
CpuInstrInfo::emitFrameIndexDebugValue(MachineFunction &MF, int FrameIx,
                                        uint64_t Offset, const MDNode *MDPtr,
                                        DebugLoc DL) const {
  MachineInstrBuilder MIB = BuildMI(MF, DL, get(Cpu::DBG_VALUE))
    .addFrameIndex(FrameIx).addImm(0).addImm(Offset).addMetadata(MDPtr);
  return &*MIB;
}

// CpuInstrInfo::expandPostRAPseudo
/// Expand Pseudo instructions into real backend instructions
bool CpuInstrInfo::expandPostRAPseudo(MachineBasicBlock::iterator MI) const {
  MachineBasicBlock &MBB = *MI->getParent();

  switch(MI->getDesc().getOpcode()) {
  default:
    return false;
  case Cpu::RetLR:
    ExpandRetLR(MBB, MI, Cpu::RET);
    break;
  }

  MBB.erase(MI);
  return true;
}

void CpuInstrInfo::ExpandRetLR(MachineBasicBlock &MBB,
                                MachineBasicBlock::iterator I,
                                unsigned Opc) const {
  BuildMI(MBB, I, I->getDebugLoc(), get(Opc)).addReg(Cpu::LR);
}


