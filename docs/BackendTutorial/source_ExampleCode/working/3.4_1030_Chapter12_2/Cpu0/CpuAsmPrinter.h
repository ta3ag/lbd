//===-- CpuAsmPrinter.h - Cpu LLVM Assembly Printer ----------*- C++ -*--===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// Cpu Assembly printer class.
//
//===----------------------------------------------------------------------===//

#ifndef CPUASMPRINTER_H
#define CPUASMPRINTER_H

#include "CpuMachineFunction.h"
#include "CpuMCInstLower.h"
#include "CpuSubtarget.h"
#include "llvm/CodeGen/AsmPrinter.h"
#include "llvm/Support/Compiler.h"
#include "llvm/Target/TargetMachine.h"

namespace llvm {
class MCStreamer;
class MachineInstr;
class MachineBasicBlock;
class Module;
class raw_ostream;

class LLVM_LIBRARY_VISIBILITY CpuAsmPrinter : public AsmPrinter {

  void EmitInstrWithMacroNoAT(const MachineInstr *MI);

public:

  const CpuSubtarget *Subtarget;
  const CpuFunctionInfo *CpuFI;
  CpuMCInstLower MCInstLowering;

  explicit CpuAsmPrinter(TargetMachine &TM,  MCStreamer &Streamer)
    : AsmPrinter(TM, Streamer), MCInstLowering(*this) {
    Subtarget = &TM.getSubtarget<CpuSubtarget>();
  }

  virtual const char *getPassName() const {
    return "Cpu Assembly Printer";
  }

  virtual bool runOnMachineFunction(MachineFunction &MF);

//- EmitInstruction() must exists or will have run time error.
  void EmitInstruction(const MachineInstr *MI);
  void printSavedRegsBitmask(raw_ostream &O);
  void printHex32(unsigned int Value, raw_ostream &O);
  void emitFrameDirective();
  const char *getCurrentABIString() const;
  virtual void EmitFunctionEntryLabel();
  virtual void EmitFunctionBodyStart();
  virtual void EmitFunctionBodyEnd();
  void EmitStartOfAsmFile(Module &M);
  virtual MachineLocation getDebugValueLocation(const MachineInstr *MI) const;
  void PrintDebugValueComment(const MachineInstr *MI, raw_ostream &OS);
};
}

#endif

