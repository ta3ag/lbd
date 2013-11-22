.. _sec-lld:

LLD for Cpu0
==============

This chapter add Cpu0 backend in lld. With this lld Cpu0 for ELF linker support,
the program with global variables can be allocated in ELF file format layout. 
Meaning the relocation records of global variable can be solved. In addition, 
llvm-objdump driver is modified for support generate Hex file from ELF.
With these two tools supported, the program with global variables exist in 
section.data and .rodata can be accessed and transfered to Hex file which feed 
to Verilog Cpu0 machine and run on your PC/Laptop.

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
Virtual Box). We porting lld Cpu0 at 2013/10/30, so please checkout the
commit id 99a43d3b8f5cf86b333055a56220c6965fd9ece4(llvm) and
5d1737ac704352357fd28cfe3b2daf9aa308fb86(lld) which commited at 2013/10/30 as 
follows,

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

  [Gamma@localhost src]$ git checkout 99a43d3b8f5cf86b333055a56220c6965fd9ece4
  Note: checking out '99a43d3b8f5cf86b333055a56220c6965fd9ece4'.

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
  [Gamma@localhost tools]$ cd lld/
  [Gamma@localhost lld]$ git checkout 5d1737ac704352357fd28cfe3b2daf9aa308fb86
  Note: checking out '5d1737ac704352357fd28cfe3b2daf9aa308fb86'.

  You are in 'detached HEAD' state. You can look around, make experimental
  changes and commit them, and you can discard any commits you make in this
  state without impacting any branches by performing another checkout.

  If you want to create a new branch to retain commits you create, you may
  do so (now or later) by using -b with the checkout command again. Example:

    git checkout -b new_branch_name

  HEAD is now at 014d684... [PECOFF] Handle "--" option explicitly


Next, update llvm 2013/10/30 source code to support Cpu0 as follows,

