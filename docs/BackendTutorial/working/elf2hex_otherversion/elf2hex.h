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

#include <stdio.h>

static cl::opt<bool>
ConvertElf2Hex("elf2hex", 
cl::desc("Display the hex content of verilog cpu0 needed sections"));

static cl::opt<bool>
DumpSo("cpu0dumpso", 
cl::desc("Dump shared library .so"));

static cl::opt<bool>
LinkSo("cpu0linkso", 
cl::desc("Link shared library .so"));

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

class Cpu0DynFunIndex {
private:
  char soStrtab[20][100];
  int soStrtabSize = 0;

  char exePltName[20][100];
  int exePltNameSize = 0;

  int findPltName(const char* pltName);
public:
  void createPltName(const ObjectFile *o);
  void createStrtab();
  uint16_t correctDynFunIndex(const char* pltName);
};

int Cpu0DynFunIndex::findPltName(const char* pltName) {
  for (int i = 0; i < exePltNameSize; i++)
    if (strcmp(pltName, exePltName[i]) == 0)
      return i;
  return -1;
}

void Cpu0DynFunIndex::createPltName(const ObjectFile *o) {
  error_code ec;
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

    if (Name == ".strtab") {
      int num_dyn_entry = 0;
      FILE *fd_num_dyn_entry;
      fd_num_dyn_entry = fopen("num_dyn_entry", "r");
      if (fd_num_dyn_entry != NULL) {
        fscanf(fd_num_dyn_entry, "%d", &num_dyn_entry);
      }
      fclose(fd_num_dyn_entry);

      for (std::size_t addr = 2+strlen(".PLT0"), end = Contents.size(); 
           addr < end; ) {
        if (Contents.substr(addr, strlen("__plt_")) != "__plt_")
          break;
        strcpy(exePltName[exePltNameSize], Contents.data()+addr);
        addr = addr + strlen(exePltName[exePltNameSize]) + 1;
        exePltNameSize++;
      }
      break;
    }
  }
}

void Cpu0DynFunIndex::createStrtab() {
  FILE *fd_dynstrAscii;

  fd_dynstrAscii = fopen("dynstrAscii", "r");
  if (fd_dynstrAscii == NULL)
    fclose(fd_dynstrAscii);
  assert(fd_dynstrAscii != NULL && "fd_dynstr == NULL");
  int i = 0;
  // function                  result on EOF or error                    
  // --------                  ----------------------
  // fgets()                   NULL
  // fscanf()                  number of succesful conversions
  //                             less than expected
  // fgetc()                   EOF
  // fread()                   number of elements read
  //                             less than expected
  int j = 0;
  for (i=0; 1; i++) {
    j=fscanf(fd_dynstrAscii, "%s", soStrtab[i]);
    if (j != 1)
      break;
  }
  soStrtabSize = i;
  fclose(fd_dynstrAscii);
}

uint16_t Cpu0DynFunIndex::correctDynFunIndex(const char* pltName) {
  int i = findPltName(pltName);
  if (i != -1) {
    int j = 0;
    for (j=0; j < soStrtabSize; j++)
      if (strcmp(soStrtab[j], (const char*)exePltName[i]+strlen("__plt_")) == 0)
        break;
    if (j == soStrtabSize) {
      outs() << "cannot find " << exePltName[i] << "\n";
      exit(1);
    }
    j++;
    return (uint16_t)(j & 0xffff);
  }
  return (uint16_t)0;
}

Cpu0DynFunIndex cpu0DynFunIndex;

