//===-- CpuSelectionDAGInfo.cpp - Cpu SelectionDAG Info -----------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements the CpuSelectionDAGInfo class.
//
//===----------------------------------------------------------------------===//

#define DEBUG_TYPE "cpu-selectiondag-info"
#include "CpuTargetMachine.h"
using namespace llvm;

CpuSelectionDAGInfo::CpuSelectionDAGInfo(const CpuTargetMachine &TM)
  : TargetSelectionDAGInfo(TM) {
}

CpuSelectionDAGInfo::~CpuSelectionDAGInfo() {
}
