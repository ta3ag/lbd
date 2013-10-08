//===---------------------------- elf2hex.cpp -----------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This program is a utility that works with llvm-objdump.
//
//===----------------------------------------------------------------------===//

static cl::opt<bool>
ConvertElf2Hex("elf2hex", 
cl::desc("Display the hex content of verilog cpu0 needed sections"));

static cl::opt<bool>
DumpSo("dumpso", 
cl::desc("Dump shared library .so"));

// Modified from PrintSectionHeaders()
static uint64_t GetSectionHeaderStartAddress(const ObjectFile *o, 
  StringRef sectionName) {
//  outs() << "Sections:\n"
//            "Idx Name          Size      Address          Type\n";
  error_code ec;
  unsigned i = 0;
  for (section_iterator si = o->begin_sections(), se = o->end_sections();
                                                  si != se; si.increment(ec)) {
    if (error(ec)) return 0;
    StringRef Name;
    if (error(si->getName(Name))) return 0;
    uint64_t Address;
    if (error(si->getAddress(Address))) return 0;
    uint64_t Size;
    if (error(si->getSize(Size))) return 0;
    bool Text, Data, BSS;
    if (error(si->isText(Text))) return 0;
    if (error(si->isData(Data))) return 0;
    if (error(si->isBSS(BSS))) return 0;
    if (Name == sectionName)
      return Address;
    else
      return 0;
    ++i;
  }
  return 0;
}

// Modified from PrintSymbolTable()
static void GetSymbolTableStartAddress(const ObjectFile *o, StringRef sectionName) {
  outs() << "SYMBOL TABLE:\n";

  if (const COFFObjectFile *coff = dyn_cast<const COFFObjectFile>(o))
    PrintCOFFSymbolTable(coff);
  else {
    error_code ec;
    for (symbol_iterator si = o->begin_symbols(),
                         se = o->end_symbols(); si != se; si.increment(ec)) {
      if (error(ec)) return;
      StringRef Name;
      uint64_t Address;
      SymbolRef::Type Type;
      uint64_t Size;
      uint32_t Flags;
      section_iterator Section = o->end_sections();
      if (error(si->getName(Name))) continue;
      if (error(si->getAddress(Address))) continue;
      if (error(si->getFlags(Flags))) continue;
      if (error(si->getType(Type))) continue;
      if (error(si->getSize(Size))) continue;
      if (error(si->getSection(Section))) continue;

      bool Global = Flags & SymbolRef::SF_Global;
      bool Weak = Flags & SymbolRef::SF_Weak;
      bool Absolute = Flags & SymbolRef::SF_Absolute;

      if (Address == UnknownAddressOrSize)
        Address = 0;
      if (Size == UnknownAddressOrSize)
        Size = 0;
      char GlobLoc = ' ';
      if (Type != SymbolRef::ST_Unknown)
        GlobLoc = Global ? 'g' : 'l';
      char Debug = (Type == SymbolRef::ST_Debug || Type == SymbolRef::ST_File)
                   ? 'd' : ' ';
      char FileFunc = ' ';
      if (Type == SymbolRef::ST_File)
        FileFunc = 'f';
      else if (Type == SymbolRef::ST_Function)
        FileFunc = 'F';

      const char *Fmt = o->getBytesInAddress() > 4 ? "%016" PRIx64 :
                                                     "%08" PRIx64;

      outs() << format(Fmt, Address) << " "
             << GlobLoc // Local -> 'l', Global -> 'g', Neither -> ' '
             << (Weak ? 'w' : ' ') // Weak?
             << ' ' // Constructor. Not supported yet.
             << ' ' // Warning. Not supported yet.
             << ' ' // Indirect reference to another symbol.
             << Debug // Debugging (d) or dynamic (D) symbol.
             << FileFunc // Name of function (F), file (f) or object (O).
             << ' ';
      if (Absolute)
        outs() << "*ABS*";
      else if (Section == o->end_sections())
        outs() << "*UND*";
      else {
        if (const MachOObjectFile *MachO =
            dyn_cast<const MachOObjectFile>(o)) {
          DataRefImpl DR = Section->getRawDataRefImpl();
          StringRef SegmentName = MachO->getSectionFinalSegmentName(DR);
          outs() << SegmentName << ",";
        }
        StringRef SectionName;
        if (error(Section->getName(SectionName)))
          SectionName = "";
        outs() << SectionName;
      }
      outs() << '\t'
             << format("%08" PRIx64 " ", Size)
             << Name
             << '\n';
    }
  }
}

