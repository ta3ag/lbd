//===- lib/ReaderWriter/ELF/Cpu0/Cpu0TargetHandler.h ------------------===//
//
//                             The LLVM Linker
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef LLD_READER_WRITER_ELF_Cpu0_TARGET_HANDLER_H
#define LLD_READER_WRITER_ELF_Cpu0_TARGET_HANDLER_H

#include "DefaultTargetHandler.h"
#include "Cpu0RelocationHandler.h"
#include "TargetLayout.h"

#include "lld/ReaderWriter/Simple.h"

#include "lld/Core/Atom.h"

struct Cpu0AtomAddress {
  const lld::Atom* PAtom;
  uint64_t Addr;
};

extern Cpu0AtomAddress Cpu0AtomAddr[100];
extern int Cpu0AtomAddrSize;

extern uint64_t textSectionAddr;

namespace lld {
namespace elf {
typedef llvm::object::ELFType<llvm::support::big, 4, false> Cpu0ELFType;
class Cpu0LinkingContext;

#if 0
class Cpu0TargetLayout LLVM_FINAL
    : public TargetLayout<Cpu0ELFType> {
public:
  Cpu0TargetLayout() { };
  inline void finalize() {
    auto gotpltSection = findOutputSection(".got.plt");
    if (gotpltSection)
      gotpltSection->setAddr(0x2000);
    int tmp = 12;
    TargetLayout<Cpu0ELFType>::finalize();
  }
};
#endif

class Cpu0TargetHandler LLVM_FINAL
    : public DefaultTargetHandler<Cpu0ELFType> {
public:
  Cpu0TargetHandler(Cpu0LinkingContext &targetInfo);

  virtual TargetLayout<Cpu0ELFType> &targetLayout() {
    return _targetLayout;
  }

  virtual const Cpu0TargetRelocationHandler &getRelocationHandler() const {
    return _relocationHandler;
  }

  virtual void addFiles(InputFiles &f);
#if 0
  void finalizeSymbolValues() {
//  void createDefaultSections() {
    if (_context.isDynamic()) {
#if 0
      auto gotAtomIter =
          _targetLayout.findAbsoluteAtom("_GLOBAL_OFFSET_TABLE_");
      _gotSymAtom = (*gotAtomIter);
#endif
      auto gotpltSection = _targetLayout.findOutputSection(".got.plt");
#if 1
      if (gotpltSection)
//        _gotSymAtom->_virtualAddr = gotpltSection->virtualAddr();
//        _gotSymAtom->_virtualAddr = 0x2000;
        gotpltSection->setAddr(0x2000);
#if 0
      else
        _gotSymAtom->_virtualAddr = 0;
#endif
      int tmp = 12;
#endif
    }
  }
#endif

private:
  class GOTFile : public SimpleFile {
  public:
    GOTFile(const ELFLinkingContext &eti) : SimpleFile(eti, "GOTFile") {}
    llvm::BumpPtrAllocator _alloc;
  } _gotFile;

  Cpu0TargetRelocationHandler _relocationHandler;
  TargetLayout<Cpu0ELFType> _targetLayout;
#if 0
//  Cpu0TargetLayout _targetLayout;
  AtomLayout *_gotSymAtom;
#endif
};
} // end namespace elf
} // end namespace lld

#endif
