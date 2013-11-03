//===-- CpuSelectionDAGInfo.h - Cpu SelectionDAG Info ---------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file defines the Cpu subclass for TargetSelectionDAGInfo.
//
//===----------------------------------------------------------------------===//

#ifndef CPUSELECTIONDAGINFO_H
#define CPUSELECTIONDAGINFO_H

#include "llvm/Target/TargetSelectionDAGInfo.h"

namespace llvm {

class CpuTargetMachine;

class CpuSelectionDAGInfo : public TargetSelectionDAGInfo {
public:
  explicit CpuSelectionDAGInfo(const CpuTargetMachine &TM);
  ~CpuSelectionDAGInfo();
};

}

#endif