// Modified from DisassembleObject()
static void DisassembleObjectForHex(const ObjectFile *Obj/*, bool InlineRelocs*/
  , uint64_t& lastAddr) {
  std::string Error;
  raw_fd_ostream fd_so_func_offset("so_func_offset", Error);

  const Target *TheTarget = getTarget(Obj);
  // getTarget() will have already issued a diagnostic if necessary, so
  // just bail here if it failed.
  if (!TheTarget)
    return;

  // Package up features to be passed to target/subtarget
  std::string FeaturesStr;
  if (MAttrs.size()) {
    SubtargetFeatures Features;
    for (unsigned i = 0; i != MAttrs.size(); ++i)
      Features.AddFeature(MAttrs[i]);
    FeaturesStr = Features.getString();
  }

  OwningPtr<const MCRegisterInfo> MRI(TheTarget->createMCRegInfo(TripleName));
  if (!MRI) {
    errs() << "error: no register info for target " << TripleName << "\n";
    return;
  }

  // Set up disassembler.
  OwningPtr<const MCAsmInfo> AsmInfo(
    TheTarget->createMCAsmInfo(*MRI, TripleName));
  if (!AsmInfo) {
    errs() << "error: no assembly info for target " << TripleName << "\n";
    return;
  }

  OwningPtr<const MCSubtargetInfo> STI(
    TheTarget->createMCSubtargetInfo(TripleName, "", FeaturesStr));
  if (!STI) {
    errs() << "error: no subtarget info for target " << TripleName << "\n";
    return;
  }

  OwningPtr<const MCInstrInfo> MII(TheTarget->createMCInstrInfo());
  if (!MII) {
    errs() << "error: no instruction info for target " << TripleName << "\n";
    return;
  }

  OwningPtr<MCDisassembler> DisAsm(TheTarget->createMCDisassembler(*STI));
  if (!DisAsm) {
    errs() << "error: no disassembler for target " << TripleName << "\n";
    return;
  }

  OwningPtr<const MCObjectFileInfo> MOFI;
  OwningPtr<MCContext> Ctx;

  if (Symbolize) {
    MOFI.reset(new MCObjectFileInfo);
    Ctx.reset(new MCContext(AsmInfo.get(), MRI.get(), MOFI.get()));
    OwningPtr<MCRelocationInfo> RelInfo(
      TheTarget->createMCRelocationInfo(TripleName, *Ctx.get()));
    if (RelInfo) {
      OwningPtr<MCSymbolizer> Symzer(
        MCObjectSymbolizer::createObjectSymbolizer(*Ctx.get(), RelInfo, Obj));
      if (Symzer)
        DisAsm->setSymbolizer(Symzer);
    }
  }

  OwningPtr<const MCInstrAnalysis>
    MIA(TheTarget->createMCInstrAnalysis(MII.get()));

  int AsmPrinterVariant = AsmInfo->getAssemblerDialect();
  OwningPtr<MCInstPrinter> IP(TheTarget->createMCInstPrinter(
      AsmPrinterVariant, *AsmInfo, *MII, *MRI, *STI));
  if (!IP) {
    errs() << "error: no instruction printer for target " << TripleName
      << '\n';
    return;
  }

  if (CFG) {
    OwningPtr<MCObjectDisassembler> OD(
      new MCObjectDisassembler(*Obj, *DisAsm, *MIA));
    OwningPtr<MCModule> Mod(OD->buildModule(/* withCFG */ true));
    for (MCModule::const_atom_iterator AI = Mod->atom_begin(),
                                       AE = Mod->atom_end();
                                       AI != AE; ++AI) {
      outs() << "Atom " << (*AI)->getName() << ": \n";
      if (const MCTextAtom *TA = dyn_cast<MCTextAtom>(*AI)) {
        for (MCTextAtom::const_iterator II = TA->begin(), IE = TA->end();
             II != IE;
             ++II) {
          IP->printInst(&II->Inst, outs(), "");
          outs() << "\n";
        }
      }
    }
    for (MCModule::const_func_iterator FI = Mod->func_begin(),
                                       FE = Mod->func_end();
                                       FI != FE; ++FI) {
      static int filenum = 0;
      emitDOTFile((Twine((*FI)->getName()) + "_" +
                   utostr(filenum) + ".dot").str().c_str(),
                    **FI, IP.get());
      ++filenum;
    }
  }


  error_code ec;
  for (section_iterator i = Obj->begin_sections(),
                        e = Obj->end_sections();
                        i != e; i.increment(ec)) {
    if (error(ec)) break;
    bool text;
    if (error(i->isText(text))) break;
    if (!text) continue;

    uint64_t SectionAddr;
    if (error(i->getAddress(SectionAddr))) break;

    // Make a list of all the symbols in this section.
    std::vector<std::pair<uint64_t, StringRef> > Symbols;
    for (symbol_iterator si = Obj->begin_symbols(),
                         se = Obj->end_symbols();
                         si != se; si.increment(ec)) {
      bool contains;
      if (!error(i->containsSymbol(*si, contains)) && contains) {
        uint64_t Address;
        if (error(si->getAddress(Address))) break;
        if (Address == UnknownAddressOrSize) continue;
        Address -= SectionAddr;

        StringRef Name;
        if (error(si->getName(Name))) break;
        Symbols.push_back(std::make_pair(Address, Name));
      }
    }

    // Sort the symbols by address, just in case they didn't come in that way.
    array_pod_sort(Symbols.begin(), Symbols.end());

    // Make a list of all the relocations for this section.
    std::vector<RelocationRef> Rels;
/*    if (InlineRelocs) {
      for (relocation_iterator ri = i->begin_relocations(),
                               re = i->end_relocations();
                               ri != re; ri.increment(ec)) {
        if (error(ec)) break;
        Rels.push_back(*ri);
      }
    }*/

    // Sort relocations by address.
    std::sort(Rels.begin(), Rels.end(), RelocAddressLess);

    StringRef SegmentName = "";
    if (const MachOObjectFile *MachO =
        dyn_cast<const MachOObjectFile>(Obj)) {
      DataRefImpl DR = i->getRawDataRefImpl();
      SegmentName = MachO->getSectionFinalSegmentName(DR);
    }
    StringRef name;
    if (error(i->getName(name))) break;
    outs() << "/*" << "Disassembly of section ";
    if (!SegmentName.empty())
      outs() << SegmentName << ",";
    outs() << name << ':' << "*/";

    // If the section has no symbols just insert a dummy one and disassemble
    // the whole section.
    if (Symbols.empty())
      Symbols.push_back(std::make_pair(0, name));


    SmallString<40> Comments;
    raw_svector_ostream CommentStream(Comments);

    StringRef Bytes;
    if (error(i->getContents(Bytes))) break;
    StringRefMemoryObject memoryObject(Bytes, SectionAddr);
    uint64_t Size;
    uint64_t Index;
    uint64_t SectSize;
    if (error(i->getSize(SectSize))) break;

    std::vector<RelocationRef>::const_iterator rel_cur = Rels.begin();
    std::vector<RelocationRef>::const_iterator rel_end = Rels.end();
    // Disassemble symbol by symbol.
    for (unsigned si = 0, se = Symbols.size(); si != se; ++si) {
      uint64_t Start = Symbols[si].first;
      uint64_t End;
      // The end is either the size of the section or the beginning of the next
      // symbol.
      if (si == se - 1)
        End = SectSize;
      // Make sure this symbol takes up space.
      else if (Symbols[si + 1].first != Start)
        End = Symbols[si + 1].first - 1;
      else
        // This symbol has the same address as the next symbol. Skip it.
        continue;

      if (DumpSo) {
        fd_so_func_offset << format("%08" PRIx64 "    ", Symbols[si].first);
        std::string str = Symbols[si].second.str();
        std::size_t idx = 0;
        std::size_t strSize = 0;
        for (idx = 0, strSize = str.size(); idx < strSize; idx++) {
          fd_so_func_offset << hexdigit((str[idx] >> 4) & 0xF, true)
                            << hexdigit(str[idx] & 0xF, true) << " ";
        }
        for (idx = strSize; idx < 48; idx++) {
          fd_so_func_offset << format("%02" PRIx64 " ", 0);
        }
        fd_so_func_offset << "/* " << Symbols[si].second << " */";
        fd_so_func_offset << "\n";
      }

      outs() << '\n' << "/*" << Symbols[si].second << ":*/\n";

#ifndef NDEBUG
        raw_ostream &DebugOut = DebugFlag ? dbgs() : nulls();
#else
        raw_ostream &DebugOut = nulls();
#endif

      for (Index = Start; Index < End; Index += Size) {
        MCInst Inst;

        if (DisAsm->getInstruction(Inst, Size, memoryObject,
                                   SectionAddr + Index,
                                   DebugOut, CommentStream)) {
          outs() << format("/*%8" PRIx64 ":*/", /*SectionAddr + */Index);
          if (!NoShowRawInsn) {
            outs() << "\t";
            DumpBytes(StringRef(Bytes.data() + Index, Size));
          }
          outs() << "/*";
          IP->printInst(&Inst, outs(), "");
          outs() << CommentStream.str();
          outs() << "*/";
          Comments.clear();
          outs() << "\n";
        } else {
          errs() << ToolName << ": warning: invalid instruction encoding\n";
          if (Size == 0)
            Size = 1; // skip illegible bytes
        }

        // Print relocation for instruction.
        while (rel_cur != rel_end) {
          bool hidden = false;
          uint64_t addr;
          SmallString<16> name;
          SmallString<32> val;

          // If this relocation is hidden, skip it.
          if (error(rel_cur->getHidden(hidden))) goto skip_print_rel;
          if (hidden) goto skip_print_rel;

          if (error(rel_cur->getOffset(addr))) goto skip_print_rel;
          // Stop when rel_cur's address is past the current instruction.
          if (addr >= Index + Size) break;
          if (error(rel_cur->getTypeName(name))) goto skip_print_rel;
          if (error(rel_cur->getValueString(val))) goto skip_print_rel;

          outs() << format("\t\t\t/*%8" PRIx64 ": ", SectionAddr + addr) << name
                 << "\t" << val << "*/\n";

        skip_print_rel:
          ++rel_cur;
        }
      }
      lastAddr = Index;
    }
  }
}

