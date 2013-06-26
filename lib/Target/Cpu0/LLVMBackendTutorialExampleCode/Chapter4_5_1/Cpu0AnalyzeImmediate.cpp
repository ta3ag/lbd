//===-- Cpu0AnalyzeImmediate.cpp - Analyze Immediates ---------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#include "Cpu0AnalyzeImmediate.h"
#include "Cpu0.h"
#include "llvm/Support/MathExtras.h"

using namespace llvm;

Cpu0AnalyzeImmediate::Inst::Inst(unsigned O, unsigned I) : Opc(O), ImmOpnd(I) {}

// Add I to the instruction sequences.
void Cpu0AnalyzeImmediate::AddInstr(InstSeqLs &SeqLs, const Inst &I) {
  // Add an instruction seqeunce consisting of just I.
  if (SeqLs.empty()) {
    SeqLs.push_back(InstSeq(1, I));
    return;
  }

  for (InstSeqLs::iterator Iter = SeqLs.begin(); Iter != SeqLs.end(); ++Iter)
    Iter->push_back(I);
}

void Cpu0AnalyzeImmediate::GetInstSeqLsADDiu(uint64_t Imm, unsigned RemSize,
                                             InstSeqLs &SeqLs) {
  GetInstSeqLs((Imm + 0x8000ULL) & 0xffffffffffff0000ULL, RemSize, SeqLs);
  AddInstr(SeqLs, Inst(ADDiu, Imm & 0xffffULL));
}

void Cpu0AnalyzeImmediate::GetInstSeqLsSHL(uint64_t Imm, unsigned RemSize,
                                           InstSeqLs &SeqLs) {
  unsigned Shamt = CountTrailingZeros_64(Imm);
  GetInstSeqLs(Imm >> Shamt, RemSize - Shamt, SeqLs);
  AddInstr(SeqLs, Inst(SHL, Shamt));
}

void Cpu0AnalyzeImmediate::GetInstSeqLs(uint64_t Imm, unsigned RemSize,
                                        InstSeqLs &SeqLs) {
  uint64_t MaskedImm = Imm & (0xffffffffffffffffULL >> (64 - Size));

  // Do nothing if Imm is 0.
  if (!MaskedImm)
    return;

  // A single ADDiu will do if RemSize <= 16.
  if (RemSize <= 16) {
    AddInstr(SeqLs, Inst(ADDiu, MaskedImm));
    return;
  }

  // Shift if the lower 16-bit is cleared.
  if (!(Imm & 0xffff)) {
    GetInstSeqLsSHL(Imm, RemSize, SeqLs);
    return;
  }

  GetInstSeqLsADDiu(Imm, RemSize, SeqLs);
}

const Cpu0AnalyzeImmediate::InstSeq
&Cpu0AnalyzeImmediate::Analyze(uint64_t Imm, unsigned Size,
                               bool LastInstrIsADDiu) {
  this->Size = Size;

  ADDiu = Cpu0::ADDiu;
  OR = Cpu0::OR;
  SHL = Cpu0::SHL;
  InstSeqLs SeqLs;

  // Get the list of instruction sequences.
  if (LastInstrIsADDiu | !Imm)
    GetInstSeqLsADDiu(Imm, Size, SeqLs);
  else
    GetInstSeqLs(Imm, Size, SeqLs);

  Insts.clear();
  Insts.append(SeqLs.begin()->begin(), SeqLs.begin()->end());

  return Insts;
}