.. code-block:: bash

  [Gamma@localhost src]$ pwd
  /home/Gamma/test/lld/src
  [Gamma@localhost src]$ cp -rf ~/test/lbd/docs/BackendTutorial/
  lbdex/3.4_1030_src_files_modify/modify/src/* .
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
  lbdex/3.4_0830_Chapter12_2/* . 
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
  lbdex/Cpu0_lld_1030/Cpu0 .
  [Gamma@localhost ELF]$ cp -f ~/test/lbd/docs/BackendTutorial/
  lbdex/Cpu0_lld_1030/CMakeLists.txt .
  [Gamma@localhost ELF]$ cp -f ~/test/lbd/docs/BackendTutorial/
  lbdex/Cpu0_lld_1030/ELFLinkingContext.cpp .
  [Gamma@localhost ELF]$ cp -f ~/test/lbd/docs/BackendTutorial/
  lbdex/Cpu0_lld_1030/Targets.h .
  [Gamma@localhost ELF]$ cp -f ~/test/lbd/docs/BackendTutorial/
  lbdex/Cpu0_lld_1030/Resolver.cpp ../../Core/.


Finally, update llvm-objdump to support convert ELF file to Hex file as follows,

.. code-block:: bash

  [Gamma@localhost ELF]$ cd ../../../../llvm-objdump/
  [Gamma@localhost llvm-objdump]$ pwd
  /home/Gamma/test/lld/src/tools/llvm-objdump
  [Gamma@localhost llvm-objdump]$ cp -rf ~/test/lbd/docs/BackendTutorial/
  lbdex/llvm-objdump/* .

Now, build llvm/lld with Cpu0 support as follows,


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


Cpu0 lld souce code
---------------------

The code added on lld to support Cpu0 ELF as follows,


.. rubric:: lbdex/Cpu0_lld_1030/CMakeLists.txt
.. code-block:: c++

  target_link_libraries(lldELF
    ...
    lldCpu0ELFTarget
    )


.. rubric:: lbdex/Cpu0_lld_1030/ELFLinkingContext.cpp
.. code-block:: c++

  uint16_t ELFLinkingContext::getOutputMachine() const {
    switch (getTriple().getArch()) {
    ...
    case llvm::Triple::cpu0:
      return llvm::ELF::EM_CPU0;
    ...
    }
  }

.. rubric:: lbdex/Cpu0_lld_1030/Targets.h
.. code-block:: c++

  #include "Cpu0/Cpu0Target.h"

.. rubric:: lbdex/Cpu0_lld_1030/Resolver.cpp
.. code-block:: c++

  bool Resolver::checkUndefines(bool final) {
    ...
        if (_context.printRemainingUndefines()) {
          if (undefAtom->name() == "_gp_disp") { // cschen debug
            foundUndefines = false;
            continue;
          }
          ...
        }
    ...
  }

.. rubric:: lbdex/Cpu0_lld_1030/Cpu0/CMakeLists.txt
.. literalinclude:: ../lbdex/Cpu0_lld_1030/Cpu0/CMakeLists.txt

.. rubric:: lbdex/Cpu0_lld_1030/Cpu0/Cpu0LinkingContext.h
.. literalinclude:: ../lbdex/Cpu0_lld_1030/Cpu0/Cpu0LinkingContext.h

.. rubric:: lbdex/Cpu0_lld_1030/Cpu0/Cpu0LinkingContext.cpp
.. literalinclude:: ../lbdex/Cpu0_lld_1030/Cpu0/Cpu0LinkingContext.cpp

.. rubric:: lbdex/Cpu0_lld_1030/Cpu0/Cpu0RelocationHandler.h
.. literalinclude:: ../lbdex/Cpu0_lld_1030/Cpu0/Cpu0RelocationHandler.h

.. rubric:: lbdex/Cpu0_lld_1030/Cpu0/Cpu0RelocationHandler.cpp
.. literalinclude:: ../lbdex/Cpu0_lld_1030/Cpu0/Cpu0RelocationHandler.cpp

.. rubric:: lbdex/Cpu0_lld_1030/Cpu0/Cpu0RelocationPass.h
.. literalinclude:: ../lbdex/Cpu0_lld_1030/Cpu0/Cpu0RelocationPass.h

.. rubric:: lbdex/Cpu0_lld_1030/Cpu0/Cpu0RelocationPass.cpp
.. literalinclude:: ../lbdex/Cpu0_lld_1030/Cpu0/Cpu0RelocationPass.cpp

.. rubric:: lbdex/Cpu0_lld_1030/Cpu0/Cpu0LinkingContext.cpp
.. literalinclude:: ../lbdex/Cpu0_lld_1030/Cpu0/Cpu0LinkingContext.cpp

.. rubric:: lbdex/Cpu0_lld_1030/Cpu0/Cpu0Target.h
.. literalinclude:: ../lbdex/Cpu0_lld_1030/Cpu0/Cpu0Target.h

.. rubric:: lbdex/Cpu0_lld_1030/Cpu0/Cpu0TargetHandler.h
.. literalinclude:: ../lbdex/Cpu0_lld_1030/Cpu0/Cpu0TargetHandler.h

.. rubric:: lbdex/Cpu0_lld_1030/Cpu0/Cpu0TargetHandler.cpp
.. literalinclude:: ../lbdex/Cpu0_lld_1030/Cpu0/Cpu0TargetHandler.cpp


Above code in Cpu0 lld support both the static and dynamic link. 
The "#ifdef DLINKER" is for dynamic link support. There are only just over 1 
thousand of code in it. Half of the code size is for the dynamic linker.


ELF to Hex
-----------

Add elf2hex.h and update llvm-objdump driver to support ELF to Hex for Cpu0 
backend as follows,

.. rubric:: lbdex/llvm-objdump/elf2hex.h
.. literalinclude:: ../lbdex/llvm-objdump/elf2hex.h

.. rubric:: lbdex/llvm-objdump/llvm-objdump.cpp
.. literalinclude:: ../lbdex/llvm-objdump/llvm-objdump.cpp
    :start-after: // 1 llvm-objdump -elf2hex code update begin:
    :end-before: // 1 llvm-objdump -elf2hex code udpate end:
.. literalinclude:: ../lbdex/llvm-objdump/llvm-objdump.cpp
    :start-after: // 2 llvm-objdump -elf2hex code update begin:
    :end-before: // 2 llvm-objdump -elf2hex code udpate end:

The "if (DumpSo)" and "if (LinkSo)" included code are for dynamic linker support.
Others are used in both static and dynamic link execution file dump.


LLD introduction 
------------------

In general, linker do the Relocation Records Resolve as Chapter ELF support 
depicted and optimization for those cannot finish in compiler stage. One of 
the optimization opportunity in linker is Dead Code Stripping which will 
explained in this section. List the LLD project status as follows,

- The lld project aims to to be the built-in linker for clang/llvm.
  Currently, clang must invoke the system linker to produce executables.

- web site http://lld.llvm.org/

- Current Status

  - lld is in its early stages of development.
  - It can currently self host on Linux x86-64 with -static.

- How to build

  - cmake -DCMAKE_CXX_COMPILER=g++ -DCMAKE_C_COMPILER=gcc -DCMAKE_CXX_FLAGS=-std
    =c++11 -DCMAKE_BUILD_TYPE=Debug -G "Unix Makefiles" ../src/


The following is for LLD introduction, we extract information from web site, 
http://lld.llvm.org/design.html. It's part and fragment. Please read the lld 
design web document first, then reading the following to ensure you agree to 
our understanding from lld design document and experience in lld backend 
implementation. Because some of the following came from our understanding.


How LLD do the linker job
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

- LLD structure

  - Internal structure Atom

    - Like llvm IR, lld operating and optimize in Atom.

  - ELF reader/writer, Mach-O reader/writer, COFF

    - Connect to any specific linker format by implement the concrete Read/Writer.

    - e.g. Implement Microsoft link format Reader/Writer
      => extend lld to support Microsoft link format.

- Atom

  - An atom is an indivisible chunk of code or data.

  - Typically each user written function or global variable is an atom.

  - In addition, the compiler may emit other atoms, such as for literal c-strings 
    or floating point constants, or for runtime data structures like dwarf unwind 
    info or pointers to initializers.

- Atoms classified:

  - The following Hello World code can be classified with these different kinds of 
    Atoms as follows,

  .. rubric:: Atom example code
  .. code-block:: c++

    extern int printf(const char *format, ...);

    int main(void)
    {
      char *ptr = "Hello world!";

      printf("%s\n", ptr);
    }

  - DefinedAtom

    - 95% of all atoms. This is a chunk of code or data

  - UndefinedAtom

    - printf in this example.

  - SharedLibraryAtom

    - Symbols defined in shared library (file *.so).

  - AbsoluteAtom

    - This is for embedded support where some stuff is implemented in ROM at some 
      fixed address.

.. _lld-atom: 
.. figure:: ../Fig/lld/atom.png
  :scale: 100 %
  :align: center

  Atom classified (from lld web)


Linking Steps
~~~~~~~~~~~~~~

- Command line processing

  - lld -flavor gnu -target cpu0-unknown-linux-gnu hello.o printf-stdarg.o -o a.out

- Parsing input files

  - ELF reader => create lld:File

- Resolving

  - dead code stripping

- Passes/Optimizations

  - Like llvm passes, give the backend chance to do something like optimization. 

- Generate output file

  - Resolving Relocation Records – I guess in this step


Parsing input files 
+++++++++++++++++++++++++

- Input Files

  - A goal of lld is to be file format independent.

  - The lld::Reader is the base class for all object file readers

  - Every Reader subclass defines its own “options” class (for instance the 
    mach-o Reader defines the class ReaderOptionsMachO). This options class is 
    the one-and-only way to control how the Reader operates when parsing an input 
    file into an Atom graph

- Reader

.. rubric:: lld/lib/ReaderWriter/Reader.cpp
.. code-block:: c++

  ~/test/lld/src/tools/lld/lib/ReaderWriter$ cat Reader.cpp
  ...
  #include "lld/ReaderWriter/Reader.h"

  #include "llvm/ADT/OwningPtr.h"
  #include "llvm/ADT/StringRef.h"
  #include "llvm/Support/MemoryBuffer.h"
  #include "llvm/Support/system_error.h"

  namespace lld {
  Reader::~Reader() {
  }
  } // end namespace lld

.. rubric:: lld/lib/ReaderWriter/ELF/Reader.cpp
.. code-block:: c++

  ~/test/lld/src/tools/lld/lib/ReaderWriter/ELF$ cat Reader.cpp 
  namespace lld {
  namespace elf {
  ...
  class ELFReader : public Reader {
  public:
    ELFReader(const ELFLinkingContext &ctx)
        : lld::Reader(ctx), _elfLinkingContext(ctx) {}

    error_code parseFile(std::unique_ptr<MemoryBuffer> &mb,
                         std::vector<std::unique_ptr<File> > &result) const {
  …
  private:
    const ELFLinkingContext &_elfLinkingContext;
  };
  } // end namespace elf

  std::unique_ptr<Reader> createReaderELF(const ELFLinkingContext &context) {
    return std::unique_ptr<Reader>(new elf::ELFReader(context));
  }
  } // end namespace lld


- lld::File representations

  - In memory, abstract C++ classes (lld::Atom, lld::Reference, and lld::File).

    - Data structure keeped in memory to be fast

  - textual (in YAML)

  - atoms:

    - name:    _main

    - scope:   global

    - type:    code

    - content: [ 55, 48, 89, e5, 48, 8d, 3d, 00, 00, 00, 00, 30, c0, e8, 00, 00,
      00, 00, 31, c0, 5d, c3 ]

  - binary format (“native”)

    - With this model for the native file format, files can be read and turned 
      into the in-memory graph of lld::Atoms with just a few memory allocations. 
      And the format can easily adapt over time to new features.


Resolving
+++++++++++

- Dead code stripping (if requested) is done at the end of resolving. 

- The linker does a simple mark-and-sweep. It starts with “root” atoms (like 
  “main” in a main executable) and follows each references and marks each Atom 
  that it visits as “live”. 

- When done, all atoms not marked “live” are removed.

.. rubric:: Dead code stripping - example (modified from llvm lto document web)

.. rubric:: a.h
.. literalinclude:: ../lbdex/InputFiles/a.h

.. rubric:: a.c
.. literalinclude:: ../lbdex/InputFiles/a.c

.. rubric:: ch13_1.c
.. literalinclude:: ../lbdex/InputFiles/ch13_1.c

Above code can be reduced to :num:`Figure #lld-deadcodestripping` to perform
mark and swip in graph for Dead Code Stripping.

.. _lld-deadcodestripping: 
.. figure:: ../Fig/lld/deadcodestripping.png
  :scale: 70 %
  :align: center

  Atom classified (from lld web)


As above example, the foo2() is an isolated node without any reference. It's 
dead code and can be removed in linker optimization. We test this example by 
build-ch13_1.sh and find foo2() cannot be removed. It's reasonable since the 
lld is in its early stages of development. 
Remind, llvm-linker is the linker works on IR level linker optimization. 
Sometime when you got the obj file only (if you have a.o in this case), 
the native linker (such as lld) have the opportunity to do Dead Code Stripping 
while the IR linker hasn't.


Passes/Optimizations
+++++++++++++++++++++++

- Passes

  - stub (PLT) generation

  - GOT instantiation

  - order_file optimization

  - branch island generation

  - branch shim generation

  - Objective-C optimizations (Darwin specific)

  - TLV instantiation (Darwin specific)

  - DTrace probe processing (Darwin specific)

  - compact unwind encoding (Darwin specific)

The Cpu0RelocationPass.cpp and Cpu0RelocationPass.h are example code for lld 
backend Passes embedded. The Relocation Pass structure shown as 
:num:`Figure #lld-f3`. 

.. _lld-f3: 
.. figure:: ../Fig/lld/3.png
  :scale: 100 %
  :align: center

  Cpu0 lld RelocationPass


Generate Output File
+++++++++++++++++++++++

- All concrete writers (e.g. ELF, mach-o, etc) are subclasses of the lld::Writer 
  class.

- Every Writer subclass defines its own “options” class (for instance the mach-o 
  Writer defines the class WriterOptionsMachO). This options class is the 
  one-and-only way to control how the Writer operates when producing an output 
  file from an Atom graph.

- Writer

.. rubric:: lld/lib/ReaderWriter
.. code-block:: c++

  ~/test/lld/src/tools/lld/lib/ReaderWriter$ cat Writer.cpp
  ...
  #include "lld/Core/File.h"
  #include "lld/ReaderWriter/Writer.h"

  namespace lld {
  Writer::Writer() {
  }

  Writer::~Writer() {
  }

  bool Writer::createImplicitFiles(std::vector<std::unique_ptr<File> > &) {
    return true;
  }
  } // end namespace lld

.. rubric:: lld/lib/ReaderWriter
.. code-block:: c++

  ~/test/lld/src/tools/lld/lib/ReaderWriter/ELF$ cat Writer.cpp 
  namespace lld {

  std::unique_ptr<Writer> createWriterELF(const ELFLinkingContext &info) {
    using llvm::object::ELFType;
    ...
    switch (info.getOutputELFType()) {
    case llvm::ELF::ET_EXEC:
      if (info.is64Bits()) {
        if (info.isLittleEndian())
          return std::unique_ptr<Writer>(new
              elf::ExecutableWriter<ELFType<support::little, 8, true>>(info));
        else
          return std::unique_ptr<Writer>(new
                  elf::ExecutableWriter<ELFType<support::big, 8, true>>(info));
  ...

  } // namespace lld


All lld backends which like to handle the Relocation 
Records Resolve need to register a pass when the lld backend code is up.
Next section will show you how to design your lld backend and register a pass 
for Relocation Records Solve. After register the pass LLD will do last two steps 
Passes/Optimization and Generate Output file interactivly just like the "Parsing 
and Generating code" in compiler. LLD will do Passes/Optimization and call your
lld backend hook function "applyRelocation()" (define in 
Cpu0TargetRelocationHandler.cpp) to finish the address binding in linker stage.
Based on this understanding, we believe the "applyRelocation()" is at the step 
of Generate output file rather than Passes/Optimization even LLD web document 
didn't indicate this.


Static linker 
---------------

Let's run the static linker first and explain it next.

Run
~~~~

File printf-stdarg.c came from internet download which is GPL2 license. GPL2 
is more restricted than LLVM license. File printf-stdarg-2.c is modified from 
printf-stdarg.c of printf() function supplied and add some test function for 
/demo/verification/debugpurpose on Cpu0 backend. 
File printf-stdarg-1.c is file for testing the printf()
function implemented on PC OS platform. Let's run printf-stdarg-2.c on Cpu0 and
compare with the result of printf() function which implemented by PC OS as 
below.

.. rubric:: lbdex/InputFiles/printf-stdarg-1.c
.. literalinclude:: ../lbdex/InputFiles/printf-stdarg-1.c
    :start-after: /// start

.. rubric:: lbdex/InputFiles/printf-stdarg-2.c
.. literalinclude:: ../lbdex/InputFiles/printf-stdarg-2.c
    :start-after: /// start

.. rubric:: lbdex/InputFiles/printf-stdarg.c
.. literalinclude:: ../lbdex/InputFiles/printf-stdarg.c
    :start-after: /// start

.. rubric:: lbdex/InputFiles/start.cpp
.. literalinclude:: ../lbdex/InputFiles/start.cpp
    :start-after: /// start

.. rubric:: lbdex/InputFiles/build-printf-stdarg-2.sh
.. code-block:: c++

  #!/usr/bin/env bash
  #TOOLDIR=/home/Gamma/test/lld/cmake_debug_build/bin
  TOOLDIR=/home/cschen/test/lld/cmake_debug_build/bin
  
  cpu=cpu032I
  
  /usr/local/llvm/release/cmake_debug_build/bin/clang -target mips-unknown-linux-
  gnu -c start.cpp -emit-llvm -o start.bc
  /usr/local/llvm/release/cmake_debug_build/bin/clang -target mips-unknown-linux-
  gnu -c printf-stdarg.c -emit-llvm -o printf-stdarg.bc
  /usr/local/llvm/release/cmake_debug_build/bin/clang -target mips-unknown-linux-
  gnu -c printf-stdarg-2.c -emit-llvm -o printf-stdarg-2.bc
  ${TOOLDIR}/llc -march=cpu0 -mcpu=${cpu} -relocation-model=static -filetype=obj 
  start.bc -o start.cpu0.o
  ${TOOLDIR}/llc -march=cpu0 -mcpu=${cpu} -relocation-model=static -filetype=obj 
  printf-stdarg.bc -o printf-stdarg.cpu0.o
  ${TOOLDIR}/llc -march=cpu0 -mcpu=${cpu} -relocation-model=static -filetype=obj 
  printf-stdarg-2.bc -o printf-stdarg-2.cpu0.o
  ${TOOLDIR}/lld -flavor gnu -target cpu0-unknown-linux-gnu start.cpu0.o 
  printf-stdarg.cpu0.o printf-stdarg-2.cpu0.o -o a.out
  ${TOOLDIR}/llvm-objdump -elf2hex a.out > ../cpu0_verilog/cpu0.hex

The cpu0_verilog/cpu0Is.v support cmp instruction and static linker as follows,

.. rubric:: lbdex/cpu0_verilog/cpu0Is.v
.. literalinclude:: ../lbdex/cpu0_verilog/cpu0Is.v

The cpu0_verilog/cpu0IIs.v support slt instruction and static linker as follows,

.. rubric:: lbdex/cpu0_verilog/cpu0IIs.v
.. literalinclude:: ../lbdex/cpu0_verilog/cpu0IIs.v

The build-printf-stdarg-2.sh is for my PC setting. Please change this script to
your lld installed directory and run static linker example code as follows,

.. code-block:: bash

  [Gamma@localhost cpu0_verilog]$ pwd
  /home/Gamma/test/lbd/docs/BackendTutorial/source_ExampleCode/cpu0_verilog
  [Gamma@localhost cpu0_verilog]$ bash clean.sh
  [Gamma@localhost InputFiles]$ cd ../InputFiles/
  [Gamma@localhost InputFiles]$ bash build-printf-stdarg-2.sh
  printf-stdarg-2.c:85:19: warning: incomplete format specifier [-Wformat]
    printf("%d %s(s)%", 0, "message");
                    ^
  1 warning generated.
  [Gamma@localhost InputFiles]$ cd ../cpu0_verilog/
  [Gamma@localhost cpu0_verilog]$ pwd
  /home/Gamma/test/lbd/docs/BackendTutorial/source_ExampleCode/cpu0_verilog
  [Gamma@localhost cpu0_verilog]$ iverilog -o cpu0IIs cpu0IIs.v 
  [Gamma@localhost cpu0_verilog]$ ls
  clean.sh  cpu0Id.v  cpu0IId.v  cpu0IIs  cpu0IIs.v  cpu0Is.v  cpu0.v  dynlinker.v  
  flashio.v
  [Gamma@localhost cpu0_verilog]$ ./cpu0IIs 
  WARNING: cpu0.v:365: $readmemh(cpu0s.hex): Not enough words in the file for 
  the requested range [0:524287].
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
  0 message(s) with \%
  justif: "left      "
  justif: "     right"
   3: 0003 zero padded
   3: 3    left justif.
   3:    3 right justif.
  -3: -003 zero padded

Let's check the result with PC program printf-stdarg-1.c output as follows,

.. code-block:: bash

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
  0 message(s) with \%
  justif: "left      "
  justif: "     right"
   3: 0003 zero padded
   3: 3    left justif.
   3:    3 right justif.
  -3: -003 zero padded
  -3: -3   left justif.
  -3:   -3 right justif.

They are same after the "Hello world!" of printf() function support.
The cpu0I use cmp instruction. You can verify the slt 
instructions is work fine too by change cpu to cpu032II as follows,

.. rubric:: lbdex/InputFiles/build-printf-stdarg-2.sh

.. code-block:: bash

  ...
  cpu=cpu032II
  ...

.. code-block:: bash

  [Gamma@localhost cpu0_verilog]$ pwd
  /home/Gamma/test/lbd/docs/BackendTutorial/source_ExampleCode/cpu0_verilog
  [Gamma@localhost cpu0_verilog]$ bash clean.sh
  [Gamma@localhost InputFiles]$ cd ../InputFiles/
  [Gamma@localhost InputFiles]$ bash build-printf-stdarg-2.sh
  printf-stdarg.c:102:19: warning: incomplete format specifier [-Wformat]
    printf("%d %s(s)\%", 0, "message");
                    ^
  1 warning generated.
  [Gamma@localhost cpu0_verilog]$ ./cpu0IIs 

The verilog machine cpu032IIs include cpu0I all instructions (cmp, jeq, ... 
are included also) and add Chapter12_2 slt, beq, ..., instructions.
Run build-printf-stdarg-2.sh with cpu=cpu032II will generate slt, beq and bne 
instructions instead cmp, jeq, ... instructions. Since cpu0IIs include both
slt, cmp, ... instructions, the slt and cmp both code generated can be run on
it without any problem.


Cpu0 lld structure
~~~~~~~~~~~~~~~~~~~~~

.. _lld-f1: 
.. figure:: ../Fig/lld/1.png
  :scale: 100 %
  :align: center

  Cpu0 lld class relationship

.. _lld-f2: 
.. figure:: ../Fig/lld/2.png
  :scale: 80 %
  :align: center

  Cpu0 lld ELFLinkingContext and DefaultLayout member functions

The Cpu0LinkingContext include the context information for those input obj 
files and output elf file you want to link.
When do linking, the following code will create Cpu0LinkingContext.

.. rubric:: lbdex/Cpu0_lld_1030/ELFLinkingContext.h
.. code-block:: c++

  class ELFLinkingContext : public LinkingContext {
  public:
    ...
    static std::unique_ptr<ELFLinkingContext> create(llvm::Triple);
    ...
  }

.. rubric:: lbdex/Cpu0_lld_1030/ELFLinkingContext.cpp
.. code-block:: c++

  std::unique_ptr<ELFLinkingContext>
  ELFLinkingContext::create(llvm::Triple triple) {
    switch (triple.getArch()) {
    ...
    case llvm::Triple::cpu0:
      return std::unique_ptr<ELFLinkingContext>(
          new lld::elf::Cpu0LinkingContext(triple));
    default:
      return nullptr;
    }
  }

While Cpu0LinkingContext is created by lld ELF driver as above, the following
code in Cpu0LinkingContext constructor will create Cpu0TargetHandler and passing
the Cpu0LinkingContext object pointer to Cpu0TargeHandler.

.. rubric:: lbdex/Cpu0_lld_1030/Cpu0/Cpu0LinkingContext.h
.. code-block:: c++

  class Cpu0LinkingContext LLVM_FINAL : public ELFLinkingContext {
  public:
    Cpu0LinkingContext(llvm::Triple triple)
        : ELFLinkingContext(triple, std::unique_ptr<TargetHandlerBase>(
                                    new Cpu0TargetHandler(*this))) {}
    ...
  }

Finally, the Cpu0TargeHandler constructor will create other related objects
and set up the relation reference object pointers as :num:`Figure #lld-f1`
depicted by the following code.

.. rubric:: lbdex/Cpu0_lld_1030/Cpu0/Cpu0TargetHandler.cpp
.. code-block:: c++

  Cpu0TargetHandler::Cpu0TargetHandler(Cpu0LinkingContext &context)
      : DefaultTargetHandler(context), _gotFile(new GOTFile(context)),
        _relocationHandler(context), _targetLayout(context) {}


According chapter ELF, the linker stands for resolve the relocation records.
The following code give the chance to let lld system call our relocation 
function at proper time.

.. rubric:: lbdex/Cpu0_lld_1030/Cpu0/Cpu0RelocationPass.cpp
.. code-block:: c++

  std::unique_ptr<Pass>
  lld::elf::createCpu0RelocationPass(const Cpu0LinkingContext &ctx) {
    switch (ctx.getOutputELFType()) {
    case llvm::ELF::ET_EXEC:
  #ifdef DLINKER
      if (ctx.isDynamic())
        return std::unique_ptr<Pass>(new DynamicRelocationPass(ctx));
      else
  #endif // DLINKER
        return std::unique_ptr<Pass>(new StaticRelocationPass(ctx));
  #ifdef DLINKER
    case llvm::ELF::ET_DYN:
      return std::unique_ptr<Pass>(new DynamicRelocationPass(ctx));
  #endif // DLINKER
    case llvm::ELF::ET_REL:
      return std::unique_ptr<Pass>();
    default:
      llvm_unreachable("Unhandled output file type");
    }
  }

The "#ifdef DLINKER" part is for dynamic linker will used in next section.
For static linker, a StaticRelocationPass object is created and return.

Now the following code of Cpu0TargetRelocationHandler::applyRelocation() 
will be called through 
Cpu0TargetHandler by lld ELF driver when it meet each relocation record.

.. rubric:: lbdex/Cpu0_lld_1030/Cpu0/Cpu0RelocationHandler.cpp
.. code-block:: c++

  ErrorOr<void> Cpu0TargetRelocationHandler::applyRelocation(
      ELFWriter &writer, llvm::FileOutputBuffer &buf, const lld::AtomLayout &atom,
      const Reference &ref) const {

    switch (ref.kind()) {
    case R_CPU0_NONE:
      break;
    case R_CPU0_HI16:
      relocHI16(location, relocVAddress, targetVAddress, ref.addend());
      break;
    case R_CPU0_LO16:
      relocLO16(location, relocVAddress, targetVAddress, ref.addend());
      break;
    ...
    }
    return error_code::success();
  }

.. rubric:: lbdex/Cpu0_lld_1030/Cpu0/Cpu0TargetHandler.h
.. code-block:: c++

  class Cpu0TargetHandler LLVM_FINAL
      : public DefaultTargetHandler<Cpu0ELFType> {
  public:
    ..
    virtual const Cpu0TargetRelocationHandler &getRelocationHandler() const {
      return _relocationHandler;
    }


Summary as :num:`Figure #lld-f4`. 

.. _lld-f4: 
.. figure:: ../Fig/lld/4.png
  :scale: 80 %
  :align: center

  Cpu0 lld related objects created sequence

Remind, static std::unique_ptr<ELFLinkingContext> 
ELFLinkingContext::create(llvm::Triple) is called without an object of 
class ELFLinkingContext instance (because the static keyword).
The Cpu0LinkingContext constructor will create it's ELFLinkingContext part.
The std::unique_ptr came from c++11 standard.
The unique_ptr objects automatically delete the object they manage (using a 
deleter) as soon as they themselves are destroyed. Just like the Singlelten 
pattern in Design Pattern book or Smart Pointers in Effective C++ book. [#]_

.. _lld-f5: 
.. figure:: ../Fig/lld/5.png
  :scale: 100 %
  :align: center

  Cpu0LinkingContext get Cpu0TargetHandler through &getTargetHandler()


As :num:`Figure #lld-f1` depicted, the Cpu0TargetHandler include the members or 
pointers which can access to other object. The way to access Cpu0TargetHandler
object from Cpu0LinkingContext or Cpu0RelocationHandler rely on 
LinkingContext::getTargetHandler() function. As :num:`Figure #lld-f5` depicted, 
the unique_ptr point to Cpu0TargetHandler will be saved in LinkingContext 
contructor function.

.. note:: std::unique_ptr::get() [#]_

  pointer get() const noexcept;

  Get pointer
  Returns the stored pointer. 


.. note:: std::move() [#]_

  for example:
    std::string bar = "bar-string";
    std::move(bar);

    bar is null after std::move(bar);


Dynamic linker 
---------------

Except the lld code with #ifdef DLINKER. The following code in Verilog exist to 
support dynamic linker.

.. rubric:: lbdex/cpu0_verilog/dynlinker.v
.. literalinclude:: ../lbdex/cpu0_verilog/dynlinker.v

.. rubric:: lbdex/cpu0_verilog/flashio.v
.. literalinclude:: ../lbdex/cpu0_verilog/flashio.v

.. rubric:: lbdex/cpu0_verilog/cpu0Id.v
.. literalinclude:: ../lbdex/cpu0_verilog/cpu0Id.v

.. rubric:: lbdex/cpu0_verilog/cpu0IId.v
.. literalinclude:: ../lbdex/cpu0_verilog/cpu0IId.v

The following code ch_dynamiclinker.cpp and foobar.cpp is the example for 
dynamic linker demostration. File dynamic_linker.cpp is what our implementaion
to execute the dynamic linker function on Cpu0 Verilog machine.

.. rubric:: lbdex/InputFiles/dynamic_linker.h
.. literalinclude:: ../lbdex/InputFiles/dynamic_linker.h

.. rubric:: lbdex/InputFiles/dynamic_linker.cpp
.. literalinclude:: ../lbdex/InputFiles/dynamic_linker.cpp
    :start-after: /// start

.. rubric:: lbdex/InputFiles/ch_dynamiclinker.cpp
.. literalinclude:: ../lbdex/InputFiles/ch_dynamiclinker.cpp
    :start-after: /// start

.. rubric:: lbdex/InputFiles/foobar.cpp
.. literalinclude:: ../lbdex/InputFiles/foobar.cpp
    :start-after: /// start

.. rubric:: lbdex/InputFiles/build-dlinker.sh
.. code-block:: c++
  
  #!/usr/bin/env bash
  #TOOLDIR=/home/Gamma/test/lld/cmake_debug_build/bin
  TOOLDIR=/home/cschen/test/lld/cmake_debug_build/bin
  
  cpu=cpu032I
  
  /usr/local/llvm/release/cmake_debug_build/bin/clang -target mips-unknown-linux-
  gnu -c start.cpp -emit-llvm -o start.bc
  /usr/local/llvm/release/cmake_debug_build/bin/clang -target mips-unknown-linux-
  gnu -c dynamic_linker.cpp -emit-llvm -o dynamic_linker.cpu0.bc
  /usr/local/llvm/release/cmake_debug_build/bin/clang -target mips-unknown-linux-
  gnu -c printf-stdarg.c -emit-llvm -o printf-stdarg.bc
  /usr/local/llvm/release/cmake_debug_build/bin/clang -target mips-unknown-linux-
  gnu -c foobar.cpp -emit-llvm -o foobar.cpu0.bc
  ${TOOLDIR}/llc -march=cpu0 -mcpu=${cpu} -relocation-model=static -filetype=obj 
  -cpu0-reserve-gp=true dynamic_linker.cpu0.bc -o dynamic_linker.cpu0.o
  ${TOOLDIR}/llc -march=cpu0 -mcpu=${cpu} -relocation-model=static -filetype=obj 
  -cpu0-reserve-gp=true printf-stdarg.bc -o printf-stdarg.cpu0.o
  ${TOOLDIR}/llc -march=cpu0 -mcpu=${cpu} -relocation-model=pic -filetype=obj 
  -cpu0-reserve-gp=true -cpu0-no-cpload=true foobar.cpu0.bc -o foobar.cpu0.o
  ${TOOLDIR}/lld -flavor gnu -target cpu0-unknown-linux-gnu -shared -o 
  libfoobar.cpu0.so foobar.cpu0.o
  ${TOOLDIR}/llc -mcpu=${cpu} -march=cpu0 -relocation-model=static -filetype=obj 
  -cpu0-reserve-gp=true start.bc -o start.cpu0.o
  /usr/local/llvm/release/cmake_debug_build/bin/clang -target mips-unknown-linux-
  gnu -c ch_dynamiclinker.cpp -emit-llvm -o ch_dynamiclinker.cpu0.bc
  ${TOOLDIR}/llc -march=cpu0 -mcpu=${cpu} -relocation-model=static -filetype=obj 
  -cpu0-reserve-gp=true ch_dynamiclinker.cpu0.bc -o ch_dynamiclinker.cpu0.o
  ${TOOLDIR}/lld -flavor gnu -target cpu0-unknown-linux-gnu start.cpu0.o printf-
  stdarg.cpu0.o dynamic_linker.cpu0.o ch_dynamiclinker.cpu0.o libfoobar.cpu0.so
  ${TOOLDIR}/llvm-objdump -elf2hex -cpu0dumpso libfoobar.cpu0.so > ../
  cpu0_verilog/libso.hex
  ${TOOLDIR}/llvm-objdump -elf2hex -cpu0linkso a.out > ../cpu0_verilog/cpu0.hex
  cp dynstr dynsym so_func_offset global_offset ../cpu0_verilog/.


Run
~~~~

.. code-block:: bash

  [Gamma@localhost cpu0_verilog]$ pwd
  /home/Gamma/test/lbd/docs/BackendTutorial/source_ExampleCode/cpu0_verilog
  [Gamma@localhost cpu0_verilog]$ bash clean.sh
  [Gamma@localhost InputFiles]$ cd ../InputFiles/
  [Gamma@localhost InputFiles]$ bash build-dlinker.sh
  [Gamma@localhost InputFiles]$ cd ../cpu0_verilog/
  [Gamma@localhost cpu0_verilog]$ pwd
  /home/Gamma/test/lbd/docs/BackendTutorial/source_ExampleCode/cpu0_verilog
  [Gamma@localhost cpu0_verilog]$ iverilog -o cpu0IId cpu0IId.v 
  [Gamma@localhost cpu0_verilog]$ ls
  clean.sh  cpu0Id  cpu0Id.v  cpu0IId.v  cpu0IIs.v  cpu0Is.v  cpu0.v  dynlinker.v  
  flashio.v
  [Gamma@localhost cpu0_verilog]$ ./cpu0Id 
  WARNING: ./cpu0.v:371: $readmemh(cpu0.hex): Not enough words in the file for 
  the requested range [0:524287].
  WARNING: ./dynlinker.v:185: $readmemh(libso.hex): Not enough words in the 
  file for the requested range [0:524287].
  WARNING: ./dynlinker.v:223: $readmemh(dynsym): Not enough words in the file 
  for the requested range [0:191].
  WARNING: ./dynlinker.v:224: $readmemh(dynstr): Not enough words in the file 
  for the requested range [0:95].
  WARNING: ./dynlinker.v:225: $readmemh(so_func_offset): Not enough words in 
  the file for the requested range [0:383].
  numDynEntry = 00000005
  taskInterrupt(001)
  loading _Z3fooii...
  run _Z3fooii...
  foo(1, 2) = 3
  loading _Z3barv...
  run _Z3barv...
  loading _Z2laii...
  run _Z2laii...
  bar() = 11
  RET to PC < 0, finished!


The "#ifdef DEBUG_DLINKER" part of code in dynamic_linker.cpp is for debugging
purpose (since we coding it and take time to debug). After skip these debug
code, the dynamic_linker.cpp is short and not difficult to read.

The run result is under expectation. The main() call foo() function first.
Function foo() is loaded by dynamic linker (dynamic_linker.cpp) from memory
address FLASHADDR (defined in dynamic_linker.h) to memory.
The flashio.v implement the simulation read from flash address.
After loaded foo() body from flash, dynamic_linker.cpp jump to this loaded
address by "ret \$t9" instruction.

Same as static linker, you can generate slt instruction instead of cmp by
change from cpu=cpu0I to cpu0=cpu0II in build-dlinker.sh and run it again to
get the same result.


How to work
~~~~~~~~~~~~~

After run build-dlinker.sh, the following files are created.

.. rubric:: lbdex/cpu0_verilog/cpu0.hex
.. code-block:: bash
  
  /*Disassembly of section .plt:*/
  /*.PLT0:*/
  /*       0:*/	36 00 00 3c                                  /*	jmp	60*/
  /*       4:*/	36 00 00 04                                  /*	jmp	4*/
  /*       8:*/	36 00 00 04                                  /*	jmp	4*/
  /*       c:*/	36 ff ff fc                                  /*	jmp	-4*/

  /*.PLT0:*/
  /*      10:*/	02 eb 00 04                                  /*	st	$lr, 4($gp)*/
  /*      14:*/	02 cb 00 08                                  /*	st	$fp, 8($gp)*/
  /*      18:*/	02 db 00 0c                                  /*	st	$sp, 12($gp)*/
  /*      1c:*/	36 00 09 b8                                  /*	jmp	2488*/

  /*__plt__Z3fooii:*/
  /*      20:*/	09 60 00 04                                  /* addiu	$t9, $zero, 4($gp)*/
  /*      24:*/	02 6b 00 00                                  /*	st	$t9, 0($gp)*/
  /*      28:*/	01 6b 00 10                                  /*	ld	$t9, 16($gp)*/
  /*      2c:*/	3c 60 00 00                                  /*	ret	$t9*/

  /*__plt__Z3barv:*/
  /*      30:*/	09 60 00 05                                  /* addiu	$t9, $zero, 5($gp)*/
  /*      34:*/	02 6b 00 00                                  /*	st	$t9, 0($gp)*/
  /*      38:*/	01 6b 00 10                                  /*	ld	$t9, 16($gp)*/
  /*      3c:*/	3c 60 00 00                                  /*	ret	$t9*/
  ...

  /*main:*/
  ...
  /*     d68:*/	3b ff f2 b4                                  /*	jsub	16773812*/ // call foo()
  ...
  /*     d80:*/	3b ff f3 28                                  /*	jsub	16773928*/ // call printf()
  /*     d84:*/	3b ff f2 a8                                  /*	jsub	16773800*/ // call bar()
  ...
  /*     d9c:*/	3b ff f3 0c                                  /*	jsub	16773900*/ // call printf()
  ...
  /*     db8:*/	3c e0 00 00                                  /*	ret	$lr*/
  ...
  /*Contents of section .data:*/
  /*20a8 */00 00 00 01  00 00 00 01  00 00 00 01  00 00 00 01 /*  ................*/
  ...

