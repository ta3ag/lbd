//===- lib/MC/MCModule.cpp - MCModule implementation ----------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "llvm/MC/MCAtom.h"
#include "llvm/MC/MCModule.h"

using namespace llvm;

<<<<<<< HEAD
MCAtom *MCModule::createAtom(MCAtom::AtomType Type,
                             uint64_t Begin, uint64_t End) {
  assert(Begin < End && "Creating MCAtom with endpoints reversed?");
=======
static bool AtomComp(const MCAtom *L, uint64_t Addr) {
  return L->getEndAddr() < Addr;
}

static bool AtomCompInv(uint64_t Addr, const MCAtom *R) {
  return Addr < R->getEndAddr();
}

void MCModule::map(MCAtom *NewAtom) {
  uint64_t Begin = NewAtom->Begin;

  assert(Begin <= NewAtom->End && "Creating MCAtom with endpoints reversed?");
>>>>>>> llvmtrunk/master

  // Check for atoms already covering this range.
  IntervalMap<uint64_t, MCAtom*>::iterator I = OffsetMap.find(Begin);
  assert((!I.valid() || I.start() < End) && "Offset range already occupied!");

  // Create the new atom and add it to our maps.
  MCAtom *NewAtom = new MCAtom(Type, this, Begin, End);
  AtomAllocationTracker.insert(NewAtom);
  OffsetMap.insert(Begin, End, NewAtom);
  return NewAtom;
}

// remap - Update the interval mapping for an atom.
void MCModule::remap(MCAtom *Atom, uint64_t NewBegin, uint64_t NewEnd) {
  // Find and erase the old mapping.
  IntervalMap<uint64_t, MCAtom*>::iterator I = OffsetMap.find(Atom->Begin);
  assert(I.valid() && "Atom offset not found in module!");
  assert(*I == Atom && "Previous atom mapping was invalid!");
  I.erase();

  // FIXME: special case NewBegin == Atom->Begin

  // Insert the new mapping.
<<<<<<< HEAD
  OffsetMap.insert(NewBegin, NewEnd, Atom);
=======
  AtomListTy::iterator NewI = std::lower_bound(atom_begin(), atom_end(),
                                               NewBegin, AtomComp);
  assert((NewI == atom_end() || (*NewI)->getBeginAddr() > Atom->End)
         && "Offset range already occupied!");
  Atoms.insert(NewI, Atom);
>>>>>>> llvmtrunk/master

  // Update the atom internal bounds.
  Atom->Begin = NewBegin;
  Atom->End = NewEnd;
}

<<<<<<< HEAD
=======
const MCAtom *MCModule::findAtomContaining(uint64_t Addr) const {
  AtomListTy::const_iterator I = std::lower_bound(atom_begin(), atom_end(),
                                                  Addr, AtomComp);
  if (I != atom_end() && (*I)->getBeginAddr() <= Addr)
    return *I;
  return 0;
}

MCAtom *MCModule::findAtomContaining(uint64_t Addr) {
  return const_cast<MCAtom*>(
    const_cast<const MCModule *>(this)->findAtomContaining(Addr));
}

const MCAtom *MCModule::findFirstAtomAfter(uint64_t Addr) const {
  AtomListTy::const_iterator I = std::upper_bound(atom_begin(), atom_end(),
                                                  Addr, AtomCompInv);
  if (I != atom_end())
    return *I;
  return 0;
}

MCAtom *MCModule::findFirstAtomAfter(uint64_t Addr) {
  return const_cast<MCAtom*>(
    const_cast<const MCModule *>(this)->findFirstAtomAfter(Addr));
}

MCFunction *MCModule::createFunction(StringRef Name) {
  Functions.push_back(new MCFunction(Name, this));
  return Functions.back();
}

static bool CompBBToAtom(MCBasicBlock *BB, const MCTextAtom *Atom) {
  return BB->getInsts() < Atom;
}

void MCModule::splitBasicBlocksForAtom(const MCTextAtom *TA,
                                       const MCTextAtom *NewTA) {
  BBsByAtomTy::iterator
    I = std::lower_bound(BBsByAtom.begin(), BBsByAtom.end(),
                         TA, CompBBToAtom);
  for (; I != BBsByAtom.end() && (*I)->getInsts() == TA; ++I) {
    MCBasicBlock *BB = *I;
    MCBasicBlock *NewBB = &BB->getParent()->createBlock(*NewTA);
    BB->splitBasicBlock(NewBB);
  }
}

void MCModule::trackBBForAtom(const MCTextAtom *Atom, MCBasicBlock *BB) {
  assert(Atom == BB->getInsts() && "Text atom doesn't back the basic block!");
  BBsByAtomTy::iterator I = std::lower_bound(BBsByAtom.begin(),
                                             BBsByAtom.end(),
                                             Atom, CompBBToAtom);
  for (; I != BBsByAtom.end() && (*I)->getInsts() == Atom; ++I)
    if (*I == BB)
      return;
  BBsByAtom.insert(I, BB);
}

MCModule::~MCModule() {
  for (AtomListTy::iterator AI = atom_begin(),
                            AE = atom_end();
                            AI != AE; ++AI)
    delete *AI;
  for (FunctionListTy::iterator FI = func_begin(),
                                FE = func_end();
                                FI != FE; ++FI)
    delete *FI;
}
>>>>>>> llvmtrunk/master
