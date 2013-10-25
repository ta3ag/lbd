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
#include "Cpu0LinkingContext.h"

using namespace lld;
using namespace elf;

#if 1
Cpu0AtomAddress Cpu0AtomAddr[100];
int Cpu0AtomAddrSize = 0;
#endif

uint64_t textSectionAddr;

Cpu0TargetHandler::Cpu0TargetHandler(Cpu0LinkingContext &context)
    : DefaultTargetHandler(context), _gotFile(context),
      _relocationHandler(context), _targetLayout(context) {}

void Cpu0TargetHandler::addFiles(InputFiles &f) {
  _gotFile.addAtom(*new (_gotFile._alloc) GLOBAL_OFFSET_TABLEAtom(_gotFile));
  _gotFile.addAtom(*new (_gotFile._alloc) TLSGETADDRAtom(_gotFile));
  f.appendFile(_gotFile);
}