.. rubric:: lbdex/cpu0_verilog/dynstr
.. code-block:: bash

  00 5f 5f 74 6c 73 5f 67 65 74 5f 61 64 64 72 00 5f 5a 32 6c 61 69 69 00 5f 5a 
  35 70 6f 77 65 72 69 00 5f 5a 33 66 6f 6f 69 69 00 5f 5a 33 62 61 72 76 00 5f 
  47 4c 4f 42 41 4c 5f 4f 46 46 53 45 54 5f 54 41 42 4c 45 5f 00 5f 44 59 4e 41 
  4d 49 43 00 

.. rubric:: lbdex/cpu0_verilog/dynsym
.. code-block:: c++

  00 00 00 00 00 00 00 01 00 00 00 10 00 00 00 18 00 00 00 22 00 00 00 2b 00 00 
  00 33 00 00 00 49 

.. rubric:: lbdex/cpu0_verilog/global_offset
.. code-block:: bash

  00 00 20 68 

.. rubric:: lbdex/InputFiles/num_dyn_entry
.. code-block:: bash

  6

.. rubric:: lbdex/InputFiles/libfoobar.cpu0.so
.. code-block:: bash

  cschen@cschen-BM6835-BM6635-BP6335:~/test/lbd/docs/BackendTutorial/lbdex/
  InputFiles$ /home/cschen/test/lld/cmake_debug_build/bin/llvm-objdump -s 
  libfoobar.cpu0.so 

  libfoobar.cpu0.so:	file format ELF32-CPU0

  Contents of section :
  ...
  Contents of section .dynsym:
   00e4 00000000 00000000 00000000 00000000  ................
   00f4 00000001 0000019c 00000000 12000004  ................
   0104 00000010 0000019c 0000003c 12000004  ...........<....
   0114 00000018 000001d8 00000038 12000004  ...........8....
   0124 00000021 00000210 00000070 12000004  ...!.......p....
   0134 00000029 00001040 00000000 10000006  ...)...@........
   0144 0000003f 00001040 00000000 11000005  ...?...@........
  Contents of section .dynstr:
   0154 005f5f74 6c735f67 65745f61 64647200  .__tls_get_addr.
   0164 5f5a326c 61696900 5f5a3366 6f6f6969  _Z2laii._Z3fooii
   0174 005f5a33 62617276 005f474c 4f42414c  ._Z3barv._GLOBAL
   0184 5f4f4646 5345545f 5441424c 455f005f  _OFFSET_TABLE_._
   0194 44594e41 4d494300                    DYNAMIC.

