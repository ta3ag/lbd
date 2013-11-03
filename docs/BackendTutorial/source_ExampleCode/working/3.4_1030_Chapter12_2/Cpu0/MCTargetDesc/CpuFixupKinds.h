//===-- CpuFixupKinds.h - Cpu Specific Fixup Entries ----------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CPU_CPUFIXUPKINDS_H
#define LLVM_CPU_CPUFIXUPKINDS_H

#include "Cpu.h"
#include "llvm/MC/MCFixup.h"

namespace llvm {
namespace Cpu {
  // Although most of the current fixup types reflect a unique relocation
  // one can have multiple fixup types for a given relocation and thus need
  // to be uniquely named.
  //
  // This table *must* be in the save order of
  // MCFixupKindInfo Infos[Cpu::NumTargetFixupKinds]
  // in CpuAsmBackend.cpp.
  //
  enum Fixups {
    // Jump 24 bit fixup resulting in - R_CPU0_24.
    fixup_Cpu_24 = FirstTargetFixupKind,

    // Pure upper 32 bit fixup resulting in - R_CPU0_32.
    fixup_Cpu_32,

    // Pure upper 16 bit fixup resulting in - R_CPU0_HI16.
    fixup_Cpu_HI16,

    // Pure lower 16 bit fixup resulting in - R_CPU0_LO16.
    fixup_Cpu_LO16,

    // Pure lower 16 bit fixup resulting in - R_CPU0_GPREL16.
    fixup_Cpu_GPREL16,

    // Global symbol fixup resulting in - R_CPU0_GOT16.
    fixup_Cpu_GOT_Global,

    // Local symbol fixup resulting in - R_CPU0_GOT16.
    fixup_Cpu_GOT_Local,

#ifdef CPU_REDESIGN_INSTRUCTION
    // PC relative branch fixup resulting in - R_CPU0_PC16.
    // cpu PC16, e.g. beq
    fixup_Cpu_PC16,
    
    // PC relative branch fixup resulting in - R_CPU0_PC24.
    // cpu PC24, e.g. jmp
    fixup_Cpu_PC24,
#else
    // PC relative branch fixup resulting in - R_CPU0_PC24.
    // cpu PC24, e.g. jeq
    fixup_Cpu_PC24,
#endif // CPU_REDESIGN_INSTRUCTION

    // resulting in - R_CPU0_CALL16.
    fixup_Cpu_CALL16,

    // PC relative branch fixup resulting in - R_CPU0_PC24
    fixup_Cpu_Branch_PCRel,

    // resulting in - R_CPU0_GOT_HI16
    fixup_Cpu_GOT_HI16,

    // resulting in - R_CPU0_GOT_LO16
    fixup_Cpu_GOT_LO16,

    // Marker
    LastTargetFixupKind,
    NumTargetFixupKinds = LastTargetFixupKind - FirstTargetFixupKind
  };
} // namespace Cpu
} // namespace llvm


#endif // LLVM_CPU_CPUFIXUPKINDS_H
