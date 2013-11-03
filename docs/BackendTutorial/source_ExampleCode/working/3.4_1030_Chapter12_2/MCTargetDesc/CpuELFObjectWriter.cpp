//===-- CpuELFObjectWriter.cpp - Cpu ELF Writer -------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "MCTargetDesc/CpuBaseInfo.h"
#include "MCTargetDesc/CpuFixupKinds.h"
#include "MCTargetDesc/CpuMCTargetDesc.h"
#include "llvm/MC/MCAssembler.h"
#include "llvm/MC/MCELFObjectWriter.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCSection.h"
#include "llvm/MC/MCValue.h"
#include "llvm/Support/ErrorHandling.h"
#include <list>

using namespace llvm;

namespace {
  struct RelEntry {
    RelEntry(const ELFRelocationEntry &R, const MCSymbol *S, int64_t O) :
      Reloc(R), Sym(S), Offset(O) {}
    ELFRelocationEntry Reloc;
    const MCSymbol *Sym;
    int64_t Offset;
  };

  typedef std::list<RelEntry> RelLs;
  typedef RelLs::iterator RelLsIter;

  class CpuELFObjectWriter : public MCELFObjectTargetWriter {
  public:
    CpuELFObjectWriter(uint8_t OSABI);

    virtual ~CpuELFObjectWriter();

    virtual unsigned GetRelocType(const MCValue &Target, const MCFixup &Fixup,
                                  bool IsPCRel, bool IsRelocWithSymbol,
                                  int64_t Addend) const;
    virtual unsigned getEFlags() const;
    virtual const MCSymbol *ExplicitRelSym(const MCAssembler &Asm,
                                           const MCValue &Target,
                                           const MCFragment &F,
                                           const MCFixup &Fixup,
                                           bool IsPCRel) const;
    virtual void sortRelocs(const MCAssembler &Asm,
                            std::vector<ELFRelocationEntry> &Relocs);
  };
}

CpuELFObjectWriter::CpuELFObjectWriter(uint8_t OSABI)
  : MCELFObjectTargetWriter(/*_is64Bit=false*/ false, OSABI, ELF::EM_CPU,
                            /*HasRelocationAddend*/ false) {}

CpuELFObjectWriter::~CpuELFObjectWriter() {}

// FIXME: get the real EABI Version from the Subtarget class.
unsigned CpuELFObjectWriter::getEFlags() const {

  // FIXME: We can't tell if we are PIC (dynamic) or CPIC (static)
  unsigned Flag = ELF::EF_CPU_NOREORDER;

  Flag |= ELF::EF_CPU_ARCH_32R2;
  return Flag;
}

const MCSymbol *CpuELFObjectWriter::ExplicitRelSym(const MCAssembler &Asm,
                                                    const MCValue &Target,
                                                    const MCFragment &F,
                                                    const MCFixup &Fixup,
                                                    bool IsPCRel) const {
  assert(Target.getSymA() && "SymA cannot be 0.");
  const MCSymbol &Sym = Target.getSymA()->getSymbol().AliasedSymbol();

  if (Sym.getSection().getKind().isMergeableCString() ||
      Sym.getSection().getKind().isMergeableConst())
    return &Sym;

  return NULL;
}

unsigned CpuELFObjectWriter::GetRelocType(const MCValue &Target,
                                           const MCFixup &Fixup,
                                           bool IsPCRel,
                                           bool IsRelocWithSymbol,
                                           int64_t Addend) const {
  // determine the type of the relocation
  unsigned Type = (unsigned)ELF::R_CPU_NONE;
  unsigned Kind = (unsigned)Fixup.getKind();

  switch (Kind) {
  default:
    llvm_unreachable("invalid fixup kind!");
  case FK_Data_4:
    Type = ELF::R_CPU_32;
    break;
  case Cpu::fixup_Cpu_24:
    Type = ELF::R_CPU_24;
    break;
  case Cpu::fixup_Cpu_32:
    Type = ELF::R_CPU_32;
    break;
  case Cpu::fixup_Cpu_HI16:
    Type = ELF::R_CPU_HI16;
    break;
  case Cpu::fixup_Cpu_LO16:
    Type = ELF::R_CPU_LO16;
    break;
  case Cpu::fixup_Cpu_GPREL16:
    Type = ELF::R_CPU_GPREL16;
    break;
  case Cpu::fixup_Cpu_CALL16:
    Type = ELF::R_CPU_CALL16;
    break;
  case Cpu::fixup_Cpu_GOT_Global:
  case Cpu::fixup_Cpu_GOT_Local:
    Type = ELF::R_CPU_GOT16;
    break;
  case Cpu::fixup_Cpu_Branch_PCRel:
  case Cpu::fixup_Cpu_PC24:
    Type = ELF::R_CPU_PC24;
    break;
  case Cpu::fixup_Cpu_GOT_HI16:
    Type = ELF::R_CPU_GOT_HI16;
    break;
  case Cpu::fixup_Cpu_GOT_LO16:
    Type = ELF::R_CPU_GOT_LO16;
    break;
  }

  return Type;
}

