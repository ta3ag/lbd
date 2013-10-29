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

#define DYNLINKER

namespace lld {
namespace elf {
class Cpu0LinkingContext;

class Cpu0TargetHandler 
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
private:
  class GOTFile : public SimpleFile {
  public:
    GOTFile(const ELFLinkingContext &eti) : SimpleFile(eti, "GOTFile") {}
    llvm::BumpPtrAllocator _alloc;
  } _gotFile;

  Cpu0TargetRelocationHandler _relocationHandler;
  TargetLayout<Cpu0ELFType> _targetLayout;
};
} // end namespace elf
} // end namespace lld

#endif