.. rubric:: lbdex/InputFiles/a.out
.. code-block:: bash

  cschen@cschen-BM6835-BM6635-BP6335:~/test/lbd/docs/BackendTutorial/lbdex/
  InputFiles$ /home/cschen/test/lld/cmake_debug_build/bin/llvm-objdump -s a.out

  a.out:	file format ELF32-CPU0

  Contents of section :
  ...
  Contents of section .dynsym:
   013c 00000000 00000000 00000000 00000000  ................
   014c 00000001 00000000 00000000 12000000  ................
   015c 0000000a 00000000 00000000 12000000  ................
  Contents of section .dynstr:
   016c 005f5a33 666f6f69 69005f5a 33626172  ._Z3fooii._Z3bar
   017c 76006c69 62666f6f 6261722e 63707530  v.libfoobar.cpu0
   018c 2e736f00                             .so.
  ...
  Contents of section .got.plt:
   2068 00000000 00000000 00000000 00000000  ................
   2078 00000000 00000000 00000000 00000000  ................
   2088 000001d0 00000000 00000000 00000000  ................
   2098 000001e0 00000000 00000000 00000000  ................
  Contents of section .data:
   20a8 00000001 00000001 00000001 00000001  ................


File dynstr is section .dynstr of libfoobar.cpu0.so. File dynsym is the first 
4 bytes of every entry of .dynsym. File global_offset contains the start address 
of section .got.plt.