static void Fill0s(uint64_t& lastDumpAddr, uint64_t BaseAddr) {
  std::size_t addr, end;

  // Fill /*address*/ 00 00 00 00 between lastDumpAddr( = the address of last
  // end section + 1) and BaseAddr
  uint64_t cellingLastAddr4 = ((lastDumpAddr + 3) / 4) * 4;
  assert((lastDumpAddr <= BaseAddr) && "lastDumpAddr must <= BaseAddr");
  // Fill /*address*/ bytes is odd for 4 by 00 
  outs() << format("/*%8" PRIx64 " */\t", lastDumpAddr);
  if (cellingLastAddr4 > BaseAddr) {
    for (std::size_t i = lastDumpAddr; i < BaseAddr; ++i) {
      outs() << "00 ";
    }
    outs() << "\n";
    lastDumpAddr = BaseAddr;
  }
  else {
    for (std::size_t i = lastDumpAddr; i < cellingLastAddr4; ++i) {
      outs() << "00 ";
    }
    outs() << "\n";
    lastDumpAddr = cellingLastAddr4;
  }
  // Fill /*address*/ 00 00 00 00 for 4 bytes (1 Cpu0 word size)
  for (addr = lastDumpAddr, end = BaseAddr; addr < end; addr += 4) {
    outs() << format("/*%8" PRIx64 " */\t", addr);
    outs() << format("%02" PRIx64 " ", 0) << format("%02" PRIx64 " ", 0) \
    << format("%02" PRIx64 " ", 0) << format("%02" PRIx64 " ", 0) << '\n';
  }

  return;
}

static void PrintDataSection(const ObjectFile *o, uint64_t& lastDumpAddr, 
  section_iterator si) {
  std::string Error;
  std::size_t addr, end;
  StringRef Name;
  StringRef Contents;
  uint64_t BaseAddr;
  bool BSS;
  if (error(si->getName(Name))) return;
  if (error(si->getContents(Contents))) return;
  if (error(si->getAddress(BaseAddr))) return;
  if (error(si->isBSS(BSS))) return;

  if (Name == ".rodata" || Name == ".rodata1" || Name == ".data" || 
    Name == ".data1" || Name == ".sdata") {
    if (Contents.size() <= 0) {
      return;
    }
    Fill0s(lastDumpAddr, BaseAddr);

    outs() << "/*Contents of section " << Name << ":*/\n";
    // Dump out the content as hex and printable ascii characters.
    for (std::size_t addr = 0, end = Contents.size(); addr < end; addr += 16) {
      outs() << format("/*%8" PRIx64 " */", BaseAddr + addr);
      // Dump line of hex.
      for (std::size_t i = 0; i < 16; ++i) {
        if (i != 0 && i % 4 == 0)
          outs() << ' ';
        if (addr + i < end)
          outs() << hexdigit((Contents[addr + i] >> 4) & 0xF, true)
                 << hexdigit(Contents[addr + i] & 0xF, true) << " ";
      }
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
    // save the end address of this section to lastDumpAddr
    lastDumpAddr = BaseAddr + Contents.size();
  }
#if 1
  else if (Name == ".bss" || Name == ".sbss") {
    if (Contents.size() <= 0) {
      return;
    }
    // Fill /*address*/ 00 00 00 00 between lastDumpAddr( = the address of last
    // end section + 1) and BaseAddr
    uint64_t cellingLastAddr4 = ((lastDumpAddr + 3) / 4) * 4;
    assert((lastDumpAddr <= BaseAddr) && "lastDumpAddr must <= BaseAddr");
    // Fill /*address*/ bytes is odd for 4 by 00 
    outs() << format("/*%04" PRIx64 " */", lastDumpAddr);
    if (cellingLastAddr4 > BaseAddr) {
      for (std::size_t i = lastDumpAddr; i < BaseAddr; ++i) {
        outs() << "00 ";
      }
      outs() << "\n";
      lastDumpAddr = BaseAddr;
    }
    else {
      for (std::size_t i = lastDumpAddr; i < cellingLastAddr4; ++i) {
        outs() << "00 ";
      }
      outs() << "\n";
      lastDumpAddr = cellingLastAddr4;
    }
    // Fill /*address*/ 00 00 00 00 for 4 bytes (1 Cpu0 word size)
    for (addr = lastDumpAddr, end = BaseAddr; addr < end; addr += 4) {
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
          outs() << "00 ";
      }
      outs() << "\n";
    }
    // save the end address of this section to lastDumpAddr
    lastDumpAddr = BaseAddr + Contents.size();
  }
#endif
}

