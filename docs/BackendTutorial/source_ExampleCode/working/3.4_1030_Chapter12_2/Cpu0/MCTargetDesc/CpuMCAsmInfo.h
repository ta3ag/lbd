//===-- CpuMCAsmInfo.h - Cpu Asm Info ------------------------*- C++ -*--===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains the declaration of the CpuMCAsmInfo class.
//
//===----------------------------------------------------------------------===//

#ifndef CPUTARGETASMINFO_H
#define CPUTARGETASMINFO_H

#include "llvm/MC/MCAsmInfo.h"

namespace llvm {
  class StringRef;
  class Target;

  class CpuMCAsmInfo : public MCAsmInfo {
    virtual void anchor();
  public:
    explicit CpuMCAsmInfo(StringRef TT);
  };

} // namespace llvm

#endif
