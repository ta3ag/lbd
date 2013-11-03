//===-- CpuMCCodeEmitter.cpp - Convert Cpu Code to Machine Code ---------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements the CpuMCCodeEmitter class.
//
//===----------------------------------------------------------------------===//
//
#define DEBUG_TYPE "mccodeemitter"
#include "Cpu.h"
#include "MCTargetDesc/CpuBaseInfo.h"
#include "MCTargetDesc/CpuFixupKinds.h"
#include "MCTargetDesc/CpuMCTargetDesc.h"
#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/MC/MCCodeEmitter.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCInst.h"
#include "llvm/MC/MCInstrInfo.h"
#include "llvm/MC/MCRegisterInfo.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

namespace {
class CpuMCCodeEmitter : public MCCodeEmitter {
  // #define LLVM_DELETED_FUNCTION
  //  LLVM_DELETED_FUNCTION - Expands to = delete if the compiler supports it. 
  //  Use to mark functions as uncallable. Member functions with this should be 
  //  declared private so that some behavior is kept in C++03 mode.
  //  class DontCopy { private: DontCopy(const DontCopy&) LLVM_DELETED_FUNCTION;
  //  DontCopy &operator =(const DontCopy&) LLVM_DELETED_FUNCTION; public: ... };
  //  Definition at line 79 of file Compiler.h.

  CpuMCCodeEmitter(const CpuMCCodeEmitter &) LLVM_DELETED_FUNCTION;
  void operator=(const CpuMCCodeEmitter &) LLVM_DELETED_FUNCTION;
  // Even though, the old function still work on LLVM version 3.2
  //  CpuMCCodeEmitter(const CpuMCCodeEmitter &); // DO NOT IMPLEMENT
  //  void operator=(const CpuMCCodeEmitter &); // DO NOT IMPLEMENT

  const MCInstrInfo &MCII;
  const MCSubtargetInfo &STI;
  MCContext &Ctx;
  bool IsLittleEndian;

public:
  CpuMCCodeEmitter(const MCInstrInfo &mcii, const MCSubtargetInfo &sti,
                    MCContext &ctx, bool IsLittle) :
            MCII(mcii), STI(sti) , Ctx(ctx), IsLittleEndian(IsLittle) {}

  ~CpuMCCodeEmitter() {}

  void EmitByte(unsigned char C, raw_ostream &OS) const {
    OS << (char)C;
  }

  void EmitInstruction(uint64_t Val, unsigned Size, raw_ostream &OS) const {
    // Output the instruction encoding in little endian byte order.
    for (unsigned i = 0; i < Size; ++i) {
      unsigned Shift = IsLittleEndian ? i * 8 : (Size - 1 - i) * 8;
      EmitByte((Val >> Shift) & 0xff, OS);
    }
  }

  void EncodeInstruction(const MCInst &MI, raw_ostream &OS,
                         SmallVectorImpl<MCFixup> &Fixups) const;

  // getBinaryCodeForInstr - TableGen'erated function for getting the
  // binary encoding for an instruction.
  uint64_t getBinaryCodeForInstr(const MCInst &MI,
                                 SmallVectorImpl<MCFixup> &Fixups) const;

  // getBranchTargetOpValue - Return binary encoding of the branch
  // target operand, such as JMP #BB01, JEQ, JSUB. If the machine operand
  // requires relocation, record the relocation and return zero.
  unsigned getBranchTargetOpValue(const MCInst &MI, unsigned OpNo,
                                  SmallVectorImpl<MCFixup> &Fixups) const;

  // getJumpTargetOpValue - Return binary encoding of the jump
  // target operand, such as SWI #interrupt_addr and JSUB #function_addr. 
  // If the machine operand requires relocation,
  // record the relocation and return zero.
   unsigned getJumpTargetOpValue(const MCInst &MI, unsigned OpNo,
                                 SmallVectorImpl<MCFixup> &Fixups) const;

   // getMachineOpValue - Return binary encoding of operand. If the machin
   // operand requires relocation, record the relocation and return zero.
  unsigned getMachineOpValue(const MCInst &MI,const MCOperand &MO,
                             SmallVectorImpl<MCFixup> &Fixups) const;

