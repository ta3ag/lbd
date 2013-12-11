//===- lib/MC/MCAtom.cpp - MCAtom implementation --------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "llvm/MC/MCAtom.h"
#include "llvm/MC/MCModule.h"
#include "llvm/Support/ErrorHandling.h"

using namespace llvm;

<<<<<<< HEAD
void MCAtom::addInst(const MCInst &I, uint64_t Address, unsigned Size) {
  assert(Type == TextAtom && "Trying to add MCInst to a non-text atom!");

  assert(Address < End+Size &&
         "Instruction not contiguous with end of atom!");
  if (Address > End)
    Parent->remap(this, Begin, End+Size);

  Text.push_back(std::make_pair(Address, I));
=======
// Pin the vtable to this file.
void MCAtom::anchor() {}

void MCAtom::remap(uint64_t NewBegin, uint64_t NewEnd) {
  Parent->remap(this, NewBegin, NewEnd);
>>>>>>> llvmtrunk/master
}

void MCAtom::addData(const MCData &D) {
  assert(Type == DataAtom && "Trying to add MCData to a non-data atom!");
  Parent->remap(this, Begin, End+1);

  Data.push_back(D);
}

MCAtom *MCAtom::split(uint64_t SplitPt) {
  assert((SplitPt > Begin && SplitPt <= End) &&
         "Splitting at point not contained in atom!");

  // Compute the new begin/end points.
  uint64_t LeftBegin = Begin;
  uint64_t LeftEnd = SplitPt - 1;
  uint64_t RightBegin = SplitPt;
  uint64_t RightEnd = End;

  // Remap this atom to become the lower of the two new ones.
  Parent->remap(this, LeftBegin, LeftEnd);

  // Create a new atom for the higher atom.
  MCAtom *RightAtom = Parent->createAtom(Type, RightBegin, RightEnd);

<<<<<<< HEAD
  // Split the contents of the original atom between it and the new one.  The
  // precise method depends on whether this is a data or a text atom.
  if (isDataAtom()) {
    std::vector<MCData>::iterator I = Data.begin() + (RightBegin - LeftBegin);
=======
void MCDataAtom::addData(const MCData &D) {
  Data.push_back(D);
  if (Data.size() > End + 1 - Begin)
    remap(Begin, End + 1);
}
>>>>>>> llvmtrunk/master

    assert(I != Data.end() && "Split point not found in range!");

    std::copy(I, Data.end(), RightAtom->Data.end());
    Data.erase(I, Data.end());
  } else if (isTextAtom()) {
    std::vector<std::pair<uint64_t, MCInst> >::iterator I = Text.begin();

    while (I != Text.end() && I->first < SplitPt) ++I;

    assert(I != Text.end() && "Split point not found in disassembly!");
    assert(I->first == SplitPt &&
           "Split point does not fall on instruction boundary!");

    std::copy(I, Text.end(), RightAtom->Text.end());
    Text.erase(I, Text.end());
  } else
    llvm_unreachable("Unknown atom type!");

  return RightAtom;
}

void MCAtom::truncate(uint64_t TruncPt) {
  assert((TruncPt >= Begin && TruncPt < End) &&
         "Truncation point not contained in atom!");

<<<<<<< HEAD
  Parent->remap(this, Begin, TruncPt);
=======
void MCTextAtom::addInst(const MCInst &I, uint64_t Size) {
  if (NextInstAddress + Size - 1 > End)
    remap(Begin, NextInstAddress + Size - 1);
  Insts.push_back(MCDecodedInst(I, NextInstAddress, Size));
  NextInstAddress += Size;
}
>>>>>>> llvmtrunk/master

  if (isDataAtom()) {
    Data.resize(TruncPt - Begin + 1);
  } else if (isTextAtom()) {
    std::vector<std::pair<uint64_t, MCInst> >::iterator I = Text.begin();

    while (I != Text.end() && I->first <= TruncPt) ++I;

    assert(I != Text.end() && "Truncation point not found in disassembly!");
    assert(I->first == TruncPt+1 &&
           "Truncation point does not fall on instruction boundary");

    Text.erase(I, Text.end());
  } else
    llvm_unreachable("Unknown atom type!");
}

<<<<<<< HEAD
=======
MCTextAtom *MCTextAtom::split(uint64_t SplitPt) {
  uint64_t LBegin, LEnd, RBegin, REnd;
  remapForSplit(SplitPt, LBegin, LEnd, RBegin, REnd);

  MCTextAtom *RightAtom = Parent->createTextAtom(RBegin, REnd);
  RightAtom->setName(getName());

  InstListTy::iterator I = Insts.begin();
  while (I != Insts.end() && I->Address < SplitPt) ++I;
  assert(I != Insts.end() && "Split point not found in disassembly!");
  assert(I->Address == SplitPt &&
         "Split point does not fall on instruction boundary!");

  std::copy(I, Insts.end(), std::back_inserter(RightAtom->Insts));
  Insts.erase(I, Insts.end());
  Parent->splitBasicBlocksForAtom(this, RightAtom);
  return RightAtom;
}
>>>>>>> llvmtrunk/master