// Modified from DisassembleObject()
static void DisassembleObjectInHexFormat(const ObjectFile *Obj
/*, bool InlineRelocs*/  , StringRef secName, uint64_t& lastDumpAddr) {
  std::string Error;
  uint64_t soLastPrintAddr = 0;
  FILE *fd_so_func_offset;
  int num_dyn_entry = 0;
  if (DumpSo) {
    fd_so_func_offset = fopen("so_func_offset", "w");
    if (fd_so_func_offset == NULL)
      fclose(fd_so_func_offset);
    assert(fd_so_func_offset != NULL && "fd_so_func_offset == NULL");
  }
  if (LinkSo) {
    cpu0DynFunIndex.createStrtab();
  }

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
    if (!text) {
      if (!DumpSo)
        PrintDataSection(Obj, lastDumpAddr, i);
      continue;
    }
    StringRef name;
    if (error(i->getName(name))) break;
    if (name != secName) continue;

    uint64_t SectionAddr;
    if (error(i->getAddress(SectionAddr))) break;

    if (!DumpSo && lastDumpAddr != 0)
      Fill0s(lastDumpAddr, SectionAddr);

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
    // StringRef name;
    if (error(i->getName(name))) break;
    if (DumpSo && name == ".plt") continue;
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
      else {
        // This symbol has the same address as the next symbol. Skip it.
        if (DumpSo/* && Symbols[si].second != "__tls_get_addr"*/) {
          fprintf(fd_so_func_offset, "%02x ", 
                  (uint8_t)(Symbols[si].first >> 24));
          fprintf(fd_so_func_offset, "%02x ", 
                  (uint8_t)((Symbols[si].first >> 16) & 0xFF));
          fprintf(fd_so_func_offset, "%02x ", 
                  (uint8_t)((Symbols[si].first >> 8) & 0xFF));
          fprintf(fd_so_func_offset, "%02x    ", 
                  (uint8_t)((Symbols[si].first) & 0xFF));
          std::string str = Symbols[si].second.str();
          std::size_t idx = 0;
          std::size_t strSize = 0;
          for (idx = 0, strSize = str.size(); idx < strSize; idx++) {
            fprintf(fd_so_func_offset, "%c%c ", 
                    hexdigit((str[idx] >> 4) & 0xF, true),
                    hexdigit(str[idx] & 0xF, true));
          }
          for (idx = strSize; idx < 48; idx++) {
            fprintf(fd_so_func_offset, "%02x ", 0);
          }
          fprintf(fd_so_func_offset, "/* %s */\n", Symbols[si].second.begin());
          num_dyn_entry++;
        }

        outs() << '\n' << "/*" << Symbols[si].second << ":*/\n";
        continue;
      }

      if (DumpSo) {
        soLastPrintAddr = Symbols[si].first;
        fprintf(fd_so_func_offset, "%02x ", (uint8_t)(Symbols[si].first >> 24));
        fprintf(fd_so_func_offset, "%02x ", 
                (uint8_t)((Symbols[si].first >> 16) & 0xFF));
        fprintf(fd_so_func_offset, "%02x ", 
                (uint8_t)((Symbols[si].first >> 8) & 0xFF));
        fprintf(fd_so_func_offset, "%02x    ", 
                (uint8_t)((Symbols[si].first) & 0xFF));
        std::string str = Symbols[si].second.str();
        std::size_t idx = 0;
        std::size_t strSize = 0;
        for (idx = 0, strSize = str.size(); idx < strSize; idx++) {
          fprintf(fd_so_func_offset, "%c%c ", 
                  hexdigit((str[idx] >> 4) & 0xF, true), 
                  hexdigit(str[idx] & 0xF, true));
        }
        for (idx = strSize; idx < 48; idx++) {
          fprintf(fd_so_func_offset, "%02x ", 0);
        }
        fprintf(fd_so_func_offset, "/* %s */\n", Symbols[si].second.begin());
        num_dyn_entry++;
      }

      outs() << '\n' << "/*" << Symbols[si].second << ":*/\n";
      uint16_t funIndex = 0;
      if (LinkSo) {
      // correctDynFunIndex
        funIndex = cpu0DynFunIndex.correctDynFunIndex(Symbols[si].second.data());
      }

#ifndef NDEBUG
        raw_ostream &DebugOut = DebugFlag ? dbgs() : nulls();
#else
        raw_ostream &DebugOut = nulls();
#endif

      for (Index = Start; Index < End; Index += Size) {
        MCInst Inst;

        if (LinkSo && funIndex && Index == Start) {
          if (!DumpSo)
            outs() << format("/*%8" PRIx64 ":*/\t", SectionAddr + /*lastDumpAddr+*/Index);
          else
            outs() << format("/*%8" PRIx64 ":*/\t", /*SectionAddr + */lastDumpAddr+Index);
          outs() << "01 6b " << format("%02" PRIx64, (funIndex*4+16) & 0xff00)
                  << format(" %02" PRIx64, (funIndex*4+16) & 0x00ff);
          outs() << "                                  /* ld\t$t9, " 
                 << funIndex*4+16 << "($gp)\n";
        }
        else {
          if (DisAsm->getInstruction(Inst, Size, memoryObject,
                                     SectionAddr + Index,
                                     DebugOut, CommentStream)) {
            if (!DumpSo)
              outs() << format("/*%8" PRIx64 ":*/", SectionAddr + /*lastDumpAddr+*/Index);
            else
              outs() << format("/*%8" PRIx64 ":*/", /*SectionAddr + */lastDumpAddr+Index);
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
        }

        //  outs() << "Size = " << Size <<  "Index = " << Index << "lastDumpAddr = "
        //         << lastDumpAddr << "\n"; // debug
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
      if (DumpSo)
        soLastPrintAddr = End;
    }
    if (!DumpSo)
      lastDumpAddr = SectionAddr + Index;
    else
      lastDumpAddr += Index;
  }
  if (DumpSo) {
// Fix the issue that __tls_get_addr appear as file offset 0.
// Old lld version the __tls_get_addr appear at the last function name.
    std::pair<uint64_t, StringRef> dummy(soLastPrintAddr, "dummy");
    fprintf(fd_so_func_offset, "%02x ", (uint8_t)(dummy.first >> 24));
    fprintf(fd_so_func_offset, "%02x ", (uint8_t)((dummy.first >> 16) & 0xFF));
    fprintf(fd_so_func_offset, "%02x ", (uint8_t)((dummy.first >> 8) & 0xFF));
    fprintf(fd_so_func_offset, "%02x    ", (uint8_t)((dummy.first) & 0xFF));
    std::string str = dummy.second.str();
    std::size_t idx = 0;
    std::size_t strSize = 0;
    for (idx = 0, strSize = str.size(); idx < strSize; idx++) {
      fprintf(fd_so_func_offset, "%c%c ", hexdigit((str[idx] >> 4) & 0xF, true)
              , hexdigit(str[idx] & 0xF, true));
    }
    for (idx = strSize; idx < 48; idx++) {
      fprintf(fd_so_func_offset, "%02x ", 0);
    }
    fprintf(fd_so_func_offset, "/* %s */\n", dummy.second.begin());
    num_dyn_entry++;
    outs() << '\n' << "/*" << dummy.second << ":*/\n";
  }
  if (DumpSo) {
    FILE *fd_num_dyn_entry;
    fd_num_dyn_entry = fopen("num_dyn_entry", "w");
    if (fd_num_dyn_entry != NULL) {
      fprintf(fd_num_dyn_entry, "%d\n", num_dyn_entry);
    }
    fclose(fd_num_dyn_entry);
  }
}

