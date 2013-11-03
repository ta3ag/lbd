//===-- CpuMCTargetDesc.cpp - Cpu Target Descriptions -------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file provides Cpu specific target descriptions.
//
//===----------------------------------------------------------------------===//

#include "CpuMCTargetDesc.h"
#include "InstPrinter/CpuInstPrinter.h"
#include "CpuMCAsmInfo.h"
#include "llvm/MC/MCStreamer.h"
#include "llvm/MC/MCCodeGenInfo.h"
#include "llvm/MC/MCELF.h"
#include "llvm/MC/MCELFStreamer.h"
#include "llvm/MC/MCInstrInfo.h"
#include "llvm/MC/MCRegisterInfo.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/MC/MCSymbol.h"
#include "llvm/MC/MachineLocation.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/FormattedStream.h"
#include "llvm/Support/TargetRegistry.h"

#define GET_INSTRINFO_MC_DESC
#include "CpuGenInstrInfo.inc"

#define GET_SUBTARGETINFO_MC_DESC
#include "CpuGenSubtargetInfo.inc"

#define GET_REGINFO_MC_DESC
#include "CpuGenRegisterInfo.inc"

using namespace llvm;

static std::string ParseCpuTriple(StringRef TT, StringRef CPU) {
  std::string CpuArchFeature;
  size_t DashPosition = 0;
  StringRef TheTriple;

  // Let's see if there is a dash, like cpu-unknown-linux.
  DashPosition = TT.find('-');

  if (DashPosition == StringRef::npos) {
    // No dash, we check the string size.
    TheTriple = TT.substr(0);
  } else {
    // We are only interested in substring before dash.
    TheTriple = TT.substr(0,DashPosition);
  }

  if (TheTriple == "cpu" || TheTriple == "cpuel") {
    if (CPU.empty() || CPU == "cpu32") {
      CpuArchFeature = "+cpu32";
    }
  }
  return CpuArchFeature;
}

static MCInstrInfo *createCpuMCInstrInfo() {
  MCInstrInfo *X = new MCInstrInfo();
  InitCpuMCInstrInfo(X); // defined in CpuGenInstrInfo.inc
  return X;
}

static MCRegisterInfo *createCpuMCRegisterInfo(StringRef TT) {
  MCRegisterInfo *X = new MCRegisterInfo();
  InitCpuMCRegisterInfo(X, Cpu::LR); // defined in CpuGenRegisterInfo.inc
  return X;
}

static MCSubtargetInfo *createCpuMCSubtargetInfo(StringRef TT, StringRef CPU,
                                                  StringRef FS) {
  std::string ArchFS = ParseCpuTriple(TT,CPU);
  if (!FS.empty()) {
    if (!ArchFS.empty())
      ArchFS = ArchFS + "," + FS.str();
    else
      ArchFS = FS;
  }
  MCSubtargetInfo *X = new MCSubtargetInfo();
  InitCpuMCSubtargetInfo(X, TT, CPU, ArchFS); // defined in CpuGenSubtargetInfo.inc
  return X;
}

static MCAsmInfo *createCpuMCAsmInfo(const MCRegisterInfo &MRI, StringRef TT) {
  MCAsmInfo *MAI = new CpuMCAsmInfo(TT);

  unsigned SP = MRI.getDwarfRegNum(Cpu::SP, true);
  MCCFIInstruction Inst = MCCFIInstruction::createDefCfa(0, SP, 0);
  MAI->addInitialFrameState(Inst);

  return MAI;
}

static MCCodeGenInfo *createCpuMCCodeGenInfo(StringRef TT, Reloc::Model RM,
                                              CodeModel::Model CM,
                                              CodeGenOpt::Level OL) {
  MCCodeGenInfo *X = new MCCodeGenInfo();
  if (CM == CodeModel::JITDefault)
    RM = Reloc::Static;
  else if (RM == Reloc::Default)
    RM = Reloc::PIC_;
  X->InitMCCodeGenInfo(RM, CM, OL); // defined in lib/MC/MCCodeGenInfo.cpp
  return X;
}

