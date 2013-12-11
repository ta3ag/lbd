//===-- llvm/MC/MCModule.h - MCModule class ---------------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains the declaration of the MCModule class, which is used to
// represent a complete, disassembled object file or executable.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_MC_MCMODULE_H
#define LLVM_MC_MCMODULE_H

#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/IntervalMap.h"
#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/Support/DataTypes.h"

namespace llvm {

class MCAtom;
<<<<<<< HEAD
=======
class MCBasicBlock;
class MCDataAtom;
class MCFunction;
class MCObjectDisassembler;
class MCTextAtom;
>>>>>>> llvmtrunk/master

/// MCModule - This class represent a completely disassembled object file or
/// executable.  It comprises a list of MCAtom's, and a branch target table.
/// Each atom represents a contiguous range of either instructions or data.
class MCModule {
  /// AtomAllocationTracker - An MCModule owns its component MCAtom's, so it
  /// must track them in order to ensure they are properly freed as atoms are
  /// merged or otherwise manipulated.
  SmallPtrSet<MCAtom*, 8> AtomAllocationTracker;

  /// OffsetMap - Efficiently maps offset ranges to MCAtom's.
  IntervalMap<uint64_t, MCAtom*> OffsetMap;

<<<<<<< HEAD
  /// BranchTargetMap - Maps offsets that are determined to be branches and
  /// can be statically resolved to their target offsets.
  DenseMap<uint64_t, MCAtom*> BranchTargetMap;

  friend class MCAtom;

  /// remap - Update the interval mapping for an MCAtom.
  void remap(MCAtom *Atom, uint64_t NewBegin, uint64_t NewEnd);

public:
  MCModule(IntervalMap<uint64_t, MCAtom*>::Allocator &A) : OffsetMap(A) { }

  /// createAtom - Creates a new MCAtom covering the specified offset range.
  MCAtom *createAtom(MCAtom::AtomType Type, uint64_t Begin, uint64_t End);
=======
  // For access to map/remap.
  friend class MCAtom;

  /// \brief Remap \p Atom to the given range, and update its Begin/End fields.
  /// \param Atom An atom belonging to this module.
  /// An atom should always use this method to update its bounds, because this
  /// enables the owning MCModule to keep track of its atoms.
  void remap(MCAtom *Atom, uint64_t NewBegin, uint64_t NewEnd);

  /// \brief Insert an atom in the module, using its Begin and End addresses.
  void map(MCAtom *NewAtom);
  /// @}

  /// \name Basic block tracking
  /// @{
  typedef std::vector<MCBasicBlock*> BBsByAtomTy;
  BBsByAtomTy BBsByAtom;

  // For access to basic block > atom tracking.
  friend class MCBasicBlock;
  friend class MCTextAtom;

  /// \brief Keep track of \p BBBackedByAtom as being backed by \p Atom.
  /// This is used to update succs/preds when \p Atom is split.
  void trackBBForAtom(const MCTextAtom *Atom, MCBasicBlock *BBBackedByAtom);
  void splitBasicBlocksForAtom(const MCTextAtom *TA, const MCTextAtom *NewTA);
  /// @}

  /// \name Function tracking
  /// @{
  typedef std::vector<MCFunction*> FunctionListTy;
  FunctionListTy Functions;
  /// @}

  /// The address of the entrypoint function.
  uint64_t Entrypoint;

  MCModule           (const MCModule &) LLVM_DELETED_FUNCTION;
  MCModule& operator=(const MCModule &) LLVM_DELETED_FUNCTION;

  // MCObjectDisassembler creates MCModules.
  friend class MCObjectDisassembler;

public:
  MCModule() : Entrypoint(0) { }
  ~MCModule();

  /// \name Create a new MCAtom covering the specified offset range.
  /// @{
  MCTextAtom *createTextAtom(uint64_t Begin, uint64_t End);
  MCDataAtom *createDataAtom(uint64_t Begin, uint64_t End);
  /// @}

  /// \name Access to the owned atom list, ordered by begin address.
  /// @{
  const MCAtom *findAtomContaining(uint64_t Addr) const;
        MCAtom *findAtomContaining(uint64_t Addr);
  const MCAtom *findFirstAtomAfter(uint64_t Addr) const;
        MCAtom *findFirstAtomAfter(uint64_t Addr);

  typedef AtomListTy::const_iterator const_atom_iterator;
  typedef AtomListTy::      iterator       atom_iterator;
  const_atom_iterator atom_begin() const { return Atoms.begin(); }
        atom_iterator atom_begin()       { return Atoms.begin(); }
  const_atom_iterator atom_end()   const { return Atoms.end(); }
        atom_iterator atom_end()         { return Atoms.end(); }
  /// @}

  /// \brief Create a new MCFunction.
  MCFunction *createFunction(StringRef Name);

  /// \name Access to the owned function list.
  /// @{
  typedef FunctionListTy::const_iterator const_func_iterator;
  typedef FunctionListTy::      iterator       func_iterator;
  const_func_iterator func_begin() const { return Functions.begin(); }
        func_iterator func_begin()       { return Functions.begin(); }
  const_func_iterator func_end()   const { return Functions.end(); }
        func_iterator func_end()         { return Functions.end(); }
  /// @}

  /// \brief Get the address of the entrypoint function, or 0 if there is none.
  uint64_t getEntrypoint() const { return Entrypoint; }
>>>>>>> llvmtrunk/master
};

}

#endif

