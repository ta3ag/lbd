//===-- CpuMCTargetDesc.h - Cpu Target Descriptions -----------*- C++ -*-===//
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

#ifndef CPUMCTARGETDESC_H
#define CPUMCTARGETDESC_H

#include "llvm/Support/DataTypes.h"

namespace llvm {
class MCAsmBackend;
class MCCodeEmitter;
class MCContext;
class MCInstrInfo;
class MCObjectWriter;
class MCRegisterInfo;
class MCSubtargetInfo;
class StringRef;
class Target;
class raw_ostream;

extern Target TheCpuTarget;
extern Target TheCpuelTarget;

MCCodeEmitter *createCpuMCCodeEmitterEB(const MCInstrInfo &MCII,
                                         const MCRegisterInfo &MRI,
                                         const MCSubtargetInfo &STI,
                                         MCContext &Ctx);
MCCodeEmitter *createCpuMCCodeEmitterEL(const MCInstrInfo &MCII,
                                         const MCRegisterInfo &MRI,
                                         const MCSubtargetInfo &STI,
                                         MCContext &Ctx);

MCAsmBackend *createCpuAsmBackendEB32(const Target &T, const MCRegisterInfo &MRI,
                                       StringRef TT, StringRef CPU);
MCAsmBackend *createCpuAsmBackendEL32(const Target &T, const MCRegisterInfo &MRI,
                                       StringRef TT, StringRef CPU);

MCObjectWriter *createCpuELFObjectWriter(raw_ostream &OS,
                                          uint8_t OSABI,
                                          bool IsLittleEndian);
} // End llvm namespace

// Defines symbolic names for Cpu registers.  This defines a mapping from
// register name to register number.
#define GET_REGINFO_ENUM
#include "CpuGenRegisterInfo.inc"

// Defines symbolic names for the Cpu instructions.
#define GET_INSTRINFO_ENUM
#include "CpuGenInstrInfo.inc"

#define GET_SUBTARGETINFO_ENUM
#include "CpuGenSubtargetInfo.inc"
#endif