static MCInstPrinter *createCpuMCInstPrinter(const Target &T,
                                              unsigned SyntaxVariant,
                                              const MCAsmInfo &MAI,
                                              const MCInstrInfo &MII,
                                              const MCRegisterInfo &MRI,
                                              const MCSubtargetInfo &STI) {
  return new CpuInstPrinter(MAI, MII, MRI);
}

static MCStreamer *createMCStreamer(const Target &T, StringRef TT,
                                    MCContext &Context, MCAsmBackend &MAB,
                                    raw_ostream &OS, MCCodeEmitter *Emitter,
                                    bool RelaxAll, bool NoExecStack) {
  MCTargetStreamer *S = new MCTargetStreamer();
  return createELFStreamer(Context, S, MAB, OS, Emitter, RelaxAll, NoExecStack);
}

static MCStreamer *
createMCAsmStreamer(MCContext &Ctx, formatted_raw_ostream &OS,
                    bool isVerboseAsm, bool useLoc, bool useCFI,
                    bool useDwarfDirectory, MCInstPrinter *InstPrint,
                    MCCodeEmitter *CE, MCAsmBackend *TAB, bool ShowInst) {
  MCTargetStreamer *S = new MCTargetStreamer();

  return llvm::createAsmStreamer(Ctx, S, OS, isVerboseAsm, useLoc, useCFI,
                                 useDwarfDirectory, InstPrint, CE, TAB,
                                 ShowInst);
}

extern "C" void LLVMInitializeCpuTargetMC() {
  // Register the MC asm info.
  RegisterMCAsmInfoFn X(TheCpuTarget, createCpuMCAsmInfo);
  RegisterMCAsmInfoFn Y(TheCpuelTarget, createCpuMCAsmInfo);

  // Register the MC codegen info.
  TargetRegistry::RegisterMCCodeGenInfo(TheCpuTarget,
                                        createCpuMCCodeGenInfo);
  TargetRegistry::RegisterMCCodeGenInfo(TheCpuelTarget,
                                        createCpuMCCodeGenInfo);
  // Register the MC instruction info.
  TargetRegistry::RegisterMCInstrInfo(TheCpuTarget, createCpuMCInstrInfo);
  TargetRegistry::RegisterMCInstrInfo(TheCpuelTarget, createCpuMCInstrInfo);

  // Register the MC register info.
  TargetRegistry::RegisterMCRegInfo(TheCpuTarget, createCpuMCRegisterInfo);
  TargetRegistry::RegisterMCRegInfo(TheCpuelTarget, createCpuMCRegisterInfo);

  // Register the MC Code Emitter
  TargetRegistry::RegisterMCCodeEmitter(TheCpuTarget,
                                        createCpuMCCodeEmitterEB);
  TargetRegistry::RegisterMCCodeEmitter(TheCpuelTarget,
                                        createCpuMCCodeEmitterEL);

  // Register the object streamer.
  TargetRegistry::RegisterMCObjectStreamer(TheCpuTarget, createMCStreamer);
  TargetRegistry::RegisterMCObjectStreamer(TheCpuelTarget, createMCStreamer);

  // Register the asm streamer.
  TargetRegistry::RegisterAsmStreamer(TheCpuTarget, createMCAsmStreamer);
  TargetRegistry::RegisterAsmStreamer(TheCpuelTarget, createMCAsmStreamer);

  // Register the asm backend.
  TargetRegistry::RegisterMCAsmBackend(TheCpuTarget,
                                       createCpuAsmBackendEB32);
  TargetRegistry::RegisterMCAsmBackend(TheCpuelTarget,
                                       createCpuAsmBackendEL32);
  // Register the MC subtarget info.
  TargetRegistry::RegisterMCSubtargetInfo(TheCpuTarget,
                                          createCpuMCSubtargetInfo);
  TargetRegistry::RegisterMCSubtargetInfo(TheCpuelTarget,
                                          createCpuMCSubtargetInfo);
  // Register the MCInstPrinter.
  TargetRegistry::RegisterMCInstPrinter(TheCpuTarget,
                                        createCpuMCInstPrinter);
  TargetRegistry::RegisterMCInstPrinter(TheCpuelTarget,
                                        createCpuMCInstPrinter);
}
