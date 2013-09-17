.. _sec-lld:

LLD for Cpu0
==============

This chapter add Cpu0 backend in lld. With this lld Cpu0 for ELF linker support,
the program with global variables can be allocated in ELF file format layout. 
Meaning the relocation records of global variable can be solved. In addition, 
llvm-objdump driver is modified for support generate Hex file from ELF.
With these two tools supported, the program with global variables exist in section
.data and .rodata can be accessed and transfered to Hex file which feed to 
Verilog Cpu0 machine and run on your PC/Laptop.

LLD web site [#]_. LLD install requirement on Linux [#]_. 
In spite of the requirement, we
only can build with gcc4.7 above (clang will fail) on Linux. 
If you run with Virtual Machine (VM), please keep your phisical memory size 
setting over 1GB to avoid link error with insufficient memory.


Install lld
-------------

LLD project is underdevelopment and can be compiled with c++11 standard (C++
2011 year announced standard). Currently, we only know how to build lld with 
llvm on Linux platform or Linux VM. Please let us know if you know how to build
it on iMac with Xcode. So, if you got iMac only, please install VM (such as 
Virtual Box). We porting lld Cpu0 at 2013/08/16, so please checkout the last
commit of 2013/08/15 of llvm and lld or the commit id are
da44b4f68bcf2adcb74214670a266b43a1a6888f(llvm)
014d684d27a0f520a30285051a5c8194c87e0194(lld) as follows,

.. code-block:: bash

  [Gamma@localhost test]$ mkdir lld
  [Gamma@localhost test]$ cd lld
  [Gamma@localhost lld]$ git clone http://llvm.org/git/llvm.git src
  Cloning into 'src'...
  remote: Counting objects: 780029, done.
  remote: Compressing objects: 100% (153947/153947), done.
  remote: Total 780029 (delta 637206), reused 764781 (delta 622170)
  Receiving objects: 100% (780029/780029), 125.74 MiB | 243 KiB/s, done.
  Resolving deltas: 100% (637206/637206), done.
  [Gamma@localhost lld]$ cd src/
  [Gamma@localhost src]$ git log
  ...
  Date:   Fri Aug 16 00:15:20 2013 +0000

      InstCombine: Simplify if(x!=0 && x!=-1).
      
      When both constants are positive or both constants are negative,
      InstCombine already simplifies comparisons like this, but when
      it's exactly zero and -1, the operand sorting ends up reversed
      and the pattern fails to match. Handle that special case.
      
      Follow up for rdar://14689217
      
      git-svn-id: https://llvm.org/svn/llvm-project/llvm/trunk@188512 91177308-0d3

  commit da44b4f68bcf2adcb74214670a266b43a1a6888f
  Author: Hans Wennborg <hans@hanshq.net>
  Date:   Thu Aug 15 23:44:31 2013 +0000

  [Gamma@localhost src]$ git checkout da44b4f68bcf2adcb74214670a266b43a1a6888f
  Note: checking out 'da44b4f68bcf2adcb74214670a266b43a1a6888f'.

  You are in 'detached HEAD' state. You can look around, make experimental
  changes and commit them, and you can discard any commits you make in this
  state without impacting any branches by performing another checkout.

  If you want to create a new branch to retain commits you create, you may
  do so (now or later) by using -b with the checkout command again. Example:

    git checkout -b new_branch_name

  HEAD is now at da44b4f... CMake: polish the Windows packaging rules

  [Gamma@localhost src]$ cd tools/
  [Gamma@localhost tools]$ git clone http://llvm.org/git/lld.git lld
  ...
  Resolving deltas: 100% (6422/6422), done.
  [Gamma@localhost tools]$ cd lld/[Gamma@localhost src]$ git log
  ...
  Date:   Wed Aug 21 22:57:10 2013 +0000

      add InputGraph functionality
      
      git-svn-id: https://llvm.org/svn/llvm-project/lld/trunk@188958 91177308-0d34

  commit 014d684d27a0f520a30285051a5c8194c87e0194
  Author: Hans Wennborg <hans@hanshq.net>
  Date:   Tue Aug 13 21:44:44 2013 +0000
  [Gamma@localhost lld]$ git checkout 014d684d27a0f520a30285051a5c8194c87e0194
  Note: checking out '014d684d27a0f520a30285051a5c8194c87e0194'.

  You are in 'detached HEAD' state. You can look around, make experimental
  changes and commit them, and you can discard any commits you make in this
  state without impacting any branches by performing another checkout.

  If you want to create a new branch to retain commits you create, you may
  do so (now or later) by using -b with the checkout command again. Example:

    git checkout -b new_branch_name

  HEAD is now at 014d684... [PECOFF] Handle "--" option explicitly


Next, update llvm 2013/08/16 source code to support Cpu0 as follows,

.. code-block:: bash

  [Gamma@localhost src]$ pwd
  /home/Gamma/test/lld/src
  [Gamma@localhost src]$ cp -rf ~/test/lbd/docs/BackendTutorial/
  LLVMBackendTutorialExampleCode/3.4_20130816_src_files_modify/modify/src/* .
  [Gamma@localhost src]$ grep -R "cpu0" include/
  include/llvm/ADT/Triple.h:#undef cpu0
  include/llvm/ADT/Triple.h:    cpu0,    // For Tutorial Backend Cpu0
  include/llvm/ADT/Triple.h:    cpu0el,
  include/llvm/Object/ELFObjectFile.h:           Triple::cpu0el : Triple::cpu0;
  include/llvm/Support/ELF.h:  EF_CPU0_ARCH_32R2 = 0x70000000, // cpu032r2
  include/llvm/Support/ELF.h:  EF_CPU0_ARCH_64R2 = 0x80000000, // cpu064r2
  [Gamma@localhost src]$ cd lib/Target/
  [Gamma@localhost Target]$ ls
  AArch64         MSP430                   TargetJITInfo.cpp
  ARM             NVPTX                    TargetLibraryInfo.cpp
  CMakeLists.txt  PowerPC                  TargetLoweringObjectFile.cpp
  CppBackend      R600                     TargetMachineC.cpp
  Hexagon         README.txt               TargetMachine.cpp
  LLVMBuild.txt   Sparc                    TargetSubtargetInfo.cpp
  Makefile        SystemZ                  X86
  Mangler.cpp     Target.cpp               XCore
  Mips            TargetIntrinsicInfo.cpp
  [Gamma@localhost Target]$ mkdir Cpu0
  [Gamma@localhost Target]$ cd Cpu0/
  [Gamma@localhost Cpu0]$ cp -rf ~/test/lbd/docs/BackendTutorial/
  LLVMBackendTutorialExampleCode/3.4_20130816_Chapter11_2/* . 
  [Gamma@localhost Cpu0]$ ls
  AsmParser                 Cpu0InstrInfo.h           Cpu0SelectionDAGInfo.h
  CMakeLists.txt            Cpu0InstrInfo.td          Cpu0Subtarget.cpp
  Cpu0AnalyzeImmediate.cpp  Cpu0ISelDAGToDAG.cpp      Cpu0Subtarget.h
  Cpu0AnalyzeImmediate.h    Cpu0ISelLowering.cpp      Cpu0TargetMachine.cpp
  Cpu0AsmPrinter.cpp        Cpu0ISelLowering.h        Cpu0TargetMachine.h
  Cpu0AsmPrinter.h          Cpu0MachineFunction.cpp   Cpu0TargetObjectFile.cpp
  Cpu0CallingConv.td        Cpu0MachineFunction.h     Cpu0TargetObjectFile.h
  Cpu0DelUselessJMP.cpp     Cpu0MCInstLower.cpp       Cpu0.td
  Cpu0EmitGPRestore.cpp     Cpu0MCInstLower.h         Disassembler
  Cpu0FrameLowering.cpp     Cpu0RegisterInfo.cpp      InstPrinter
  Cpu0FrameLowering.h       Cpu0RegisterInfo.h        LLVMBuild.txt
  Cpu0.h                    Cpu0RegisterInfo.td       MCTargetDesc
  Cpu0InstrFormats.td       Cpu0Schedule.td           TargetInfo
  Cpu0InstrInfo.cpp         Cpu0SelectionDAGInfo.cpp


Next, copy lld Cpu0 architecture ELF support as follows,

.. code-block:: bash

  [Gamma@localhost Cpu0]$ cd ../../../tools/lld/lib/ReaderWriter/ELF/
  [Gamma@localhost ELF]$ pwd
  /home/Gamma/test/lld/src/tools/lld/lib/ReaderWriter/ELF
  [Gamma@localhost ELF]$ cp -rf ~/test/lbd/docs/BackendTutorial/
  LLVMBackendTutorialExampleCode/Cpu0_lld_20130816/CMakeLists.txt ~/test/lbd/
  docs/BackendTutorial/LLVMBackendTutorialExampleCode/Cpu0_lld_20130816/
  ELFLinkingContext.cpp ~/test/lbd/docs/BackendTutorial/
  LLVMBackendTutorialExampleCode/Cpu0_lld_20130816/Targets.h . 
  [Gamma@localhost ELF]$ cp -rf ~/test/lbd/docs/BackendTutorial/
  LLVMBackendTutorialExampleCode/Cpu0_lld_20130816/Resolver.cpp ../../Core/.


Finally, update llvm-objdump to support convert ELF file to Hex file as follows,

.. code-block:: bash

  [Gamma@localhost ELF]$ cd ../../../../llvm-objdump/
  [Gamma@localhost llvm-objdump]$ pwd
  /home/Gamma/test/lld/src/tools/llvm-objdump
  [Gamma@localhost llvm-objdump]$ cp -rf ~/test/lbd/docs/BackendTutorial/
  LLVMBackendTutorialExampleCode/llvm-objdump/* .

Now, build llvm/lld 2013/08/16 with Cpu0 support as follows,


.. code-block:: bash

  [Gamma@localhost cmake_debug_build]$ cmake -DCMAKE_CXX_COMPILER=g++ -
  DCMAKE_C_COMPILER=gcc -DCMAKE_CXX_FLAGS=-std=c++11 -DCMAKE_BUILD_TYPE=Debug
  -G "Unix Makefiles" ../src
  -- The C compiler identification is GNU 4.7.2
  -- The CXX compiler identification is GNU 4.7.2
  ...
  -- Targeting Cpu0
  ...
  -- Configuring done
  -- Generating done
  -- Build files have been written to: /home/Gamma/test/lld/cmake_debug_build


Cpu0 lld
---------

The code added on lld to support Cpu0 ELF as follows,


.. rubric:: LLVMBackendTutorialExampleCode/Cpu0_lld_20130816/CMakeLists.txt
.. code-block:: c++

  target_link_libraries(lldELF
    ...
    lldCpu0ELFTarget
    )


.. rubric:: LLVMBackendTutorialExampleCode/Cpu0_lld_20130816/ELFLinkingContext.cpp
.. code-block:: c++

  uint16_t ELFLinkingContext::getOutputMachine() const {
    switch (getTriple().getArch()) {
    ...
    case llvm::Triple::cpu0:
      return llvm::ELF::EM_CPU0;
    ...
    }
  }

.. rubric:: LLVMBackendTutorialExampleCode/Cpu0_lld_20130816/Targets.h
.. code-block:: c++

  #include "Cpu0/Cpu0Target.h"

.. rubric:: LLVMBackendTutorialExampleCode/Cpu0_lld_20130816/Resolver.cpp
.. code-block:: c++

  bool Resolver::checkUndefines(bool final) {
    ...
        if (_context.printRemainingUndefines()) {
          if (undefAtom->name() == "_start") { // cschen debug
            foundUndefines = false;
            continue;
          }
          ...
        }
    ...
  }

.. rubric:: LLVMBackendTutorialExampleCode/Cpu0_lld_20130816/Cpu0/CMakeLists.txt
.. literalinclude:: ../LLVMBackendTutorialExampleCode/Cpu0_lld_20130816/Cpu0/CMakeLists.txt

.. rubric:: LLVMBackendTutorialExampleCode/Cpu0_lld_20130816/Cpu0/Cpu0LinkingContext.h
.. literalinclude:: ../LLVMBackendTutorialExampleCode/Cpu0_lld_20130816/Cpu0/Cpu0LinkingContext.h

.. rubric:: LLVMBackendTutorialExampleCode/Cpu0_lld_20130816/Cpu0/Cpu0LinkingContext.cpp
.. literalinclude:: ../LLVMBackendTutorialExampleCode/Cpu0_lld_20130816/Cpu0/Cpu0LinkingContext.cpp

.. rubric:: LLVMBackendTutorialExampleCode/Cpu0_lld_20130816/Cpu0/Cpu0RelocationHandler.h
.. literalinclude:: ../LLVMBackendTutorialExampleCode/Cpu0_lld_20130816/Cpu0/Cpu0RelocationHandler.h

.. rubric:: LLVMBackendTutorialExampleCode/Cpu0_lld_20130816/Cpu0/Cpu0RelocationHandler.cpp
.. literalinclude:: ../LLVMBackendTutorialExampleCode/Cpu0_lld_20130816/Cpu0/Cpu0RelocationHandler.cpp
.. rubric:: LLVMBackendTutorialExampleCode/Cpu0_lld_20130816/Cpu0/Cpu0LinkingContext.cpp
.. literalinclude:: ../LLVMBackendTutorialExampleCode/Cpu0_lld_20130816/Cpu0/Cpu0LinkingContext.cpp

.. rubric:: LLVMBackendTutorialExampleCode/Cpu0_lld_20130816/Cpu0/Cpu0Target.h
.. literalinclude:: ../LLVMBackendTutorialExampleCode/Cpu0_lld_20130816/Cpu0/Cpu0Target.h

.. rubric:: LLVMBackendTutorialExampleCode/Cpu0_lld_20130816/Cpu0/Cpu0TargetHandler.h
.. literalinclude:: ../LLVMBackendTutorialExampleCode/Cpu0_lld_20130816/Cpu0/Cpu0TargetHandler.h

.. rubric:: LLVMBackendTutorialExampleCode/Cpu0_lld_20130816/Cpu0/Cpu0TargetHandler.cpp
.. literalinclude:: ../LLVMBackendTutorialExampleCode/Cpu0_lld_20130816/Cpu0/Cpu0TargetHandler.cpp


ELF to Hex
-----------

Update llvm-objdump driver to support ELF to Hex for Cpu0 backend as follows,

.. rubric:: LLVMBackendTutorialExampleCode/llvm-objdump/llvm-objdump.cpp
.. code-block:: c++

  ...
  ConvertElf2Hex("elf2hex", cl::desc("Display the hex content of verilog cpu0 needed sections"));
  
  static cl::opt<bool>
  ...
  static void PrintSectionContents(const ObjectFile *o) {
    error_code ec;
    for (section_iterator si = o->begin_sections(),
                          se = o->end_sections();
                          si != se; si.increment(ec)) {
      ...
      // Dump out the content as hex and printable ascii characters.
      for (std::size_t addr = 0, end = Contents.size(); addr < end; addr += 16) {
        outs() << "/*" << format(" %04" PRIx64 " ", BaseAddr + addr);
        ...
        outs() << "*/";
        // Print ascii.
        ...
      }
    }
  }
  
  // For cpu0 -elf2hex begin:
  static uint64_t GetSectionHeaderStartAddress(const ObjectFile *o, StringRef sectionName) {
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
  
  static void DisassembleObjectForHex(const ObjectFile *Obj/*, bool InlineRelocs*/, uint64_t& lastAddr) {
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
  
  static void PrintDataSections(const ObjectFile *o, uint64_t lastAddr) {
    error_code ec;
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
  
      if (Name == ".rodata" || Name == ".data") {
        if (Contents.size() <= 0) {
          continue;
        }
        for (std::size_t addr = lastAddr, end = BaseAddr; addr < end; addr += 4) {
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
    }
  }
  
  static void Elf2Hex(const ObjectFile *o) {
    uint64_t startAddr = GetSectionHeaderStartAddress(o, "_start");
  //  outs() << format("_start address:%08" PRIx64 "\n", startAddr);
    uint64_t lastAddr;
    DisassembleObjectForHex(o, lastAddr);
    PrintDataSections(o, lastAddr);
  }
  // For cpu0 -elf2hex end:
  
  static void DumpObject(const ObjectFile *o) {
    outs() << '\n';
    if (ConvertElf2Hex)
      outs() << "/*";
    outs() << o->getFileName()
           << ":\tfile format " << o->getFileFormatName();
    if (ConvertElf2Hex)
      outs() << "*/";
    ...
    if (ConvertElf2Hex)
      Elf2Hex(o);
    ...
  }
  ...
  int main(int argc, char **argv) {
    ...
    if (!Disassemble
        ...
        && !ConvertElf2Hex
        ...) {
      ...
    }
    ...
  }


Run
-----

File printf-stdarg.c came from internet download which is GPL2 license. GPL2 
is more restricted than LLVM license. File printf-stdarg-2.c is modified from 
printf-stdarg.c which to supply the printf() function for /demo/verification/debug
purpose on Cpu0 backend. File printf-stdarg-1.c is file for testing the printf()
function implemented on PC OS platform. Let's run printf-stdarg-2.c on Cpu0 and
compare with the result of printf() function which implemented by PC OS as follows,

.. rubric:: LLVMBackendTutorialExampleCode/InputFiles/printf-stdarg-1.c
.. literalinclude:: ../LLVMBackendTutorialExampleCode/InputFiles/printf-stdarg-1.c
    :start-after: /// start

.. rubric:: LLVMBackendTutorialExampleCode/InputFiles/printf-stdarg-2.c
.. literalinclude:: ../LLVMBackendTutorialExampleCode/InputFiles/printf-stdarg-2.c
    :start-after: /// start

.. code-block:: bash

  [Gamma@localhost InputFiles]$ /usr/local/llvm/release/cmake_debug_build/bin/
  clang -target mips-unknown-linux-gnu -c printf-stdarg-2.c -emit-llvm -o
  printf-stdarg-2.bc
  printf-stdarg-2.c:75:19: warning: incomplete format specifier [-Wformat]
    printf("%d %s(s)%", 0, "message");
                    ^
  1 warning generated.
  [Gamma@localhost InputFiles]$ /home/Gamma/test/lld/cmake_debug_build/bin/llc
  -march=cpu0 -relocation-model=static -filetype=obj printf-stdarg-2.bc -o
  printf-stdarg-2.cpu0.o
  [Gamma@localhost InputFiles]$ /home/Gamma/test/lld/cmake_debug_build/bin/lld 
  -flavor gnu -target cpu0-unknown-linux-gnu printf-stdarg-2.cpu0.o -o a.out
  [Gamma@localhost InputFiles]$ /home/Gamma/test/lld/cmake_debug_build/bin/
  llvm-objdump -elf2hex a.out > ../cpu0_verilog/redesign/cpu0s.hex
  [Gamma@localhost InputFiles]$ cd ../cpu0_verilog/redesign/
  [Gamma@localhost redesign]$ iverilog -o cpu0s cpu0s.v 
  [Gamma@localhost redesign]$ ls
  cpu0s  cpu0s.hex  cpu0s.v
  [Gamma@localhost redesign]$ ./cpu0s 
  WARNING: cpu0s.v:317: $readmemh(cpu0s.hex): Not enough words in the file for 
  the requested range [0:65535].
  taskInterrupt(001)
  global variable gI = 100
  time1 = 1 10 12
  date = 2012 10 12 1 2 3
  time2 = 1 10 12
  Hello world!
  printf test
  (null) is null pointer
  5 = 5
  -2147483647 = - max int
  char a = 'a'
  hex ff = ff
  hex 00 = 00
  signed -3 = unsigned 4294967293 = hex fffffffd
  0 message(s)
  0 message(s) with %
  justif: "left      "
  justif: "     right"
   3: 0003 zero padded
   3: 3    left justif.
   3:    3 right justif.
  -3: -003 zero padded

  [Gamma@localhost InputFiles]$ gcc printf-stdarg-1.c
  /usr/lib/gcc/x86_64-redhat-linux/4.7.2/../../../../lib64/crt1.o: In function 
  `_start':
  (.text+0x20): undefined reference to `main'
  collect2: error: ld returned 1 exit status
  [Gamma@localhost InputFiles]$ gcc printf-stdarg-1.c
  [Gamma@localhost InputFiles]$ ./a.out
  Hello world!
  printf test
  (null) is null pointer
  5 = 5
  -2147483647 = - max int
  char a = 'a'
  hex ff = ff
  hex 00 = 00
  signed -3 = unsigned 4294967293 = hex fffffffd
  0 message(s)
  0 message(s) with %
  justif: "left      "
  justif: "     right"
   3: 0003 zero padded
   3: 3    left justif.
   3:    3 right justif.
  -3: -003 zero padded
  -3: -3   left justif.
  -3:   -3 right justif.


Except "signed -3 = unsigned - = hex -" didn't display well on Cpu0 version, 
others are fine.


Summary
--------

Thanks the llvm open source project. 
To write a linker and ELF to Hex tools for the new CPU architecture is easy and 
reliable. 
Combine with the llvm compiler backend of support new architecture Cpu0 and 
Verilog language program in the previouse Chapters, we design a software 
toolchain to compile C/C++ code, link and run it on Verilog Cpu0 simulated
machine of PC without any real hardware to investment.
If you like to pay money to buy the FPGA development hardware, we believe the 
code can run on FPGA CPU without problem even though we didn't do it.
System program toolchain can be designed just like we show you at this point. 
School knowledge of system program, compiler, linker, loader, computer 
architecture and CPU design can be translate into a real work and see how it be 
run. Now, these school books knowledge is not limited on paper. 
We program it, design it and run it on real world.


.. [#] http://lld.llvm.org/

.. [#] http://lld.llvm.org/getting_started.html#on-unix-like-systems