The code of dynlinker.v will set the memory as follows after program is loaded.
(gp value below is 2068 came from file global_offset).

.. rubric:: memory contents
.. code-block:: bash

  //                                 -----------------------------------
  // gp ---------------------------> | all 0                           | (16 bytes)
  // gp+16 ------------------------> | 0                          |
  // gp+16+1*4 --------------------> | 1st plt entry address      | (4 bytes)
  //                                 | ...                        |
  // gp+16+(numDynEntry-1)*4 ------> | the last plt entry address |
  //                                 ------------------------------
  // gp ---------------------------> | all 0                           | (16 bytes)
  // gp+16+0*8'h10 ----------------> | 32'h10: pointer to plt0         |
  // gp+16+1*8'h10 ----------------> | 1st plt entry                   |
  // gp+16+2*8'h10 ----------------> | 2nd plt entry                   |
  //                                 | ...                             |
  // gp+16+(numDynEntry-1)*8'h10 --> | the last plt entry              |
  //                                 -----------------------------------

For example as ch_dynamiclinker.cpp and foobar.cpp, gp is 2068, numDynEntry is 
the contents of file num_dyn_entry which is 6. Every plt entry above (memory 
address gp+16+1*8'h10..gp+16+(numDynEntry-1)*8'h10) is initialize to "addiu	$t9, 
$zero, 4($gp); st	$t9, 0($gp); ld	$t9, 16($gp); ret	$t9" as follows,