  unsigned getMemEncoding(const MCInst &MI, unsigned OpNo,
                          SmallVectorImpl<MCFixup> &Fixups) const;
}; // class CpuMCCodeEmitter
}  // namespace

MCCodeEmitter *llvm::createCpuMCCodeEmitterEB(const MCInstrInfo &MCII,
                                               const MCRegisterInfo &MRI,
                                               const MCSubtargetInfo &STI,
                                               MCContext &Ctx)
{
  return new CpuMCCodeEmitter(MCII, STI, Ctx, false);
}

MCCodeEmitter *llvm::createCpuMCCodeEmitterEL(const MCInstrInfo &MCII,
                                               const MCRegisterInfo &MRI,
                                               const MCSubtargetInfo &STI,
                                               MCContext &Ctx)
{
  return new CpuMCCodeEmitter(MCII, STI, Ctx, true);
}

/// EncodeInstruction - Emit the instruction.
/// Size the instruction (currently only 4 bytes
void CpuMCCodeEmitter::
EncodeInstruction(const MCInst &MI, raw_ostream &OS,
                  SmallVectorImpl<MCFixup> &Fixups) const
{
  uint32_t Binary = getBinaryCodeForInstr(MI, Fixups);

  // Check for unimplemented opcodes.
  // Unfortunately in CPU both NOT and SLL will come in with Binary == 0
  // so we have to special check for them.
  unsigned Opcode = MI.getOpcode();
  if ((Opcode != Cpu::NOP) && (Opcode != Cpu::SHL) && !Binary)
    llvm_unreachable("unimplemented opcode in EncodeInstruction()");

  const MCInstrDesc &Desc = MCII.get(MI.getOpcode());
  uint64_t TSFlags = Desc.TSFlags;

  // Pseudo instructions don't get encoded and shouldn't be here
  // in the first place!
  if ((TSFlags & CpuII::FormMask) == CpuII::Pseudo)
    llvm_unreachable("Pseudo opcode found in EncodeInstruction()");

  // For now all instructions are 4 bytes
  int Size = 4; // FIXME: Have Desc.getSize() return the correct value!

  EmitInstruction(Binary, Size, OS);
}

/// getBranchTargetOpValue - Return binary encoding of the branch
/// target operand. If the machine operand requires relocation,
/// record the relocation and return zero.
unsigned CpuMCCodeEmitter::
getBranchTargetOpValue(const MCInst &MI, unsigned OpNo,
                       SmallVectorImpl<MCFixup> &Fixups) const {

  const MCOperand &MO = MI.getOperand(OpNo);

  // If the destination is an immediate, we have nothing to do.
  if (MO.isImm()) return MO.getImm();
  assert(MO.isExpr() && "getBranchTargetOpValue expects only expressions");

  const MCExpr *Expr = MO.getExpr();
  Fixups.push_back(MCFixup::Create(0, Expr,
#ifdef CPU_REDESIGN_INSTRUCTION
                                   MCFixupKind(Cpu::fixup_Cpu_PC16)));
#else
                                   MCFixupKind(Cpu::fixup_Cpu_PC24)));
#endif // CPU_REDESIGN_INSTRUCTION
  return 0;
}

/// getJumpTargetOpValue - Return binary encoding of the jump
/// target operand. Such as SWI and JSUB. 
/// If the machine operand requires relocation,
/// record the relocation and return zero.
unsigned CpuMCCodeEmitter::
getJumpTargetOpValue(const MCInst &MI, unsigned OpNo,
                     SmallVectorImpl<MCFixup> &Fixups) const {

  unsigned Opcode = MI.getOpcode();
  const MCOperand &MO = MI.getOperand(OpNo);
  // If the destination is an immediate, we have nothing to do.
  if (MO.isImm()) return MO.getImm();
  assert(MO.isExpr() && "getJumpTargetOpValue expects only expressions");

  const MCExpr *Expr = MO.getExpr();
#ifdef CPU_REDESIGN_INSTRUCTION
  if (Opcode == Cpu::JSUB || Opcode == Cpu::JMP)
#else
  if (Opcode == Cpu::JSUB)
#endif // CPU_REDESIGN_INSTRUCTION
    Fixups.push_back(MCFixup::Create(0, Expr,
                                     MCFixupKind(Cpu::fixup_Cpu_PC24)));
  else if (Opcode == Cpu::SWI)
    Fixups.push_back(MCFixup::Create(0, Expr,
                                     MCFixupKind(Cpu::fixup_Cpu_24)));
  else
    llvm_unreachable("unexpect opcode in getJumpAbsoluteTargetOpValue()");
  
  return 0;
}

