//===-- llvm/MC/MCAtom.h - MCAtom class ---------------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains the declaration of the MCAtom class, which is used to
// represent a contiguous region in a decoded object that is uniformly data or
// instructions;
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_MC_MCATOM_H
#define LLVM_MC_MCATOM_H

#include "llvm/ADT/ArrayRef.h"
#include "llvm/MC/MCInst.h"
#include "llvm/Support/DataTypes.h"
#include <vector>

namespace llvm {

class MCModule;

/// MCData - An entry in a data MCAtom.
// NOTE: This may change to a more complex type in the future.
typedef uint8_t MCData;

/// \brief Represents a contiguous range of either instructions (a TextAtom)
/// or data (a DataAtom).  Address ranges are expressed as _closed_ intervals.
class MCAtom {
<<<<<<< HEAD
=======
  virtual void anchor();
public:
  virtual ~MCAtom() {}

  enum AtomKind { TextAtom, DataAtom };
  AtomKind getKind() const { return Kind; }

  /// \brief Get the start address of the atom.
  uint64_t getBeginAddr() const { return Begin; }
  /// \brief Get the end address, i.e. the last one inside the atom.
  uint64_t getEndAddr() const { return End; }

  /// \name Atom modification methods:
  /// When modifying a TextAtom, keep instruction boundaries in mind.
  /// For instance, split must me given the start address of an instruction.
  /// @{

  /// \brief Splits the atom in two at a given address.
  /// \param SplitPt Address at which to start a new atom, splitting this one.
  /// \returns The newly created atom starting at \p SplitPt.
  virtual MCAtom *split(uint64_t SplitPt) = 0;

  /// \brief Truncates an atom, discarding everything after \p TruncPt.
  /// \param TruncPt Last byte address to be contained in this atom.
  virtual void truncate(uint64_t TruncPt) = 0;
  /// @}

  /// \name Naming:
  ///
  /// This is mostly for display purposes, and may contain anything that hints
  /// at what the atom contains: section or symbol name, BB start address, ..
  /// @{
  StringRef getName() const { return Name; }
  void setName(StringRef NewName) { Name = NewName.str(); }
  /// @}

protected:
  const AtomKind Kind;
  std::string Name;
  MCModule *Parent;
  uint64_t Begin, End;

>>>>>>> llvmtrunk/master
  friend class MCModule;
  typedef enum { TextAtom, DataAtom } AtomType;

  AtomType Type;
  MCModule *Parent;
  uint64_t Begin, End;

  std::vector<std::pair<uint64_t, MCInst> > Text;
  std::vector<MCData> Data;

<<<<<<< HEAD
=======
  /// \brief The address of the next appended instruction, i.e., the
  /// address immediately after the last instruction in the atom.
  uint64_t NextInstAddress;
public:
  /// Append an instruction, expanding the atom if necessary.
  void addInst(const MCInst &Inst, uint64_t Size);

  /// \name Instruction list access
  /// @{
  typedef InstListTy::const_iterator const_iterator;
  const_iterator begin() const { return Insts.begin(); }
  const_iterator end()   const { return Insts.end(); }

  const MCDecodedInst &back() const { return Insts.back(); }
  const MCDecodedInst &at(size_t n) const { return Insts.at(n); }
  size_t size() const { return Insts.size(); }
  /// @}

  /// \name Atom type specific split/truncate logic.
  /// @{
  MCTextAtom *split(uint64_t SplitPt) LLVM_OVERRIDE;
  void     truncate(uint64_t TruncPt) LLVM_OVERRIDE;
  /// @}

  // Class hierarchy.
  static bool classof(const MCAtom *A) { return A->getKind() == TextAtom; }
private:
  friend class MCModule;
>>>>>>> llvmtrunk/master
  // Private constructor - only callable by MCModule
  MCAtom(AtomType T, MCModule *P, uint64_t B, uint64_t E)
    : Type(T), Parent(P), Begin(B), End(E) { }

public:
  bool isTextAtom() const { return Type == TextAtom; }
  bool isDataAtom() const { return Type == DataAtom; }

  void addInst(const MCInst &I, uint64_t Address, unsigned Size);
  void addData(const MCData &D);

<<<<<<< HEAD
  /// split - Splits the atom in two at a given address, which must align with
  /// and instruction boundary if this is a TextAtom.  Returns the newly created
  /// atom representing the high part of the split.
  MCAtom *split(uint64_t SplitPt);

  /// truncate - Truncates an atom so that TruncPt is the last byte address
  /// contained in the atom.
  void truncate(uint64_t TruncPt);
=======
  /// Get a reference to the data in this atom.
  ArrayRef<MCData> getData() const { return Data; }

  /// \name Atom type specific split/truncate logic.
  /// @{
  MCDataAtom *split(uint64_t SplitPt) LLVM_OVERRIDE;
  void     truncate(uint64_t TruncPt) LLVM_OVERRIDE;
  /// @}

  // Class hierarchy.
  static bool classof(const MCAtom *A) { return A->getKind() == DataAtom; }
private:
  friend class MCModule;
  // Private constructor - only callable by MCModule
  MCDataAtom(MCModule *P, uint64_t Begin, uint64_t End)
    : MCAtom(DataAtom, P, Begin, End) {
    Data.reserve(End + 1 - Begin);
  }
>>>>>>> llvmtrunk/master
};

}

#endif

