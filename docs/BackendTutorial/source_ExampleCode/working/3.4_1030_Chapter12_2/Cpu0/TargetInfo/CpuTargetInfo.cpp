//===-- CpuTargetInfo.cpp - Cpu Target Implementation -------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "Cpu.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/TargetRegistry.h"
using namespace llvm;

Target llvm::TheCpuTarget, llvm::TheCpuelTarget;

extern "C" void LLVMInitializeCpuTargetInfo() {
  RegisterTarget<Triple::cpu,
        /*HasJIT=*/true> X(TheCpuTarget, "cpu", "Cpu");

  RegisterTarget<Triple::cpuel,
        /*HasJIT=*/true> Y(TheCpuelTarget, "cpuel", "Cpuel");
}
