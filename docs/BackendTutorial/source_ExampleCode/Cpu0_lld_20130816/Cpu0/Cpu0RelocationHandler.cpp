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
#include "llvm/Object/ObjectFile.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/system_error.h"

using namespace lld;
using namespace elf;
using namespace llvm;
using namespace object;

static bool error(error_code ec) {
  if (!ec) return false;

  outs() << "Cpu0RelocationHandler.cpp : error reading file: " << ec.message() << ".\n";
  outs().flush();
  return true;
}

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

/// \brief R_CPU0_GOT16 - word32: S
void relocGOT16(uint8_t *location, uint64_t P, uint64_t S, int64_t A) {
  uint32_t result = (uint32_t)(S);
  *reinterpret_cast<llvm::support::ubig32_t *>(location) =
      result |
      (uint32_t) * reinterpret_cast<llvm::support::ubig32_t *>(location);
}

/// \brief R_CPU0_PC24 - word32: S + A - P
void relocPC24(uint8_t *location, uint64_t P, uint64_t S, int64_t A) {
  uint32_t result = (uint32_t)(S  - P);
  uint32_t machinecode = (uint32_t) * 
                         reinterpret_cast<llvm::support::ubig32_t *>(location);
  uint32_t opcode = (machinecode & 0xff000000);
  uint32_t offset = (machinecode & 0x00ffffff);
  *reinterpret_cast<llvm::support::ubig32_t *>(location) =
      (((result + offset) & 0x00ffffff) | opcode);
}

/// \brief R_CPU0_32 - word24:  S
void reloc24(uint8_t *location, uint64_t P, uint64_t S, int64_t A) {
  int32_t addr = (uint32_t)(S & 0x00ffffff);
  uint32_t machinecode = (uint32_t) * 
                         reinterpret_cast<llvm::support::ubig32_t *>(location);
  uint32_t opcode = (machinecode & 0xff000000);
  *reinterpret_cast<llvm::support::ubig32_t *>(location) =
      (opcode | addr);
  // TODO: Make sure that the result zero extends to the 64bit value.
}

/// \brief R_CPU0_32 - word32:  S
void reloc32(uint8_t *location, uint64_t P, uint64_t S, int64_t A) {
  int32_t result = (uint32_t)(S);
  *reinterpret_cast<llvm::support::ubig32_t *>(location) =
      result |
      (uint32_t) * reinterpret_cast<llvm::support::ubig32_t *>(location);
  // TODO: Make sure that the result zero extends to the 64bit value.
}