/// getMachineOpValue - Return binary encoding of operand. If the machine
/// operand requires relocation, record the relocation and return zero.
unsigned CpuMCCodeEmitter::
getMachineOpValue(const MCInst &MI, const MCOperand &MO,
                  SmallVectorImpl<MCFixup> &Fixups) const {
  if (MO.isReg()) {
    unsigned Reg = MO.getReg();
    unsigned RegNo = getCpuRegisterNumbering(Reg);
    return RegNo;
  } else if (MO.isImm()) {
    return static_cast<unsigned>(MO.getImm());
  } else if (MO.isFPImm()) {
    return static_cast<unsigned>(APFloat(MO.getFPImm())
        .bitcastToAPInt().getHiBits(32).getLimitedValue());
  } 

  // MO must be an Expr.
  assert(MO.isExpr());

  const MCExpr *Expr = MO.getExpr();
  MCExpr::ExprKind Kind = Expr->getKind();

  if (Kind == MCExpr::Binary) {
    Expr = static_cast<const MCBinaryExpr*>(Expr)->getLHS();
    Kind = Expr->getKind();
  }

  assert (Kind == MCExpr::SymbolRef);

  Cpu::Fixups FixupKind = Cpu::Fixups(0);

  switch(cast<MCSymbolRefExpr>(Expr)->getKind()) {
  case MCSymbolRefExpr::VK_Cpu_GPREL:
    FixupKind = Cpu::fixup_Cpu_GPREL16;
    break;
  case MCSymbolRefExpr::VK_Cpu_GOT_CALL:
    FixupKind = Cpu::fixup_Cpu_CALL16;
    break;
  case MCSymbolRefExpr::VK_Cpu_GOT16:
    FixupKind = Cpu::fixup_Cpu_GOT_Global;
    break;
  case MCSymbolRefExpr::VK_Cpu_GOT:
    FixupKind = Cpu::fixup_Cpu_GOT_Local;
    break;
  case MCSymbolRefExpr::VK_Cpu_ABS_HI:
    FixupKind = Cpu::fixup_Cpu_HI16;
    break;
  case MCSymbolRefExpr::VK_Cpu_ABS_LO:
    FixupKind = Cpu::fixup_Cpu_LO16;
    break;
  case MCSymbolRefExpr::VK_Cpu_GOT_HI16:
    FixupKind = Cpu::fixup_Cpu_GOT_HI16;
    break;
  case MCSymbolRefExpr::VK_Cpu_GOT_LO16:
    FixupKind = Cpu::fixup_Cpu_GOT_LO16;
    break;
  default:
    break;
  } // switch

  Fixups.push_back(MCFixup::Create(0, MO.getExpr(), MCFixupKind(FixupKind)));

  // All of the information is in the fixup.
  return 0;
}

/// getMemEncoding - Return binary encoding of memory related operand.
/// If the offset operand requires relocation, record the relocation.
unsigned
CpuMCCodeEmitter::getMemEncoding(const MCInst &MI, unsigned OpNo,
                                  SmallVectorImpl<MCFixup> &Fixups) const {
  // Base register is encoded in bits 20-16, offset is encoded in bits 15-0.
  assert(MI.getOperand(OpNo).isReg());
  unsigned RegBits = getMachineOpValue(MI, MI.getOperand(OpNo),Fixups) << 16;
  unsigned OffBits = getMachineOpValue(MI, MI.getOperand(OpNo+1), Fixups);

  return (OffBits & 0xFFFF) | RegBits;
}

#include "CpuGenMCCodeEmitter.inc"

