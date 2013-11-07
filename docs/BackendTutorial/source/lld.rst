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
  LLVMBackendTutorialExampleCode/3.4_1030_src_files_modify/modify/src/* .
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
  LLVMBackendTutorialExampleCode/3.4_0830_Chapter12_2/* . 
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
  LLVMBackendTutorialExampleCode/Cpu0_lld_1030/Cpu0 .
  [Gamma@localhost ELF]$ cp -f ~/test/lbd/docs/BackendTutorial/
  LLVMBackendTutorialExampleCode/Cpu0_lld_1030/CMakeLists.txt .
  [Gamma@localhost ELF]$ cp -f ~/test/lbd/docs/BackendTutorial/
  LLVMBackendTutorialExampleCode/Cpu0_lld_1030/ELFLinkingContext.cpp .
  [Gamma@localhost ELF]$ cp -f ~/test/lbd/docs/BackendTutorial/
  LLVMBackendTutorialExampleCode/Cpu0_lld_1030/Targets.h .
  [Gamma@localhost ELF]$ cp -f ~/test/lbd/docs/BackendTutorial/
  LLVMBackendTutorialExampleCode/Cpu0_lld_1030/Resolver.cpp ../../Core/.


Finally, update llvm-objdump to support convert ELF file to Hex file as follows,

.. code-block:: bash

  [Gamma@localhost ELF]$ cd ../../../../llvm-objdump/
  [Gamma@localhost llvm-objdump]$ pwd
  /home/Gamma/test/lld/src/tools/llvm-objdump
  [Gamma@localhost llvm-objdump]$ cp -rf ~/test/lbd/docs/BackendTutorial/
  LLVMBackendTutorialExampleCode/llvm-objdump/* .

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


.. rubric:: LLVMBackendTutorialExampleCode/Cpu0_lld_1030/CMakeLists.txt
.. code-block:: c++

  target_link_libraries(lldELF
    ...
    lldCpu0ELFTarget
    )


.. rubric:: LLVMBackendTutorialExampleCode/Cpu0_lld_1030/ELFLinkingContext.cpp
.. code-block:: c++

  uint16_t ELFLinkingContext::getOutputMachine() const {
    switch (getTriple().getArch()) {
    ...
    case llvm::Triple::cpu0:
      return llvm::ELF::EM_CPU0;
    ...
    }
  }

.. rubric:: LLVMBackendTutorialExampleCode/Cpu0_lld_1030/Targets.h
.. code-block:: c++

  #include "Cpu0/Cpu0Target.h"

.. rubric:: LLVMBackendTutorialExampleCode/Cpu0_lld_1030/Resolver.cpp
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

.. rubric:: LLVMBackendTutorialExampleCode/Cpu0_lld_1030/Cpu0/CMakeLists.txt
.. literalinclude:: ../LLVMBackendTutorialExampleCode/Cpu0_lld_1030/Cpu0/CMakeLists.txt

.. rubric:: LLVMBackendTutorialExampleCode/Cpu0_lld_1030/Cpu0/Cpu0LinkingContext.h
.. literalinclude:: ../LLVMBackendTutorialExampleCode/Cpu0_lld_1030/Cpu0/Cpu0LinkingContext.h

.. rubric:: LLVMBackendTutorialExampleCode/Cpu0_lld_1030/Cpu0/Cpu0LinkingContext.cpp
.. literalinclude:: ../LLVMBackendTutorialExampleCode/Cpu0_lld_1030/Cpu0/Cpu0LinkingContext.cpp

.. rubric:: LLVMBackendTutorialExampleCode/Cpu0_lld_1030/Cpu0/Cpu0RelocationHandler.h
.. literalinclude:: ../LLVMBackendTutorialExampleCode/Cpu0_lld_1030/Cpu0/Cpu0RelocationHandler.h

.. rubric:: LLVMBackendTutorialExampleCode/Cpu0_lld_1030/Cpu0/Cpu0RelocationHandler.cpp
.. literalinclude:: ../LLVMBackendTutorialExampleCode/Cpu0_lld_1030/Cpu0/Cpu0RelocationHandler.cpp

.. rubric:: LLVMBackendTutorialExampleCode/Cpu0_lld_1030/Cpu0/Cpu0RelocationPass.h
.. literalinclude:: ../LLVMBackendTutorialExampleCode/Cpu0_lld_1030/Cpu0/Cpu0RelocationPass.h

.. rubric:: LLVMBackendTutorialExampleCode/Cpu0_lld_1030/Cpu0/Cpu0RelocationPass.cpp
.. literalinclude:: ../LLVMBackendTutorialExampleCode/Cpu0_lld_1030/Cpu0/Cpu0RelocationPass.cpp

.. rubric:: LLVMBackendTutorialExampleCode/Cpu0_lld_1030/Cpu0/Cpu0LinkingContext.cpp
.. literalinclude:: ../LLVMBackendTutorialExampleCode/Cpu0_lld_1030/Cpu0/Cpu0LinkingContext.cpp

.. rubric:: LLVMBackendTutorialExampleCode/Cpu0_lld_1030/Cpu0/Cpu0Target.h
.. literalinclude:: ../LLVMBackendTutorialExampleCode/Cpu0_lld_1030/Cpu0/Cpu0Target.h

.. rubric:: LLVMBackendTutorialExampleCode/Cpu0_lld_1030/Cpu0/Cpu0TargetHandler.h
.. literalinclude:: ../LLVMBackendTutorialExampleCode/Cpu0_lld_1030/Cpu0/Cpu0TargetHandler.h

.. rubric:: LLVMBackendTutorialExampleCode/Cpu0_lld_1030/Cpu0/Cpu0TargetHandler.cpp
.. literalinclude:: ../LLVMBackendTutorialExampleCode/Cpu0_lld_1030/Cpu0/Cpu0TargetHandler.cpp


Above code in Cpu0 lld support both the static and dynamic link. 
The "#ifdef DLINKER" is for dynamic link support. There are only just over 1 
thousand of code in it. Half of the code size is for the dynamic linker.


ELF to Hex
-----------

Add elf2hex.h and update llvm-objdump driver to support ELF to Hex for Cpu0 
backend as follows,

.. rubric:: LLVMBackendTutorialExampleCode/llvm-objdump/elf2hex.h
.. literalinclude:: ../LLVMBackendTutorialExampleCode/llvm-objdump/elf2hex.h

.. rubric:: LLVMBackendTutorialExampleCode/llvm-objdump/llvm-objdump.cpp
.. literalinclude:: ../LLVMBackendTutorialExampleCode/llvm-objdump/llvm-objdump.cpp
    :start-after: // 1 llvm-objdump -elf2hex code update begin:
    :end-before: // 1 llvm-objdump -elf2hex code udpate end:
.. literalinclude:: ../LLVMBackendTutorialExampleCode/llvm-objdump/llvm-objdump.cpp
    :start-after: // 2 llvm-objdump -elf2hex code update begin:
    :end-before: // 2 llvm-objdump -elf2hex code udpate end:

The "if (DumpSo)" and "if (LinkSo)" included code are for dynamic linker support.
Others are used in both static and dynamic link execution file dump.


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

.. rubric:: LLVMBackendTutorialExampleCode/InputFiles/printf-stdarg-1.c
.. literalinclude:: ../LLVMBackendTutorialExampleCode/InputFiles/printf-stdarg-1.c
    :start-after: /// start

.. rubric:: LLVMBackendTutorialExampleCode/InputFiles/printf-stdarg-2.c
.. literalinclude:: ../LLVMBackendTutorialExampleCode/InputFiles/printf-stdarg-2.c
    :start-after: /// start

.. rubric:: LLVMBackendTutorialExampleCode/InputFiles/printf-stdarg.c
.. literalinclude:: ../LLVMBackendTutorialExampleCode/InputFiles/printf-stdarg.c
    :start-after: /// start

.. rubric:: LLVMBackendTutorialExampleCode/InputFiles/start.cpp
.. literalinclude:: ../LLVMBackendTutorialExampleCode/InputFiles/start.cpp
    :start-after: /// start

.. rubric:: LLVMBackendTutorialExampleCode/InputFiles/build-printf-stdarg-2.sh
.. literalinclude:: ../LLVMBackendTutorialExampleCode/InputFiles/build-printf-stdarg-2.sh

The cpu0_verilog/cpu0Is.v support cmp instruction and static linker as follows,

.. rubric:: LLVMBackendTutorialExampleCode/cpu0_verilog/cpu0Is.v
.. literalinclude:: ../LLVMBackendTutorialExampleCode/cpu0_verilog/cpu0Is.v

The cpu0_verilog/cpu0IIs.v support slt instruction and static linker as follows,

.. rubric:: LLVMBackendTutorialExampleCode/cpu0_verilog/cpu0IIs.v
.. literalinclude:: ../LLVMBackendTutorialExampleCode/cpu0_verilog/cpu0IIs.v

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

.. rubric:: LLVMBackendTutorialExampleCode/InputFiles/build-printf-stdarg-2.sh

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

The verilog machine cpu032II include cpu0I all instructions (cmp, jeq, ... 
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

  Cpu0 lld class relation ship

.. _lld-f2: 
.. figure:: ../Fig/lld/2.png
  :scale: 80 %
  :align: center

  Cpu0 lld ELFLinkingContext and DefaultLayout member functions

.. _lld-f3: 
.. figure:: ../Fig/lld/3.png
  :scale: 100 %
  :align: center

  Cpu0 lld RelocationPass


The Cpu0LinkingContext include the context information for those obj files you
want to link.
When do linking, the following code will create Cpu0LinkingContext.

.. rubric:: LLVMBackendTutorialExampleCode/Cpu0_lld_1030/ELFLinkingContext.cpp
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

.. rubric:: LLVMBackendTutorialExampleCode/Cpu0_lld_1030/Cpu0/Cpu0LinkingContext.h
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

.. rubric:: LLVMBackendTutorialExampleCode/Cpu0_lld_1030/Cpu0/Cpu0TargetHandler.cpp
.. code-block:: c++

  Cpu0TargetHandler::Cpu0TargetHandler(Cpu0LinkingContext &context)
      : DefaultTargetHandler(context), _gotFile(new GOTFile(context)),
        _relocationHandler(context), _targetLayout(context) {}


According chapter ELF, the linker stands for resolve the relocation records.
The following code give the chance to let lld system call our relocation 
function at proper time.

.. rubric:: LLVMBackendTutorialExampleCode/Cpu0_lld_1030/Cpu0/Cpu0RelocationPass.cpp
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

.. rubric:: LLVMBackendTutorialExampleCode/Cpu0_lld_1030/Cpu0/Cpu0RelocationHandler.cpp
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

.. rubric:: LLVMBackendTutorialExampleCode/Cpu0_lld_1030/Cpu0/Cpu0TargetHandler.h
.. code-block:: c++

  class Cpu0TargetHandler LLVM_FINAL
      : public DefaultTargetHandler<Cpu0ELFType> {
  public:
    ..
    virtual const Cpu0TargetRelocationHandler &getRelocationHandler() const {
      return _relocationHandler;
    }


Dynamic linker 
---------------

Except the lld code with #ifdef DLINKER. The following code in Verilog exist to 
support dynamic linker.

.. rubric:: LLVMBackendTutorialExampleCode/cpu0_verilog/dynlinker.v
.. literalinclude:: ../LLVMBackendTutorialExampleCode/cpu0_verilog/dynlinker.v

.. rubric:: LLVMBackendTutorialExampleCode/cpu0_verilog/flashio.v
.. literalinclude:: ../LLVMBackendTutorialExampleCode/cpu0_verilog/flashio.v

.. rubric:: LLVMBackendTutorialExampleCode/cpu0_verilog/cpu0Id.v
.. literalinclude:: ../LLVMBackendTutorialExampleCode/cpu0_verilog/cpu0Id.v

.. rubric:: LLVMBackendTutorialExampleCode/cpu0_verilog/cpu0IId.v
.. literalinclude:: ../LLVMBackendTutorialExampleCode/cpu0_verilog/cpu0IId.v

The following code ch_dynamiclinker.cpp and foobar.cpp is the example for 
dynamic linker demostration. File dynamic_linker.cpp is what our implementaion
to execute the dynamic linker function on Cpu0 Verilog machine.

.. rubric:: LLVMBackendTutorialExampleCode/InputFiles/dynamic_linker.h
.. literalinclude:: ../LLVMBackendTutorialExampleCode/InputFiles/dynamic_linker.h

.. rubric:: LLVMBackendTutorialExampleCode/InputFiles/dynamic_linker.cpp
.. literalinclude:: ../LLVMBackendTutorialExampleCode/InputFiles/dynamic_linker.cpp
    :start-after: /// start

.. rubric:: LLVMBackendTutorialExampleCode/InputFiles/ch_dynamiclinker.cpp
.. literalinclude:: ../LLVMBackendTutorialExampleCode/InputFiles/ch_dynamiclinker.cpp
    :start-after: /// start

.. rubric:: LLVMBackendTutorialExampleCode/InputFiles/foobar.cpp
.. literalinclude:: ../LLVMBackendTutorialExampleCode/InputFiles/foobar.cpp
    :start-after: /// start


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


Cpu0 lld dynamic linker structure
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~



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
architecture and CPU design can be translated into a real work and see how it be 
run. Now, these school books knowledge is not limited on paper. 
We program it, design it and run it on real world.


.. [#] http://lld.llvm.org/

.. [#] http://lld.llvm.org/getting_started.html#on-unix-like-systems

