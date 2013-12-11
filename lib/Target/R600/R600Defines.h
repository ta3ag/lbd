//===-- R600Defines.h - R600 Helper Macros ----------------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
/// \file
//===----------------------------------------------------------------------===//

#ifndef R600DEFINES_H_
#define R600DEFINES_H_

#include "llvm/MC/MCRegisterInfo.h"

// Operand Flags
#define MO_FLAG_CLAMP (1 << 0)
#define MO_FLAG_NEG   (1 << 1)
#define MO_FLAG_ABS   (1 << 2)
#define MO_FLAG_MASK  (1 << 3)
#define MO_FLAG_PUSH  (1 << 4)
#define MO_FLAG_NOT_LAST  (1 << 5)
#define MO_FLAG_LAST  (1 << 6)
#define NUM_MO_FLAGS 7

/// \brief Helper for getting the operand index for the instruction flags
/// operand.
#define GET_FLAG_OPERAND_IDX(Flags) (((Flags) >> 7) & 0x3)

namespace R600_InstFlag {
  enum TIF {
    TRANS_ONLY = (1 << 0),
    TEX = (1 << 1),
    REDUCTION = (1 << 2),
    FC = (1 << 3),
    TRIG = (1 << 4),
    OP3 = (1 << 5),
    VECTOR = (1 << 6),
    //FlagOperand bits 7, 8
    NATIVE_OPERANDS = (1 << 9),
    OP1 = (1 << 10),
    OP2 = (1 << 11),
    VTX_INST  = (1 << 12),
    TEX_INST = (1 << 13),
    ALU_INST = (1 << 14),
    LDS_1A = (1 << 15),
    LDS_1A1D = (1 << 16),
    IS_EXPORT = (1 << 17),
    LDS_1A2D = (1 << 18)
  };
}

#define HAS_NATIVE_OPERANDS(Flags) ((Flags) & R600_InstFlag::NATIVE_OPERANDS)

/// \brief Defines for extracting register infomation from register encoding
#define HW_REG_MASK 0x1ff
#define HW_CHAN_SHIFT 9

#define GET_REG_CHAN(reg) ((reg) >> HW_CHAN_SHIFT)
#define GET_REG_INDEX(reg) ((reg) & HW_REG_MASK)

#define IS_VTX(desc) ((desc).TSFlags & R600_InstFlag::VTX_INST)
#define IS_TEX(desc) ((desc).TSFlags & R600_InstFlag::TEX_INST)

namespace OpName {

}

//===----------------------------------------------------------------------===//
// Config register definitions
//===----------------------------------------------------------------------===//

#define R_02880C_DB_SHADER_CONTROL                    0x02880C
#define   S_02880C_KILL_ENABLE(x)                      (((x) & 0x1) << 6)

// These fields are the same for all shader types and families.
#define   S_NUM_GPRS(x)                         (((x) & 0xFF) << 0)
#define   S_STACK_SIZE(x)                       (((x) & 0xFF) << 8)
//===----------------------------------------------------------------------===//
// R600, R700 Registers
//===----------------------------------------------------------------------===//

#define R_028850_SQ_PGM_RESOURCES_PS                 0x028850
#define R_028868_SQ_PGM_RESOURCES_VS                 0x028868

//===----------------------------------------------------------------------===//
// Evergreen, Northern Islands Registers
//===----------------------------------------------------------------------===//

#define R_028844_SQ_PGM_RESOURCES_PS                 0x028844
#define R_028860_SQ_PGM_RESOURCES_VS                 0x028860
#define R_028878_SQ_PGM_RESOURCES_GS                 0x028878
#define R_0288D4_SQ_PGM_RESOURCES_LS                 0x0288d4

#define R_0288E8_SQ_LDS_ALLOC                        0x0288E8

#endif // R600DEFINES_H_