.. rubric:: memory contents
.. code-block:: bash

  //                                 -----------------------------------
  // gp ---------------------------> | all 0                           | (16 bytes)
  // gp+16 ------------------------> | 0                          |
  // gp+16+1*4 --------------------> | 1st plt entry address      | (4 bytes)
  // gp+16+2*4 --------------------> | 1st plt entry address      | (4 bytes)
  //                                 | ...                        |
  // gp+16+(6-1)*4 ----------------> | the last plt entry address |
  //                                 ------------------------------
  // gp ---------------------------> | all 0                           | (16 bytes)
  // gp+16+0*8'h10 ----------------> | 32'h10: pointer to plt0         |
  // gp+16+1*8'h10 ----------------> | addiu	$t9, $zero, 4($gp)       |
  //                                 | st	$t9, 0($gp)                  |
  //                                 | ld	$t9, 16($gp)                 |
  //                                 | ret	$t9                        |
  // gp+16+2*8'h10 ----------------> | addiu	$t9, $zero, 4($gp)       |
  //                                 | st	$t9, 0($gp)                  |
  //                                 | ld	$t9, 16($gp)                 |
  //                                 | ret	$t9                        |
  // ...                             | ...                             |
  // gp+16+(6-1)*8'h10 ------------> | addiu	$t9, $zero, 4($gp)       |
  //                                 | st	$t9, 0($gp)                  |
  //                                 | ld	$t9, 16($gp)                 |
  //                                 | ret	$t9                        |
  //                                 -----------------------------------