// Return true if R is either a GOT16 against a local symbol or HI16.
static bool NeedsMatchingLo(const MCAssembler &Asm, const RelEntry &R) {
  if (!R.Sym)
    return false;

  MCSymbolData &SD = Asm.getSymbolData(R.Sym->AliasedSymbol());

  return ((R.Reloc.Type == ELF::R_CPU_GOT16) && !SD.isExternal()) ||
    (R.Reloc.Type == ELF::R_CPU_HI16);
}

static bool HasMatchingLo(const MCAssembler &Asm, RelLsIter I, RelLsIter Last) {
  if (I == Last)
    return false;

  RelLsIter Hi = I++;

  return (I->Reloc.Type == ELF::R_CPU_LO16) && (Hi->Sym == I->Sym) &&
    (Hi->Offset == I->Offset);
}

static bool HasSameSymbol(const RelEntry &R0, const RelEntry &R1) {
  return R0.Sym == R1.Sym;
}

static int CompareOffset(const RelEntry &R0, const RelEntry &R1) {
  return (R0.Offset > R1.Offset) ? 1 : ((R0.Offset == R1.Offset) ? 0 : -1);
}

void CpuELFObjectWriter::sortRelocs(const MCAssembler &Asm,
                                     std::vector<ELFRelocationEntry> &Relocs) {
  // Call the defualt function first. Relocations are sorted in descending
  // order of r_offset.
  MCELFObjectTargetWriter::sortRelocs(Asm, Relocs);
  
  RelLs RelocLs;
  std::vector<RelLsIter> Unmatched;

  // Fill RelocLs. Traverse Relocs backwards so that relocations in RelocLs
  // are in ascending order of r_offset.
  for (std::vector<ELFRelocationEntry>::reverse_iterator R = Relocs.rbegin();
       R != Relocs.rend(); ++R) {
     std::pair<const MCSymbolRefExpr*, int64_t> P =
       CpuGetSymAndOffset(*R->Fixup);
     RelocLs.push_back(RelEntry(*R, P.first ? &P.first->getSymbol() : 0,
                                P.second));
  }

  // Get list of unmatched HI16 and GOT16.
  for (RelLsIter R = RelocLs.begin(); R != RelocLs.end(); ++R)
    if (NeedsMatchingLo(Asm, *R) && !HasMatchingLo(Asm, R, --RelocLs.end()))
      Unmatched.push_back(R);

  // Insert unmatched HI16 and GOT16 immediately before their matching LO16.
  for (std::vector<RelLsIter>::iterator U = Unmatched.begin();
       U != Unmatched.end(); ++U) {
    RelLsIter LoPos = RelocLs.end(), HiPos = *U;
    bool MatchedLo = false;

    for (RelLsIter R = RelocLs.begin(); R != RelocLs.end(); ++R) {
      if ((R->Reloc.Type == ELF::R_CPU_LO16) && HasSameSymbol(*HiPos, *R) &&
          (CompareOffset(*R, *HiPos) >= 0) &&
          ((LoPos == RelocLs.end()) || ((CompareOffset(*R, *LoPos) < 0)) ||
           (!MatchedLo && !CompareOffset(*R, *LoPos))))
        LoPos = R;

      MatchedLo = NeedsMatchingLo(Asm, *R) &&
        HasMatchingLo(Asm, R, --RelocLs.end());
    }

    // If a matching LoPos was found, move HiPos and insert it before LoPos.
    // Make the offsets of HiPos and LoPos match.
    if (LoPos != RelocLs.end()) {
      HiPos->Offset = LoPos->Offset;
      RelocLs.insert(LoPos, *HiPos);
      RelocLs.erase(HiPos);
    }
  }

  // Put the sorted list back in reverse order.
  assert(Relocs.size() == RelocLs.size());
  unsigned I = RelocLs.size();

  for (RelLsIter R = RelocLs.begin(); R != RelocLs.end(); ++R)
    Relocs[--I] = R->Reloc;
}

MCObjectWriter *llvm::createCpuELFObjectWriter(raw_ostream &OS,
                                                uint8_t OSABI,
                                                bool IsLittleEndian) {
  MCELFObjectTargetWriter *MOTW = new CpuELFObjectWriter(OSABI);
  return createELFObjectWriter(MOTW, OS, IsLittleEndian);
}