#define DYNSYM_LIB_OFFSET 9

static void DisassemblePltSecInHexFormat(const ObjectFile *Obj, 
  OwningPtr<MCDisassembler> &DisAsm, OwningPtr<MCInstPrinter> &IP, 
  uint64_t secOffset, uint64_t secDistance, uint64_t& lastDumpAddr) {
  if (LinkSo) {
    cpu0DynFunIndex.createStrtab();
  }
  error_code ec;
  for (section_iterator i = Obj->begin_sections(),
                        e = Obj->end_sections();
                        i != e; i.increment(ec)) {
    if (error(ec)) break;
    bool text;
    if (error(i->isText(text))) break;
    if (!text) {
      continue;
    }
    StringRef name;
    if (error(i->getName(name))) break;
    if (name == ".plt") {
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

      // Sort relocations by address.
      std::sort(Rels.begin(), Rels.end(), RelocAddressLess);

      StringRef SegmentName = "";
      if (const MachOObjectFile *MachO =
          dyn_cast<const MachOObjectFile>(Obj)) {
        DataRefImpl DR = i->getRawDataRefImpl();
        SegmentName = MachO->getSectionFinalSegmentName(DR);
      }
      if (error(i->getName(name))) break;
      outs() << "/*" << "Disassembly of section ";
      if (!SegmentName.empty())
        outs() << SegmentName << ",";
      outs() << name << ':' << "*/\n";

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
        else {
          outs() << '\n' << "/*" << Symbols[si].second << ":*/\n";
          continue;
        }

        outs() << '\n' << "/*" << Symbols[si].second << ":*/\n";
        uint16_t funIndex = 0;
        if (LinkSo) {
        // correctDynFunIndex
          funIndex = cpu0DynFunIndex.correctDynFunIndex(Symbols[si].second.data());
        }

  #ifndef NDEBUG
          raw_ostream &DebugOut = DebugFlag ? dbgs() : nulls();
  #else
          raw_ostream &DebugOut = nulls();
  #endif

//        if (!DumpSo && lastDumpAddr != 0)
//          Fill0s(lastDumpAddr, SectionAddr);

        for (Index = Start; Index < End; Index += Size) {
          MCInst Inst;

          if (LinkSo && funIndex && Index == Start) {
            outs() << format("/*%8" PRIx64 ":*/\t", Index);
            outs() << "01 6b " << format("%02" PRIx64, (funIndex*4+16) & 0xff00)
                    << format(" %02" PRIx64, (funIndex*4+16) & 0x00ff);
            outs() << "                                  /* ld\t$t9, " 
                   << funIndex*4+16 << "($gp)\n";
          }
          else {
            if (Index == 0) {
             // Correct offset address for "jmp start" where start is at .text section. 
             // Since we move .plt from address 0x140 to 0x00 and keep .text at where 
             // it is.
              uint64_t addrA = SectionAddr + (((uint64_t) (Bytes[1]) & 0xff) << 16) | 
                (((int64_t) (Bytes[2]) & 0xff) << 8) | ((uint8_t) (Bytes[3]) & 0xff);
              outs() << "/*       0:*/	36 " << format("%02" PRIx64, (addrA & 0xff0000) >> 16) 
                << format(" %02" PRIx64, (addrA & 0xff00) >> 8)
                << format(" %02" PRIx64, (addrA & 0xff)) 
                << "\t\t\t\t     /* jmp	" << addrA << "*/\n";
              Size = 4;
            }
            else {
              if (DisAsm->getInstruction(Inst, Size, memoryObject,
                                         SectionAddr + Index,
                                         DebugOut, CommentStream)) {
                outs() << format("/*%8" PRIx64 ":*/", Index);
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
            }
          }
        }
        lastDumpAddr = SectSize;
      }
    }
  }
}