:num:`Figure #lld-f6` is the memory content after the example program is loaded.

.. _lld-f6: 
.. figure:: ../Fig/lld/6.png
  :scale: 80 %
  :align: center

  Memory content after the program is loaded

.. _lld-f7: 
.. figure:: ../Fig/lld/7.png
  :scale: 80 %
  :align: center

  Control flow transfer from calling foo() instruction of main() to dynamic linker

:num:`Figure #lld-f7` is the Control flow transfer from call foo() of main() to 
dynamic linker. After ch_dynamiclinker.cpp call foo() first time, it jump to 
__plt_Z3fooii plt entry. Since __plt_Z3fooii is the 3rd plt entry,
it save 3 to 0($gp) memory address then jump to PLT0. The PLT0 purpose is to
save $lr, $fp, $sp and jump to dynamic linker. 
Now, the control flow transfer to dynamic linker.
Dynamic linker will get the loaded function name and function offset of shared 
library by the value of 0($gp) which is 3 set in __plt_Z3fooii. The value 
3 tell dynamic linker load foo() (3rd string in .dynstr) from offset of shared
library, 0x3c (3rd value of Function offset area in Figure).
Now, dynamic linker can load foo() function from flash to memory, set the 
address gp+3*4 to 0x40000 where the address 0x40000 is the foo() function
loaded to memory and prepare jump to the foo() memory address. 
Remind we say the prepare jump to foo(). It because
before jump to foo(), dynamic linker need to restore the $lr, $fp, $sp to the 
value of just before caller calling foo() (they are saved in 4, 8, 12 of $gp 
offset in PLT0, so them can be restored from that address).

