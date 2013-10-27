//===-- Cpu0FixupKinds.h - Cpu0 Specific Fixup Entries ----------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CPU0_CPU0FIXUPKINDS_H
#define LLVM_CPU0_CPU0FIXUPKINDS_H

#include "llvm/MC/MCFixup.h"

namespace llvm {
namespace Cpu0 {
  // Although most of the current fixup types reflect a unique relocation
  // one can have multiple fixup types for a given relocation and thus need
  // to be uniquely named.
  //
  // This table *must* be in the save order of
  // MCFixupKindInfo Infos[Cpu0::NumTargetFixupKinds]
  // in Cpu0AsmBackend.cpp.
  //
  enum Fixups {
    // Jump 24 bit fixup resulting in - R_CPU0_24.
    fixup_Cpu0_24 = FirstTargetFixupKind,

    // Pure upper 32 bit fixup resulting in - R_CPU0_32.
    fixup_Cpu0_32,

    // Pure upper 16 bit fixup resulting in - R_CPU0_HI16.
    fixup_Cpu0_HI16,

    // Pure lower 16 bit fixup resulting in - R_CPU0_LO16.
    fixup_Cpu0_LO16,

    // Pure lower 16 bit fixup resulting in - R_CPU0_GPREL16.
    fixup_Cpu0_GPREL16,

    // Global symbol fixup resulting in - R_CPU0_GOT16.
    fixup_Cpu0_GOT_Global,

    // Local symbol fixup resulting in - R_CPU0_GOT16.
    fixup_Cpu0_GOT_Local,

    // PC relative branch fixup resulting in - R_CPU0_PC24.
    // cpu0 PC24, e.g. jeq
    fixup_Cpu0_PC24,

    // resulting in - R_CPU0_CALL16.
    fixup_Cpu0_CALL16,

    // PC relative branch fixup resulting in - R_CPU0_PC24
    fixup_Cpu0_Branch_PCRel,

    // resulting in - R_MIPS_GOT_HI16
    fixup_Cpu0_GOT_HI16,

    // resulting in - R_MIPS_GOT_LO16
    fixup_Cpu0_GOT_LO16,

    // Marker
    LastTargetFixupKind,
    NumTargetFixupKinds = LastTargetFixupKind - FirstTargetFixupKind
  };
} // namespace Cpu0
} // namespace llvm


#endif // LLVM_CPU0_CPU0FIXUPKINDS_H