void relocPlt(uint8_t *location, uint64_t P, uint64_t S, int64_t A) {
//  int32_t result = (uint32_t)(S + A);
  int32_t result = (uint32_t)(S & 0xffff);
  uint32_t tmp = (uint32_t) * reinterpret_cast<llvm::support::ubig32_t *>
                 (location) & 0xffff0000;
  *reinterpret_cast<llvm::support::ubig32_t *>(location) =
      (result |
      tmp);
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

#if 1
struct Cpu0SoPlt {
  uint32_t funAddr[100];
  int funAddrSize = 0;
  struct dynsym {
    uint32_t stridx;
  };
  dynsym Dynsym[100];
  int DynsymSize;
  uint8_t Dynstr[1000];
  int DynstrSize;
  int create(ObjectFile *o) {
    error_code ec;
    for (section_iterator si = o->begin_sections(),
                          se = o->end_sections();
                          si != se; si.increment(ec)) {
      if (ec) return 1;
      StringRef Name;
      StringRef Contents;
      uint64_t BaseAddr;
      if (error(si->getName(Name))) continue;
      if (error(si->getContents(Contents))) continue;
      if (error(si->getAddress(BaseAddr))) continue;
      if (Name == ".dynsym") {
        // Dump out the content as hex and printable ascii characters.
        for (std::size_t addr = 0, end = Contents.size(); addr < end; addr += 16) {
          Dynsym[DynsymSize].stridx = *reinterpret_cast<llvm::support::ubig32_t*>((uint32_t*)(Contents.begin()+addr));
        }
      }
      if (Name == ".dynstr") {
        memcpy(Dynstr, Contents.begin(), Contents.size());
        DynstrSize = Contents.size();
      }
    }

    return 0;
  }
  // Return dynsym entry number
  int getDynsymEntryIdxByTargetAddr(uint64_t fAddr, 
      uint32_t *funAddr, int funAddrSize) {
    for (int i = 0; i < funAddrSize; i++) {
      if (fAddr == funAddr[i]) {
        return i;
      }
    }
    return -1;
  }
  // 0: fail to find index by name
  // > 0: get the dynsym index
  int getDynsymIdxByName(uint8_t *name) {
    for (int idx = 1, i = 1; i < DynstrSize; idx++) {
      if (strcmp((char*)name, (char*)Dynstr+i) == 0) {
        return idx;
      }
      i = i + strlen((char*)(Dynstr+i)) + 1;
    }
    return 0;
  }
};

Cpu0SoPlt cpu0SoPlt;

struct Cpu0ExePlt {
  struct dynsym {
    uint32_t stridx;
  };
  dynsym Dynsym[100];
  int DynsymSize;
  uint8_t Dynstr[1000];
  int DynstrSize;
  int create(const Cpu0LinkingContext &context, llvm::FileOutputBuffer &buf) {
    auto dynsymSection = context.getTargetHandler<Cpu0ELFType>().targetLayout().findOutputSection(".dynsym");
    uint64_t dynsymFileOffset, dynsymSizeOfBytes;
    if (dynsymSection) {
      dynsymFileOffset = dynsymSection->fileOffset();
      dynsymSizeOfBytes = dynsymSection->memSize();
      uint8_t *atomContent = buf.getBufferStart() + dynsymFileOffset;
      for (uint64_t i = 0; i < dynsymSizeOfBytes; i += 16) {
        Dynsym[DynsymSize].stridx = *reinterpret_cast<llvm::support::ubig32_t*>((uint32_t*)(atomContent + i));
        DynsymSize++;
      }
    }
    else
      return 1;
    auto dynstrSection = context.getTargetHandler<Cpu0ELFType>().targetLayout().findOutputSection(".dynstr");
    uint64_t dynstrFileOffset, dynstrSize;
    if (dynstrSection) {
      dynstrFileOffset = dynstrSection->fileOffset();
      dynstrSize = dynstrSection->memSize();
      uint8_t *atomContent = buf.getBufferStart() + dynstrFileOffset;
      memcpy(Dynstr, atomContent, dynstrSize);
      DynstrSize = dynstrSize;
    }
    else
      return 1;

    return 0;
  }
};

Cpu0ExePlt cpu0ExePlt;
#endif

ErrorOr<void> Cpu0TargetRelocationHandler::applyRelocation(
    ELFWriter &writer, llvm::FileOutputBuffer &buf, const lld::AtomLayout &atom,
    const Reference &ref) const {
  static bool firstTime = true;
  std::string soName("libfoobar.cpu0.so");
  bool find = false;
  int idx = 0;
  if (firstTime) {
    if (_context.getOutputType() == llvm::ELF::ET_DYN) {
      auto dynsymSection = _context.getTargetHandler<Cpu0ELFType>().targetLayout().findOutputSection(".dynsym");
      uint64_t dynsymFileOffset, dynsymSize;
      if (dynsymSection) {
        dynsymFileOffset = dynsymSection->fileOffset();
        dynsymSize = dynsymSection->memSize();
        uint8_t *atomContent = buf.getBufferStart() + dynsymFileOffset;
        for (uint64_t i = 4; i < dynsymSize; i += 16) {
          cpu0SoPlt.funAddr[cpu0SoPlt.funAddrSize] = *reinterpret_cast<llvm::support::ubig32_t*>((uint32_t*)(atomContent + i));
          cpu0SoPlt.funAddrSize++;
        }
      }
    }
    else if (_context.getOutputType() == llvm::ELF::ET_EXEC && !_context.isStaticExecutable()) {
#if 1
      auto dynsymSection = _context.getTargetHandler<Cpu0ELFType>().targetLayout().findOutputSection(".dynsym");
      uint64_t dynsymFileOffset, dynsymSize;
      if (dynsymSection) {
        dynsymFileOffset = dynsymSection->fileOffset();
        dynsymSize = dynsymSection->memSize();
        uint8_t *atomContent = buf.getBufferStart() + dynsymFileOffset;
        for (uint64_t i = 0; i < dynsymSize; i += 16) {
          cpu0SoPlt.funAddr[cpu0SoPlt.funAddrSize] = *reinterpret_cast<llvm::support::ubig32_t*>((uint32_t*)(atomContent + i));
          cpu0SoPlt.funAddrSize++;
        }
      }
#endif
      // Attempt to open the binary.
      OwningPtr<Binary> binary;
      if (error_code ec = createBinary(soName, binary)) {
        errs() << "Input file " << soName << " cannot open" << ec.message() << ".\n";
        return make_error_code(llvm::errc::executable_format_error);
      }
      if (ObjectFile *o = dyn_cast<ObjectFile>(binary.get()))
      // Create .so (share library Plt) to use for case LLD_R_CPU0_GOTRELINDEX.
        cpu0SoPlt.create(o);
      int success = cpu0ExePlt.create(_context, buf);
      assert(success == 0 && "cpu0ExePlt.create() fail\n");
    }
    firstTime = false;
  }
  uint8_t *atomContent = buf.getBufferStart() + atom._fileOffset;
  uint8_t *location = atomContent + ref.offsetInAtom();
  uint64_t targetVAddress = writer.addressOfAtom(ref.target());
  uint64_t relocVAddress = atom._virtualAddr + ref.offsetInAtom();
//  auto gotAtomIter = _context.getTargetHandler<Cpu0ELFType>().targetLayout().findAbsoluteAtom("_GLOBAL_OFFSET_TABLE_");
//  uint64_t globalOffsetTableAddress = writer.addressOfAtom(*gotAtomIter);
// .got.plt start from _GLOBAL_OFFSET_TABLE_
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
  case R_CPU0_GOT16:
#if 1
    idx = cpu0SoPlt.getDynsymEntryIdxByTargetAddr(targetVAddress, cpu0SoPlt.funAddr, cpu0SoPlt.funAddrSize);
    relocGOT16(location, relocVAddress, idx, ref.addend());
#else
    relocGOT16(location, relocVAddress, (targetVAddress - gotPltFileOffset), ref.addend());
#endif
    break;
  case R_CPU0_PC24:
    relocPC24(location, relocVAddress, targetVAddress, ref.addend());
    break;
#if 1
  case R_CPU0_CALL16:
  // offset at _GLOBAL_OFFSET_TABLE_ and $gp point to _GLOBAL_OFFSET_TABLE_.
#if 1
    idx = cpu0SoPlt.getDynsymEntryIdxByTargetAddr(targetVAddress, cpu0SoPlt.funAddr, cpu0SoPlt.funAddrSize);
#endif
    reloc32(location, relocVAddress, idx*0x04+16, ref.addend());
    break;
#endif
  case R_CPU0_24:
    reloc24(location, relocVAddress, targetVAddress, ref.addend());
    break;
  case R_CPU0_32:
    reloc32(location, relocVAddress, targetVAddress, ref.addend());
    break;

  case LLD_R_CPU0_GOTRELINDEX: {
    const DefinedAtom *target = cast<const DefinedAtom>(ref.target());
    for (const Reference *r : *target) {
      if (r->kind() == R_CPU0_JUMP_SLOT) {
        uint32_t index;
        if (!_context.getTargetHandler<Cpu0ELFType>().targetLayout()
                .getPLTRelocationTable()->getRelocationIndex(*r, index))
          llvm_unreachable("Relocation doesn't exist");
      // index: the entry number of PLT
      // index: 1st entry is 1, 2nd is 2, 3rd is 3, ...
//        reloc32(location, 0, index+1, 0);
        uint32_t stridx = cpu0ExePlt.Dynsym[index+1].stridx;
        uint8_t* dynstr = cpu0ExePlt.Dynstr+stridx;
        index = (uint32_t)cpu0SoPlt.getDynsymIdxByName(dynstr);
        relocPlt(location, 0, index, 0);
        break;
      }
    }
    break;
  }
  // Runtime only relocations. Ignore here.
  case R_CPU0_JUMP_SLOT:
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