.. _lld-f8: 
.. figure:: ../Fig/lld/8.png
  :scale: 80 %
  :align: center

  Transfer from dynamic linker to foo() and back to main()

As :num:`Figure #lld-f8` depicted, control flow from dynamic linker to foo() and
back to caller main() when it meets the instruction "ret $lr" in foo().

Now the program run at the next instruction of call foo() in main(). When it run 
to address 0xd8 "jsub __plt__Z3barv", the control flow will transfer from 
__plt_Z3barv through PLT0 to dynamic linker then load and run bar() from flash 
to memory just like the calling __plt__Z3fooii. 
The difference is bar() will call foo() first and call la() next. 
The call foo() in bar() will jump to foo() directly as 
:num:`Figure #lld-f8` because the content of gp+28 is the address of 0x40000 which
set in dynamic linker when the foo() function is called first time. 

Finally the when bar() call la() function it will jump to __plt_Z3laii since the 
content of $gp+24 point to __plt_Z3laii. 
The __plt_Z3laii code will call dynamic linker
to load la() function, run la() and back to bar() as :num:`Figure #lld-f9`.

.. _lld-f9: 
.. figure:: ../Fig/lld/9.png
  :scale: 80 %
  :align: center

  Call la through __plt_Z3laii in bar()


Cpu0 lld dynamic linker structure
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


Summary
--------

Thanks the llvm open source project. 
To write a linker and ELF to Hex tools for a new CPU architecture is easy and 
reliable. 
Combined with the llvm Cpu0 backend code and Verilog language code we program 
in previouse Chapters, we design a software 
toolchain to compile C/C++ code, link and run it on Verilog Cpu0 simulated
machine of PC without any real hardware to investment.
If you like to pay money to buy the FPGA development hardware, we believe these  
code can run on FPGA CPU even though we didn't do it.
Extend system program toolchain to support a new CPU instructions can be 
designed just like we show you at this point. 
School knowledges of system program, compiler, linker, loader, computer 
architecture and CPU design has been translated into a real work and see how it 
is run. Now, these school books knowledge is not limited on paper. 
We design it, program it and run it on real world.

The total code size of llvm Cpu0 backend compiler, Cpu0 lld linker, llvm-objdump 
with elf2hex Cpu0 support and Cpu0 Verilog Language is under 10 thousands lines 
of source code include comments. 
The clang, llvm and lld has 1000 thousands lines exclude the
test and documents parts. It is only 1 % of the llvm size. 
Based on this truth, we believe llvm is a well defined structure in compiler 
architecture. 

Finally, 10 thousands lines of source code in Cpu0 backend is very small in UI 
program. But it's quite complex in system program which based on llvm. 
We spent 500 pages of pdf to explain these code. Open source code give 
programmers best opportunity to understand the code and enhance/extend the 
code function. But not enough, we believe the documentation is the next most 
important thing to improve the open source code development. 
The Open Source Fundation recognized this point before us and set 
Open Source Document Project years ago.
We all learned the knowledge through books during school and after school. 
So, if you cannot find a good way to produce documents, you can consider to 
write the document like this book. This book document use sphinx tool just 
like the llvm development team. Appendix A tell you how to install sphinx tool. 
Writing document will help yourself to re-examine your software and make the 
program better in structure and reliability.


.. [#] http://lld.llvm.org/

.. [#] http://lld.llvm.org/getting_started.html#on-unix-like-systems

.. [#] http://www.cplusplus.com/reference/memory/unique_ptr/

.. [#] http://www.cplusplus.com/reference/memory/unique_ptr/get/

.. [#] http://www.cplusplus.com/reference/utility/move/


