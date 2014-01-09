//===- lib/ReaderWriter/ELF/Cpu0/Cpu0RelocationHandler.h
//------------------===//
//
//                             The LLVM Linker
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef LLD_READER_WRITER_ELF_CPU0_RELOCATION_HANDLER_H
#define LLD_READER_WRITER_ELF_CPU0_RELOCATION_HANDLER_H

#include "Cpu0TargetHandler.h"

namespace lld {
namespace elf {
typedef llvm::object::ELFType<llvm::support::big, 4, false> Cpu0ELFType;
class Cpu0LinkingContext;

class Cpu0TargetRelocationHandler LLVM_FINAL
    : public TargetRelocationHandler<Cpu0ELFType> {
public:
  Cpu0TargetRelocationHandler(const Cpu0LinkingContext &context)
      : _tlsSize(0), _context(context) {}

  virtual error_code applyRelocation(ELFWriter &, llvm::FileOutputBuffer &,
                                        const lld::AtomLayout &,
                                        const Reference &) const;

  virtual int64_t relocAddend(const Reference &) const;

private:
  // Cached size of the TLS segment.
  mutable uint64_t _tlsSize;
  const Cpu0LinkingContext &_context;
};

} // end namespace elf
} // end namespace lld

#endif