static void DumpSoSectionsInfoToFile(const ObjectFile *o) {
  error_code ec;
  std::string Error;
  std::size_t addr, end;
  StringRef Name;
  StringRef Contents;
  uint64_t BaseAddr;
  bool BSS;
  for (section_iterator si = o->begin_sections(),
                        se = o->end_sections();
                        si != se; si.increment(ec)) {
    if (error(ec)) return;
    if (error(si->getName(Name))) return;
    if (error(si->getContents(Contents))) return;
    if (error(si->getAddress(BaseAddr))) return;
    if (error(si->isBSS(BSS))) return;
    if (Name == ".dynsym") {
      int num_dyn_entry = 0;
      FILE *fd_num_dyn_entry;
      fd_num_dyn_entry = fopen("num_dyn_entry", "r");
      if (fd_num_dyn_entry != NULL) {
        fscanf(fd_num_dyn_entry, "%d", &num_dyn_entry);
      }
      fclose(fd_num_dyn_entry);
      raw_fd_ostream fd_dynsym("dynsym", Error);
      int count = 0;
      for (std::size_t addr = 0, end = Contents.size(); addr < end; addr += 16) {
        fd_dynsym << hexdigit((Contents[addr] >> 4) & 0xF, true)
                   << hexdigit(Contents[addr] & 0xF, true) << " ";
        fd_dynsym << hexdigit((Contents[addr+1] >> 4) & 0xF, true)
                   << hexdigit(Contents[addr+1] & 0xF, true) << " ";
        fd_dynsym << hexdigit((Contents[addr+2] >> 4) & 0xF, true)
                   << hexdigit(Contents[addr+2] & 0xF, true) << " ";
        fd_dynsym << hexdigit((Contents[addr+3] >> 4) & 0xF, true)
                   << hexdigit(Contents[addr+3] & 0xF, true) << " ";
        count++;
      }
      for (int i = count; i < num_dyn_entry; i++) {
        fd_dynsym << "00 00 00 00 ";
      }
    }
    else if (Name == ".dynstr") {
      raw_fd_ostream fd_dynstr("dynstr", Error);
      raw_fd_ostream fd_dynstrAscii("dynstrAscii", Error);
      for (std::size_t addr = 0, end = Contents.size(); addr < end; addr++) {
        fd_dynstr << hexdigit((Contents[addr] >> 4) & 0xF, true)
                   << hexdigit(Contents[addr] & 0xF, true) << " ";
        if (addr == 0)
          continue;
        if (Contents[addr] == '\0')
          fd_dynstrAscii << "\n";
        else
          fd_dynstrAscii << Contents[addr];
      }
    }
  }
}

