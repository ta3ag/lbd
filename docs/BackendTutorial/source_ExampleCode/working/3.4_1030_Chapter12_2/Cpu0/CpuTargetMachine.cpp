//===-- CpuTargetMachine.cpp - Define TargetMachine for Cpu -------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// Implements the info about Cpu target spec.
//
//===----------------------------------------------------------------------===//

#include "CpuTargetMachine.h"
#include "Cpu.h"
#include "llvm/PassManager.h"
#include "llvm/CodeGen/Passes.h"
#include "llvm/Support/TargetRegistry.h"
using namespace llvm;

extern "C" void LLVMInitializeCpuTarget() {
  // Register the target.
  //- Big endian Target Machine
  RegisterTargetMachine<CpuebTargetMachine> X(TheCpuTarget);
  //- Little endian Target Machine
  RegisterTargetMachine<CpuelTargetMachine> Y(TheCpuelTarget);
}

// DataLayout --> Big-endian, 32-bit pointer/ABI/alignment
// The stack is always 8 byte aligned
// On function prologue, the stack is created by decrementing
// its pointer. Once decremented, all references are done with positive
// offset from the stack/frame pointer, using StackGrowsUp enables
// an easier handling.
// Using CodeModel::Large enables different CALL behavior.
CpuTargetMachine::
CpuTargetMachine(const Target &T, StringRef TT,
                  StringRef CPU, StringRef FS, const TargetOptions &Options,
                  Reloc::Model RM, CodeModel::Model CM,
                  CodeGenOpt::Level OL,
                  bool isLittle)
  //- Default is big endian
  : LLVMTargetMachine(T, TT, CPU, FS, Options, RM, CM, OL),
    Subtarget(TT, CPU, FS, isLittle, RM),
    DL(isLittle ?
               ("e-p:32:32:32-i8:8:32-i16:16:32-i64:64:64-n32") :
               ("E-p:32:32:32-i8:8:32-i16:16:32-i64:64:64-n32")),
    InstrInfo(*this),
    FrameLowering(Subtarget), 
    TLInfo(*this), TSInfo(*this) {
  initAsmInfo();
}

void CpuebTargetMachine::anchor() { }

CpuebTargetMachine::
CpuebTargetMachine(const Target &T, StringRef TT,
                    StringRef CPU, StringRef FS, const TargetOptions &Options,
                    Reloc::Model RM, CodeModel::Model CM,
                    CodeGenOpt::Level OL)
  : CpuTargetMachine(T, TT, CPU, FS, Options, RM, CM, OL, false) {}

void CpuelTargetMachine::anchor() { }

CpuelTargetMachine::
CpuelTargetMachine(const Target &T, StringRef TT,
                    StringRef CPU, StringRef FS, const TargetOptions &Options,
                    Reloc::Model RM, CodeModel::Model CM,
                    CodeGenOpt::Level OL)
  : CpuTargetMachine(T, TT, CPU, FS, Options, RM, CM, OL, true) {}
namespace {
/// Cpu Code Generator Pass Configuration Options.
class CpuPassConfig : public TargetPassConfig {
public:
  CpuPassConfig(CpuTargetMachine *TM, PassManagerBase &PM)
    : TargetPassConfig(TM, PM) {}

  CpuTargetMachine &getCpuTargetMachine() const {
    return getTM<CpuTargetMachine>();
  }

  const CpuSubtarget &getCpuSubtarget() const {
    return *getCpuTargetMachine().getSubtargetImpl();
  }
  virtual bool addInstSelector();
  virtual bool addPreRegAlloc();
  virtual bool addPreEmitPass();
};
} // namespace

TargetPassConfig *CpuTargetMachine::createPassConfig(PassManagerBase &PM) {
  return new CpuPassConfig(this, PM);
}

// Install an instruction selector pass using
// the ISelDag to gen Cpu code.
bool CpuPassConfig::addInstSelector() {
  addPass(createCpuISelDag(getCpuTargetMachine()));
  return false;
}

bool CpuPassConfig::addPreRegAlloc() {
  // $gp is a caller-saved register.

  addPass(createCpuEmitGPRestorePass(getCpuTargetMachine()));
  return true;
}

// Implemented by targets that want to run passes immediately before
// machine code is emitted. return true if -print-machineinstrs should
// print out the code after the passes.
bool CpuPassConfig::addPreEmitPass() {
  CpuTargetMachine &TM = getCpuTargetMachine();
  addPass(createCpuDelJmpPass(TM));
  return true;
}
