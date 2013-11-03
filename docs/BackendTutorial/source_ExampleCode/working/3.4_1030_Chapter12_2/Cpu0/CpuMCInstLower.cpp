//===-- CpuMCInstLower.cpp - Convert Cpu MachineInstr to MCInst ---------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains code to lower Cpu MachineInstrs to their corresponding
// MCInst records.
//
//===----------------------------------------------------------------------===//

#include "CpuMCInstLower.h"
#include "CpuAsmPrinter.h"
#include "CpuInstrInfo.h"
#include "MCTargetDesc/CpuBaseInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/CodeGen/MachineOperand.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCInst.h"
#include "llvm/Target/Mangler.h"

using namespace llvm;

CpuMCInstLower::CpuMCInstLower(CpuAsmPrinter &asmprinter)
  : AsmPrinter(asmprinter) {}

void CpuMCInstLower::Initialize(MCContext* C) {
  Ctx = C;
}

MCOperand CpuMCInstLower::LowerSymbolOperand(const MachineOperand &MO,
                                              MachineOperandType MOTy,
                                              unsigned Offset) const {
  MCSymbolRefExpr::VariantKind Kind;
  const MCSymbol *Symbol;

  switch(MO.getTargetFlags()) {
  default:                   llvm_unreachable("Invalid target flag!");
  case CpuII::MO_NO_FLAG:   Kind = MCSymbolRefExpr::VK_None; break;

// Cpu_GPREL is for llc -march=cpu -relocation-model=static -cpu-islinux-
//  format=false (global var in .sdata).
  case CpuII::MO_GPREL:     Kind = MCSymbolRefExpr::VK_Cpu_GPREL; break;

  case CpuII::MO_GOT_CALL:  Kind = MCSymbolRefExpr::VK_Cpu_GOT_CALL; break;
  case CpuII::MO_GOT16:     Kind = MCSymbolRefExpr::VK_Cpu_GOT16; break;
  case CpuII::MO_GOT:       Kind = MCSymbolRefExpr::VK_Cpu_GOT; break;
// ABS_HI and ABS_LO is for llc -march=cpu -relocation-model=static (global 
//  var in .data).
  case CpuII::MO_ABS_HI:    Kind = MCSymbolRefExpr::VK_Cpu_ABS_HI; break;
  case CpuII::MO_ABS_LO:    Kind = MCSymbolRefExpr::VK_Cpu_ABS_LO; break;
  case CpuII::MO_GOT_HI16:  Kind = MCSymbolRefExpr::VK_Cpu_GOT_HI16; break;
  case CpuII::MO_GOT_LO16:  Kind = MCSymbolRefExpr::VK_Cpu_GOT_LO16; break;
  }

  switch (MOTy) {
  case MachineOperand::MO_MachineBasicBlock:
    Symbol = MO.getMBB()->getSymbol();
    break;

  case MachineOperand::MO_GlobalAddress:
    Symbol = AsmPrinter.getSymbol(MO.getGlobal());
    break;

  case MachineOperand::MO_BlockAddress:
    Symbol = AsmPrinter.GetBlockAddressSymbol(MO.getBlockAddress());
    Offset += MO.getOffset();
    break;

  case MachineOperand::MO_ExternalSymbol:
    Symbol = AsmPrinter.GetExternalSymbolSymbol(MO.getSymbolName());
    Offset += MO.getOffset();
    break;

  default:
    llvm_unreachable("<unknown operand type>");
  }

  const MCSymbolRefExpr *MCSym = MCSymbolRefExpr::Create(Symbol, Kind, *Ctx);

  if (!Offset)
    return MCOperand::CreateExpr(MCSym);

  // Assume offset is never negative.
  assert(Offset > 0);

  const MCConstantExpr *OffsetExpr =  MCConstantExpr::Create(Offset, *Ctx);
  const MCBinaryExpr *AddExpr = MCBinaryExpr::CreateAdd(MCSym, OffsetExpr, *Ctx);
  return MCOperand::CreateExpr(AddExpr);
}

static void CreateMCInst(MCInst& Inst, unsigned Opc, const MCOperand& Opnd0,
                         const MCOperand& Opnd1,
                         const MCOperand& Opnd2 = MCOperand()) {
  Inst.setOpcode(Opc);
  Inst.addOperand(Opnd0);
  Inst.addOperand(Opnd1);
  if (Opnd2.isValid())
    Inst.addOperand(Opnd2);
}