static void DumpExeSectionsInfoToFile(const ObjectFile *o) {
  error_code ec;
  std::string Error;
  std::size_t addr, end;
  StringRef Name;
  StringRef Contents;
  uint64_t BaseAddr;
  bool BSS;
  for (section_iterator si = o->begin_sections(),
                        se = o->end_sections();
                        si != se; si.increment(ec)) {
    if (error(ec)) return;
    if (error(si->getName(Name))) return;
    if (error(si->getContents(Contents))) return;
    if (error(si->getAddress(BaseAddr))) return;
    if (error(si->isBSS(BSS))) return;
    if (Name == ".got.plt") {
      uint64_t BaseAddr;
      if (error(si->getAddress(BaseAddr))) 
        assert(1 && "Cannot get BaseAddr of section .got.plt");
      raw_fd_ostream fd_global_offset("global_offset", Error);
      fd_global_offset << format("%02" PRIx64 " ", BaseAddr >> 24);
      fd_global_offset << format("%02" PRIx64 " ", (BaseAddr >> 16) & 0xFF);
      fd_global_offset << format("%02" PRIx64 " ", (BaseAddr >> 8) & 0xFF);
      fd_global_offset << format("%02" PRIx64 "    ", BaseAddr & 0xFF);
    }
  }
}

