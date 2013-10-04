//===- lib/ReaderWriter/ELF/Cpu0/Cpu0RelocationHandler.cpp ------------===//
//
//                             The LLVM Linker
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "Cpu0TargetHandler.h"
#include "Cpu0LinkingContext.h"

using namespace lld;
using namespace elf;

namespace {
/// \brief R_CPU0_HI16 - word64: (S + A) >> 16
void relocHI16(uint8_t *location, uint64_t P, uint64_t S, int64_t A) {
//  uint32_t result = (uint32_t)((S + A) >> 16); // Don't know why ref.addend() = 9
  uint32_t result = (uint32_t)(S >> 16);
  *reinterpret_cast<llvm::support::ubig32_t *>(location) =
      result |
      (uint32_t) * reinterpret_cast<llvm::support::ubig32_t *>(location);
}

void relocLO16(uint8_t *location, uint64_t P, uint64_t S, uint64_t A) {
//  uint32_t result = (uint32_t)((S + A) & 0x0000ffff);
  uint32_t result = (uint32_t)(S & 0x0000ffff);
  *reinterpret_cast<llvm::support::ubig32_t *>(location) =
      result |
      (uint32_t) * reinterpret_cast<llvm::support::ubig32_t *>(location);
}

/// \brief R_CPU0_GOT_HI16 - word64: S >> 16
void relocGOTHI16(uint8_t *location, uint64_t P, uint64_t S, int64_t A) {
  uint32_t result = (uint32_t)(S >> 16);
  *reinterpret_cast<llvm::support::ubig32_t *>(location) =
      result |
      (uint32_t) * reinterpret_cast<llvm::support::ubig32_t *>(location);
}

/// \brief R_CPU0_PC24 - word32: S + A - P
void relocPC24(uint8_t *location, uint64_t P, uint64_t S, int64_t A) {
//  uint32_t result = (uint32_t)((S + A) - P);
  uint32_t result = (uint32_t)(S  - P);
  uint32_t machinecode = (uint32_t) * 
                         reinterpret_cast<llvm::support::ubig32_t *>(location);
  uint32_t opcode = (machinecode & 0xff000000);
  uint32_t offset = (machinecode & 0x00ffffff);
  *reinterpret_cast<llvm::support::ubig32_t *>(location) =
      ((result + offset) & 0x00ffffff | opcode);
}

/// \brief R_CPU0_32 - word32:  S + A
void reloc32(uint8_t *location, uint64_t P, uint64_t S, int64_t A) {
//  int32_t result = (uint32_t)(S + A);
  int32_t result = (uint32_t)(S);
  *reinterpret_cast<llvm::support::ubig32_t *>(location) =
      result |
      (uint32_t) * reinterpret_cast<llvm::support::ubig32_t *>(location);
  // TODO: Make sure that the result zero extends to the 64bit value.
}

} // end anon namespace

int64_t Cpu0TargetRelocationHandler::relocAddend(const Reference &ref) const {
  switch (ref.kind()) {
  case R_CPU0_PC24:
    return 4;
  default:
    return 0;
  }
  return 0;
}

ErrorOr<void> Cpu0TargetRelocationHandler::applyRelocation(
    ELFWriter &writer, llvm::FileOutputBuffer &buf, const lld::AtomLayout &atom,
    const Reference &ref) const {
  uint8_t *atomContent = buf.getBufferStart() + atom._fileOffset;
  uint8_t *location = atomContent + ref.offsetInAtom();
  uint64_t targetVAddress = writer.addressOfAtom(ref.target());
  uint64_t relocVAddress = atom._virtualAddr + ref.offsetInAtom();
//  auto gotAtomIter = _context.getTargetHandler<Cpu0ELFType>().targetLayout().findAbsoluteAtom("_GLOBAL_OFFSET_TABLE_");
//  uint64_t globalOffsetTableAddress = writer.addressOfAtom(*gotAtomIter);
  auto gotpltSection = _context.getTargetHandler<Cpu0ELFType>().targetLayout().findOutputSection(".got.plt");
  uint64_t gotPltFileOffset;
  if (gotpltSection)
    gotPltFileOffset = gotpltSection->fileOffset();
  else
    gotPltFileOffset = 0;

  switch (ref.kind()) {
  case R_CPU0_NONE:
    break;
  case R_CPU0_HI16:
    relocHI16(location, relocVAddress, targetVAddress, ref.addend());
    break;
  case R_CPU0_LO16:
    relocLO16(location, relocVAddress, targetVAddress, ref.addend());
    break;
  case R_CPU0_GOT_HI16:
    relocGOTHI16(location, gotPltFileOffset, targetVAddress, ref.addend());
    break;
  case R_CPU0_PC24:
    relocPC24(location, relocVAddress, targetVAddress, ref.addend());
    break;
#if 1
  case R_CPU0_CALL24:
    relocPC24(location, relocVAddress, targetVAddress, ref.addend());
    break;
#endif
  case R_CPU0_32:
    reloc32(location, relocVAddress, targetVAddress, ref.addend());
    break;

  case lld::Reference::kindLayoutAfter:
  case lld::Reference::kindLayoutBefore:
  case lld::Reference::kindInGroup:
    break;

  default: {
    std::string str;
    llvm::raw_string_ostream s(str);
    auto name = _context.stringFromRelocKind(ref.kind());
    s << "Unhandled relocation: " << atom._atom->file().path() << ":"
      << atom._atom->name() << "@" << ref.offsetInAtom() << " "
      << (name ? *name : "<unknown>") << " (" << ref.kind() << ")";
    s.flush();
    llvm_unreachable(str.c_str());
  }
  }

  return error_code::success();
}