// Lower ".cpload $reg" to
//  "lui   $gp, %hi(_gp_disp)"
//  "addiu $gp, $gp, %lo(_gp_disp)"
//  "addu  $gp, $gp, $t9"
void CpuMCInstLower::LowerCPLOAD(SmallVector<MCInst, 4>& MCInsts) {
  MCOperand GPReg = MCOperand::CreateReg(Cpu::GP);
  MCOperand T9Reg = MCOperand::CreateReg(Cpu::T9);
  StringRef SymName("_gp_disp");
  const MCSymbol *Sym = Ctx->GetOrCreateSymbol(SymName);
  const MCSymbolRefExpr *MCSym;

  MCSym = MCSymbolRefExpr::Create(Sym, MCSymbolRefExpr::VK_Cpu_ABS_HI, *Ctx);
  MCOperand SymHi = MCOperand::CreateExpr(MCSym);
  MCSym = MCSymbolRefExpr::Create(Sym, MCSymbolRefExpr::VK_Cpu_ABS_LO, *Ctx);
  MCOperand SymLo = MCOperand::CreateExpr(MCSym);

  MCInsts.resize(3);

  CreateMCInst(MCInsts[0], Cpu::LUi, GPReg, SymHi);
  CreateMCInst(MCInsts[1], Cpu::ADDiu, GPReg, GPReg, SymLo);
  CreateMCInst(MCInsts[2], Cpu::ADD, GPReg, GPReg, T9Reg);
}

// Lower ".cprestore offset" to "st $gp, offset($sp)".
void CpuMCInstLower::LowerCPRESTORE(int64_t Offset,
                                     SmallVector<MCInst, 4>& MCInsts) {
  assert(isInt<32>(Offset) && (Offset >= 0) &&
         "Imm operand of .cprestore must be a non-negative 32-bit value.");

  MCOperand SPReg = MCOperand::CreateReg(Cpu::SP), BaseReg = SPReg;
  MCOperand GPReg = MCOperand::CreateReg(Cpu::GP);
  MCOperand ZEROReg = MCOperand::CreateReg(Cpu::ZERO);

  if (!isInt<16>(Offset)) {
    unsigned Hi = ((Offset + 0x8000) >> 16) & 0xffff;
    Offset &= 0xffff;
    MCOperand ATReg = MCOperand::CreateReg(Cpu::AT);
    BaseReg = ATReg;

    // lui   at,hi
    // add   at,at,sp
    MCInsts.resize(2);
    CreateMCInst(MCInsts[0], Cpu::LUi, ATReg, ZEROReg, MCOperand::CreateImm(Hi));
    CreateMCInst(MCInsts[1], Cpu::ADD, ATReg, ATReg, SPReg);
  }

  MCInst St;
  CreateMCInst(St, Cpu::ST, GPReg, BaseReg, MCOperand::CreateImm(Offset));
  MCInsts.push_back(St);
}

MCOperand CpuMCInstLower::LowerOperand(const MachineOperand& MO,
                                        unsigned offset) const {
  MachineOperandType MOTy = MO.getType();

  switch (MOTy) {
  default: llvm_unreachable("unknown operand type");
  case MachineOperand::MO_Register:
    // Ignore all implicit register operands.
    if (MO.isImplicit()) break;
    return MCOperand::CreateReg(MO.getReg());
  case MachineOperand::MO_Immediate:
    return MCOperand::CreateImm(MO.getImm() + offset);
  case MachineOperand::MO_MachineBasicBlock:
  case MachineOperand::MO_GlobalAddress:
  case MachineOperand::MO_ExternalSymbol:
  case MachineOperand::MO_BlockAddress:
    return LowerSymbolOperand(MO, MOTy, offset);
  case MachineOperand::MO_RegisterMask:
    break;
 }

  return MCOperand();
}

void CpuMCInstLower::Lower(const MachineInstr *MI, MCInst &OutMI) const {
  OutMI.setOpcode(MI->getOpcode());

  for (unsigned i = 0, e = MI->getNumOperands(); i != e; ++i) {
    const MachineOperand &MO = MI->getOperand(i);
    MCOperand MCOp = LowerOperand(MO);

    if (MCOp.isValid())
      OutMI.addOperand(MCOp);
  }
}

