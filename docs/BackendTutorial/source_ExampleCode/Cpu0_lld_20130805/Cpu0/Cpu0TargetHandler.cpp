//===- lib/ReaderWriter/ELF/Cpu0/Cpu0TargetHandler.cpp ----------------===//
//
//                             The LLVM Linker
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "Atoms.h"
#include "Cpu0TargetHandler.h"
#include "Cpu0TargetInfo.h"

using namespace lld;
using namespace elf;

Cpu0TargetHandler::Cpu0TargetHandler(Cpu0TargetInfo &targetInfo)
    : DefaultTargetHandler(targetInfo), _gotFile(targetInfo),
      _relocationHandler(targetInfo), _targetLayout(targetInfo) {}

void Cpu0TargetHandler::addFiles(InputFiles &f) {
  _gotFile.addAtom(*new (_gotFile._alloc) GLOBAL_OFFSET_TABLEAtom(_gotFile));
  _gotFile.addAtom(*new (_gotFile._alloc) TLSGETADDRAtom(_gotFile));
  f.appendFile(_gotFile);
}
