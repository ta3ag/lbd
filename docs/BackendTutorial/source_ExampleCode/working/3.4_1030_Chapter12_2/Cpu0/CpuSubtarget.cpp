//===-- CpuSubtarget.cpp - Cpu Subtarget Information --------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements the Cpu specific subclass of TargetSubtargetInfo.
//
//===----------------------------------------------------------------------===//

#include "CpuSubtarget.h"
#include "Cpu.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/Support/CommandLine.h"

#define GET_SUBTARGETINFO_TARGET_DESC
#define GET_SUBTARGETINFO_CTOR
#include "CpuGenSubtargetInfo.inc"

using namespace llvm;

static cl::opt<bool>
UseSmallSectionOpt("cpu-use-small-section", cl::Hidden, cl::init(false),
                 cl::desc("Use small section. Only work when -relocation-model="
                 "static. pic always not use small section."));

static cl::opt<bool>
ReserveGPOpt("cpu-reserve-gp", cl::Hidden, cl::init(false),
                 cl::desc("Never allocate $gp to variable"));

static cl::opt<bool>
NoCploadOpt("cpu-no-cpload", cl::Hidden, cl::init(false),
                 cl::desc("No issue .cpload"));

bool CpuReserveGP;
bool CpuNoCpload;

extern bool FixGlobalBaseReg;

void CpuSubtarget::anchor() { }

CpuSubtarget::CpuSubtarget(const std::string &TT, const std::string &CPU,
                             const std::string &FS, bool little, 
                             Reloc::Model _RM) :
  CpuGenSubtargetInfo(TT, CPU, FS),
  CpuABI(UnknownABI), IsLittle(little), RM(_RM)
{
  std::string CPUName = CPU;
  if (CPUName.empty())
    CPUName = "cpu32";

  // Parse features string.
  ParseSubtargetFeatures(CPUName, FS);

  // Initialize scheduling itinerary for the specified CPU.
  InstrItins = getInstrItineraryForCPU(CPUName);

  // Set CpuABI if it hasn't been set yet.
  if (CpuABI == UnknownABI)
    CpuABI = O32;

  // Set UseSmallSection.
  UseSmallSection = UseSmallSectionOpt;
  CpuReserveGP = ReserveGPOpt;
  CpuNoCpload = NoCploadOpt;
  if (RM == Reloc::Static && !UseSmallSection && !CpuReserveGP)
    FixGlobalBaseReg = false;
  else
    FixGlobalBaseReg = true;
}