#define DYNSYM_LIB_OFFSET 9
/*
struct DynsymEntry {
  uint32_t DynstrFunOffset;
  uint32_t DynstrLibOffset;
};*/

// Modified from PrintSectionContents()
static void PrintDataSections(const ObjectFile *o, uint64_t lastAddr) {
  error_code ec;
  std::size_t addr, end;
  std::string Error;

  for (section_iterator si = o->begin_sections(),
                        se = o->end_sections();
                        si != se; si.increment(ec)) {
    if (error(ec)) return;
    StringRef Name;
    StringRef Contents;
    uint64_t BaseAddr;
    bool BSS;
    if (error(si->getName(Name))) continue;
    if (error(si->getContents(Contents))) continue;
    if (error(si->getAddress(BaseAddr))) continue;
    if (error(si->isBSS(BSS))) continue;

    if (Name == ".rodata" || Name == ".rodata1" || Name == ".data" || 
      Name == ".data1" || Name == ".sdata") {
      if (Contents.size() <= 0) {
        continue;
      }
      // Fill /*address*/ 00 00 00 00 between lastAddr and BaseAddr
      for (addr = lastAddr, end = BaseAddr; addr < end; addr += 4) {
        outs() << format("/*%04" PRIx64 " */", addr);
        outs() << format("%02" PRIx64 " ", 0) << format("%02" PRIx64 " ", 0) \
        << format("%02" PRIx64 " ", 0) << format("%02" PRIx64 " ", 0) << '\n';
      }

      outs() << "/*Contents of section " << Name << ":*/\n";
      // Dump out the content as hex and printable ascii characters.
      for (std::size_t addr = 0, end = Contents.size(); addr < end; addr += 16) {
        outs() << format("/*%04" PRIx64 " */", BaseAddr + addr);
        // Dump line of hex.
        for (std::size_t i = 0; i < 16; ++i) {
          if (i != 0 && i % 4 == 0)
            outs() << ' ';
          if (addr + i < end)
            outs() << hexdigit((Contents[addr + i] >> 4) & 0xF, true)
                   << hexdigit(Contents[addr + i] & 0xF, true) << " ";
          else
            outs() << "  ";
        }
        // save lastAddr
        if ((BaseAddr + addr + 16) > end) 
          lastAddr = BaseAddr + end;
        else
          lastAddr = BaseAddr + addr + 16;
        // Print ascii.
        outs() << "/*" << "  ";
        for (std::size_t i = 0; i < 16 && addr + i < end; ++i) {
          if (std::isprint(static_cast<unsigned char>(Contents[addr + i]) & 0xFF))
            outs() << Contents[addr + i];
          else
            outs() << ".";
        }
        outs() << "*/" << "\n";
      }
    }
    else if (!DumpSo) {
      if (Name == ".dynsym") {
        raw_fd_ostream fd_dynsym("dynsym", Error);
        for (std::size_t addr = 0, end = Contents.size(); addr < end; addr += 16) {
          fd_dynsym << hexdigit((Contents[addr] >> 4) & 0xF, true)
                     << hexdigit(Contents[addr] & 0xF, true) << " ";
          fd_dynsym << hexdigit((Contents[addr+1] >> 4) & 0xF, true)
                     << hexdigit(Contents[addr+1] & 0xF, true) << " ";
          fd_dynsym << hexdigit((Contents[addr+2] >> 4) & 0xF, true)
                     << hexdigit(Contents[addr+2] & 0xF, true) << " ";
          fd_dynsym << hexdigit((Contents[addr+3] >> 4) & 0xF, true)
                     << hexdigit(Contents[addr+3] & 0xF, true) << " ";

          fd_dynsym << hexdigit((Contents[addr+DYNSYM_LIB_OFFSET] >> 4) & 0xF, true)
                     << hexdigit(Contents[addr+DYNSYM_LIB_OFFSET] & 0xF, true) << " ";
          fd_dynsym << hexdigit((Contents[addr+DYNSYM_LIB_OFFSET+1] >> 4) & 0xF, true)
                     << hexdigit(Contents[addr+DYNSYM_LIB_OFFSET+1] & 0xF, true) << " ";
          fd_dynsym << hexdigit((Contents[addr+DYNSYM_LIB_OFFSET+2] >> 4) & 0xF, true)
                     << hexdigit(Contents[addr+DYNSYM_LIB_OFFSET+2] & 0xF, true) << " ";
          fd_dynsym << hexdigit((Contents[addr+DYNSYM_LIB_OFFSET+3] >> 4) & 0xF, true)
                     << hexdigit(Contents[addr+DYNSYM_LIB_OFFSET+3] & 0xF, true) << " ";
        }
      }
      else if (Name == ".dynstr") {
        raw_fd_ostream fd_dynstr("dynstr", Error);
        for (std::size_t addr = 0, end = Contents.size(); addr < end; addr++) {
          fd_dynstr << hexdigit((Contents[addr] >> 4) & 0xF, true)
                     << hexdigit(Contents[addr] & 0xF, true) << " ";
        }
      }
    }
  }
}

static void Elf2Hex(const ObjectFile *o) {
  uint64_t startAddr = GetSectionHeaderStartAddress(o, "_start");
//  outs() << format("_start address:%08" PRIx64 "\n", startAddr);
  uint64_t lastAddr;
  DisassembleObjectForHex(o, lastAddr);
//  outs() << format("lastAddr:%08" PRIx64 "\n", lastAddr);
  PrintDataSections(o, lastAddr);
}
