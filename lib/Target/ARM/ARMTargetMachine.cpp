//===-- ARMTargetMachine.cpp - Define TargetMachine for ARM ---------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
//
//===----------------------------------------------------------------------===//

#include "ARMTargetMachine.h"
#include "ARM.h"
#include "ARMFrameLowering.h"
#include "llvm/CodeGen/Passes.h"
#include "llvm/MC/MCAsmInfo.h"
#include "llvm/PassManager.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/FormattedStream.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/Target/TargetOptions.h"
#include "llvm/Transforms/Scalar.h"
using namespace llvm;

static cl::opt<bool>
EnableGlobalMerge("global-merge", cl::Hidden,
                  cl::desc("Enable global merge pass"),
                  cl::init(true));

static cl::opt<bool>
DisableA15SDOptimization("disable-a15-sd-optimization", cl::Hidden,
                   cl::desc("Inhibit optimization of S->D register accesses on A15"),
                   cl::init(false));

extern "C" void LLVMInitializeARMTarget() {
  // Register the target.
  RegisterTargetMachine<ARMTargetMachine> X(TheARMTarget);
  RegisterTargetMachine<ThumbTargetMachine> Y(TheThumbTarget);
}


/// TargetMachine ctor - Create an ARM architecture model.
///
ARMBaseTargetMachine::ARMBaseTargetMachine(const Target &T, StringRef TT,
                                           StringRef CPU, StringRef FS,
                                           const TargetOptions &Options,
                                           Reloc::Model RM, CodeModel::Model CM,
                                           CodeGenOpt::Level OL)
  : LLVMTargetMachine(T, TT, CPU, FS, Options, RM, CM, OL),
    Subtarget(TT, CPU, FS, Options),
    JITInfo(),
    InstrItins(Subtarget.getInstrItineraryData()) {
  // Default to soft float ABI
  if (Options.FloatABIType == FloatABI::Default)
    this->Options.FloatABIType = FloatABI::Soft;
}

void ARMBaseTargetMachine::addAnalysisPasses(PassManagerBase &PM) {
  // Add first the target-independent BasicTTI pass, then our ARM pass. This
  // allows the ARM pass to delegate to the target independent layer when
  // appropriate.
  PM.add(createBasicTargetTransformInfoPass(this));
  PM.add(createARMTargetTransformInfoPass(this));
}


void ARMTargetMachine::anchor() { }

static std::string computeDataLayout(ARMSubtarget &ST) {
  // Little endian. Pointers are 32 bits and aligned to 32 bits.
  std::string Ret = "e-p:32:32";

  // We have 64 bits floats and integers. The APCS ABI requires them to be
  // aligned s them to 32 bits, others to 64 bits. We always try to align to
  // 64 bits.
  if (ST.isAPCS_ABI())
    Ret += "-f64:32:64-i64:32:64";
  else
    Ret += "-f64:64:64-i64:64:64";

  // On thumb, i16,i18 and i1 have natural aligment requirements, but we try to
  // align to 32.
  if (ST.isThumb())
    Ret += "-i16:16:32-i8:8:32-i1:8:32";

  // We have 128 and 64 bit vectors. The APCS ABI aligns them to 32 bits, others
  // to 64. We always ty to give them natural alignment.
  if (ST.isAPCS_ABI())
    Ret += "-v128:32:128-v64:32:64";
  else
    Ret += "-v128:64:128-v64:64:64";

  // An aggregate of size 0 is ABI aligned to 0.
  // FIXME: explain better what this means.
  if (ST.isThumb())
    Ret += "-a:0:32";

  // Integer registers are 32 bits.
  Ret += "-n32";

  // The stack is 64 bit aligned on AAPCS and 32 bit aligned everywhere else.
  if (ST.isAAPCS_ABI())
    Ret += "-S64";
  else
    Ret += "-S32";

  return Ret;
}

ARMTargetMachine::ARMTargetMachine(const Target &T, StringRef TT,
                                   StringRef CPU, StringRef FS,
                                   const TargetOptions &Options,
                                   Reloc::Model RM, CodeModel::Model CM,
                                   CodeGenOpt::Level OL)
  : ARMBaseTargetMachine(T, TT, CPU, FS, Options, RM, CM, OL),
    InstrInfo(Subtarget),
    DL(computeDataLayout(Subtarget)),
    TLInfo(*this),
    TSInfo(*this),
    FrameLowering(Subtarget) {
  initAsmInfo();
  if (!Subtarget.hasARMOps())
    report_fatal_error("CPU: '" + Subtarget.getCPUString() + "' does not "
                       "support ARM mode execution!");
}

void ThumbTargetMachine::anchor() { }

