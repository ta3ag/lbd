.. _sec-lld:

LLD for Cpu0
==============

This chapter add Cpu0 backend in lld. With this lld Cpu0 for ELF linker support,
the program with global variables can be allocated in ELF file format layout. 
Meaning the relocation records of global variable can be solved. In addition, 
llvm-objdump driver is modified for support generate Hex file from ELF.
With these two tool supported, the program with global variables exist in section
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




.. [#] http://lld.llvm.org/

.. [#] http://lld.llvm.org/getting_started.html#on-unix-like-systems

