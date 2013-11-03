//===-- CpuTargetMachine.h - Define TargetMachine for Cpu -----*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file declares the Cpu specific subclass of TargetMachine.
//
//===----------------------------------------------------------------------===//

#ifndef CPUTARGETMACHINE_H
#define CPUTARGETMACHINE_H

#include "CpuFrameLowering.h"
#include "CpuInstrInfo.h"
#include "CpuISelLowering.h"
#include "CpuSelectionDAGInfo.h"
#include "CpuSubtarget.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/Target/TargetFrameLowering.h"

namespace llvm {
  class formatted_raw_ostream;

  class CpuTargetMachine : public LLVMTargetMachine {
    CpuSubtarget       Subtarget;
    const DataLayout    DL; // Calculates type size & alignment
    CpuInstrInfo       InstrInfo;	//- Instructions
    CpuFrameLowering   FrameLowering;	//- Stack(Frame) and Stack direction
    CpuTargetLowering  TLInfo;	//- Stack(Frame) and Stack direction
    CpuSelectionDAGInfo TSInfo;	//- Map .bc DAG to backend DAG

  public:
    CpuTargetMachine(const Target &T, StringRef TT,
                      StringRef CPU, StringRef FS, const TargetOptions &Options,
                      Reloc::Model RM, CodeModel::Model CM,
                      CodeGenOpt::Level OL,
                      bool isLittle);

    virtual const CpuInstrInfo   *getInstrInfo()     const
    { return &InstrInfo; }
    virtual const TargetFrameLowering *getFrameLowering()     const
    { return &FrameLowering; }
    virtual const CpuSubtarget   *getSubtargetImpl() const
    { return &Subtarget; }
    virtual const DataLayout *getDataLayout()    const
    { return &DL;}

    virtual const CpuRegisterInfo *getRegisterInfo()  const {
      return &InstrInfo.getRegisterInfo();
    }

    virtual const CpuTargetLowering *getTargetLowering() const {
      return &TLInfo;
    }

    virtual const CpuSelectionDAGInfo* getSelectionDAGInfo() const {
      return &TSInfo;
    }

    // Pass Pipeline Configuration
    virtual TargetPassConfig *createPassConfig(PassManagerBase &PM);
  };

/// CpuebTargetMachine - Cpu32 big endian target machine.
///
class CpuebTargetMachine : public CpuTargetMachine {
  virtual void anchor();
public:
  CpuebTargetMachine(const Target &T, StringRef TT,
                      StringRef CPU, StringRef FS, const TargetOptions &Options,
                      Reloc::Model RM, CodeModel::Model CM,
                      CodeGenOpt::Level OL);
};

/// CpuelTargetMachine - Cpu32 little endian target machine.
///
class CpuelTargetMachine : public CpuTargetMachine {
  virtual void anchor();
public:
  CpuelTargetMachine(const Target &T, StringRef TT,
                      StringRef CPU, StringRef FS, const TargetOptions &Options,
                      Reloc::Model RM, CodeModel::Model CM,
                      CodeGenOpt::Level OL);
};
} // End llvm namespace

#endif