// Modified from PrintSectionContents()
static void PrintDataSections(const ObjectFile *o, uint64_t& lastDumpAddr) {
  error_code ec;

  for (section_iterator si = o->begin_sections(),
                        se = o->end_sections();
                        si != se; si.increment(ec)) {
    if (error(ec)) return;
    PrintDataSection(o, lastDumpAddr, si);
  }
}


// return 0x80000000 if fail
// return (BaseAddrB - BaseAddrA)
static uint64_t SectionsDistance(const ObjectFile *o, StringRef secA, 
  StringRef secB) {
  error_code ec;
  uint64_t BaseAddrA = 0;
  uint64_t BaseAddrB = 0;

  for (section_iterator si = o->begin_sections(),
                        se = o->end_sections();
                        si != se; si.increment(ec)) {
    if (error(ec)) return 0x80000000;
    StringRef Name;
    StringRef Contents;
    uint64_t BaseAddr;
    bool BSS;
    if (error(si->getName(Name))) return 0x80000000;
    if (error(si->getContents(Contents))) return 0x80000000;
    if (error(si->getAddress(BaseAddr))) return 0x80000000;
    if (error(si->isBSS(BSS))) return 0x80000000;

    if (Name == secA)
      BaseAddrA = BaseAddr;
    if (Name == secB)
      BaseAddrB = BaseAddr;
  }
  if (BaseAddrA == 0 || BaseAddrB == 0)
    return 0x80000000;
  else
    return (BaseAddrB - BaseAddrA);
}

static uint64_t SectionOffset(const ObjectFile *o, StringRef secName) {
  error_code ec;

  for (section_iterator si = o->begin_sections(),
                        se = o->end_sections();
                        si != se; si.increment(ec)) {
    if (error(ec)) return 0;
    StringRef Name;
    StringRef Contents;
    uint64_t BaseAddr;
    bool BSS;
    if (error(si->getName(Name))) return 0;
    if (error(si->getContents(Contents))) return 0;
    if (error(si->getAddress(BaseAddr))) return 0;
    if (error(si->isBSS(BSS))) return 0;

    if (Name == secName)
      return BaseAddr;
  }
  return 0;
}

static void Elf2Hex(const ObjectFile *o) {
  std::string Error;
  const Target *TheTarget = getTarget(o);
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
        MCObjectSymbolizer::createObjectSymbolizer(*Ctx.get(), RelInfo, o));
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
      new MCObjectDisassembler(*o, *DisAsm, *MIA));
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

  uint64_t lastDumpAddr = 0;
  uint64_t startAddr = GetSectionHeaderStartAddress(o, "_start");
//  outs() << format("_start address:%08" PRIx64 "\n", startAddr);
  if (DumpSo) { // .so
    DisassembleObjectInHexFormat(o, ".text", lastDumpAddr);
    // outs() << format("lastDumpAddr:%08" PRIx64 "\n", lastDumpAddr);
    PrintDataSections(o, lastDumpAddr);
    DumpSoSectionsInfoToFile(o);
  }
  else if (LinkSo) { // exe refer to .so
    cpu0DynFunIndex.createPltName(o);
    uint64_t secOffset = SectionOffset(o, ".plt");
    uint64_t secDistance = SectionsDistance(o, ".plt", ".text");
    DisassemblePltSecInHexFormat(o, DisAsm, IP, secOffset, secDistance, lastDumpAddr);
    DisassembleObjectInHexFormat(o, ".text", lastDumpAddr);
    // outs() << format("lastDumpAddr:%08" PRIx64 "\n", lastDumpAddr);
    DumpExeSectionsInfoToFile(o);
  }
  else { // exe without refer to .so
    uint64_t secOffset = SectionOffset(o, ".plt");
    uint64_t secDistance = SectionsDistance(o, ".plt", ".text");
    DisassemblePltSecInHexFormat(o, DisAsm, IP, secOffset, secDistance, lastDumpAddr);
    DisassembleObjectInHexFormat(o, ".text", lastDumpAddr);
  }
}
