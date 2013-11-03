//===-- CpuMachineFunctionInfo.cpp - Private data used for Cpu ----------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "CpuMachineFunction.h"
#include "CpuInstrInfo.h"
#include "CpuSubtarget.h"
#include "MCTargetDesc/CpuBaseInfo.h"
#include "llvm/IR/Function.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"

using namespace llvm;

bool FixGlobalBaseReg = true;

bool CpuFunctionInfo::globalBaseRegFixed() const {
  return FixGlobalBaseReg;
}

bool CpuFunctionInfo::globalBaseRegSet() const {
  return GlobalBaseReg;
}

unsigned CpuFunctionInfo::getGlobalBaseReg() {
  // Return if it has already been initialized.
  if (GlobalBaseReg)
    return GlobalBaseReg;

  if (FixGlobalBaseReg) // $gp is the global base register.
    return GlobalBaseReg = Cpu::GP;

  const TargetRegisterClass *RC;
  RC = (const TargetRegisterClass*)&Cpu::CPURegsRegClass;

  return GlobalBaseReg = MF.getRegInfo().createVirtualRegister(RC);
}

void CpuFunctionInfo::anchor() { }