ThumbTargetMachine::ThumbTargetMachine(const Target &T, StringRef TT,
                                       StringRef CPU, StringRef FS,
                                       const TargetOptions &Options,
                                       Reloc::Model RM, CodeModel::Model CM,
                                       CodeGenOpt::Level OL)
  : ARMBaseTargetMachine(T, TT, CPU, FS, Options, RM, CM, OL),
    InstrInfo(Subtarget.hasThumb2()
              ? ((ARMBaseInstrInfo*)new Thumb2InstrInfo(Subtarget))
              : ((ARMBaseInstrInfo*)new Thumb1InstrInfo(Subtarget))),
    DL(computeDataLayout(Subtarget)),
    TLInfo(*this),
    TSInfo(*this),
    FrameLowering(Subtarget.hasThumb2()
              ? new ARMFrameLowering(Subtarget)
              : (ARMFrameLowering*)new Thumb1FrameLowering(Subtarget)) {
  initAsmInfo();
}

namespace {
/// ARM Code Generator Pass Configuration Options.
class ARMPassConfig : public TargetPassConfig {
public:
  ARMPassConfig(ARMBaseTargetMachine *TM, PassManagerBase &PM)
    : TargetPassConfig(TM, PM) {}

  ARMBaseTargetMachine &getARMTargetMachine() const {
    return getTM<ARMBaseTargetMachine>();
  }

  const ARMSubtarget &getARMSubtarget() const {
    return *getARMTargetMachine().getSubtargetImpl();
  }

  virtual bool addPreISel();
  virtual bool addInstSelector();
  virtual bool addPreRegAlloc();
  virtual bool addPreSched2();
  virtual bool addPreEmitPass();
};
} // namespace

TargetPassConfig *ARMBaseTargetMachine::createPassConfig(PassManagerBase &PM) {
  return new ARMPassConfig(this, PM);
}

bool ARMPassConfig::addPreISel() {
  if (TM->getOptLevel() != CodeGenOpt::None && EnableGlobalMerge)
    addPass(createGlobalMergePass(TM));

  return false;
}

bool ARMPassConfig::addInstSelector() {
  addPass(createARMISelDag(getARMTargetMachine(), getOptLevel()));

  const ARMSubtarget *Subtarget = &getARMSubtarget();
  if (Subtarget->isTargetELF() && !Subtarget->isThumb1Only() &&
      TM->Options.EnableFastISel)
    addPass(createARMGlobalBaseRegPass());
  return false;
}

bool ARMPassConfig::addPreRegAlloc() {
  // FIXME: temporarily disabling load / store optimization pass for Thumb1.
  if (getOptLevel() != CodeGenOpt::None && !getARMSubtarget().isThumb1Only())
    addPass(createARMLoadStoreOptimizationPass(true));
  if (getOptLevel() != CodeGenOpt::None && getARMSubtarget().isCortexA9())
    addPass(createMLxExpansionPass());
  // Since the A15SDOptimizer pass can insert VDUP instructions, it can only be
  // enabled when NEON is available.
  if (getOptLevel() != CodeGenOpt::None && getARMSubtarget().isCortexA15() &&
    getARMSubtarget().hasNEON() && !DisableA15SDOptimization) {
    addPass(createA15SDOptimizerPass());
  }
  return true;
}

bool ARMPassConfig::addPreSched2() {
  // FIXME: temporarily disabling load / store optimization pass for Thumb1.
  if (getOptLevel() != CodeGenOpt::None) {
    if (!getARMSubtarget().isThumb1Only()) {
      addPass(createARMLoadStoreOptimizationPass());
      printAndVerify("After ARM load / store optimizer");
    }
    if (getARMSubtarget().hasNEON())
      addPass(createExecutionDependencyFixPass(&ARM::DPRRegClass));
  }

  // Expand some pseudo instructions into multiple instructions to allow
  // proper scheduling.
  addPass(createARMExpandPseudoPass());

  if (getOptLevel() != CodeGenOpt::None) {
    if (!getARMSubtarget().isThumb1Only()) {
      // in v8, IfConversion depends on Thumb instruction widths
      if (getARMSubtarget().restrictIT() &&
          !getARMSubtarget().prefers32BitThumb())
        addPass(createThumb2SizeReductionPass());
      addPass(&IfConverterID);
    }
  }
  if (getARMSubtarget().isThumb2())
    addPass(createThumb2ITBlockPass());

  return true;
}

bool ARMPassConfig::addPreEmitPass() {
  if (getARMSubtarget().isThumb2()) {
    if (!getARMSubtarget().prefers32BitThumb())
      addPass(createThumb2SizeReductionPass());

    // Constant island pass work on unbundled instructions.
    addPass(&UnpackMachineBundlesID);
  }

  addPass(createARMConstantIslandPass());

  return true;
}

bool ARMBaseTargetMachine::addCodeEmitter(PassManagerBase &PM,
                                          JITCodeEmitter &JCE) {
  // Machine code emitter pass for ARM.
  PM.add(createARMJITCodeEmitterPass(*this, JCE));
  return false;
}
