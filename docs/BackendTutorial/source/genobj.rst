.. _sec-genobjfiles:

Generating object files
=======================

The previous chapters only introduce the assembly code generated. 
This chapter will introduce you the obj support first, and display the obj by 
objdump utility. With LLVM support, the cpu0 backend can generate both big 
endian and little endian obj files with only a few code added.  
The Target Registration mechanism and their structure will be introduced in 
this chapter.

Translate into obj file
------------------------

Currently, we only support translate llvm IR code into assembly code. 
If you try to run Chapter4_2/ to translate obj code will get the error message as 
follows,

.. code-block:: bash

  [Gamma@localhost 3]$ /usr/local/llvm/test/cmake_debug_build/bin/
  llc -march=cpu0 -relocation-model=pic -filetype=obj ch4_1.bc -o ch4_1.cpu0.o
  /usr/local/llvm/test/cmake_debug_build/bin/llc: target does not 
  support generation of this file type! 
	
The Chapter5_1/ support obj file generated. 
It can get result for big endian and little endian with command 
``llc -march=cpu0`` and ``llc -march=cpu0el``. 
Run it will get the obj files as follows,

.. code-block:: bash

  [Gamma@localhost InputFiles]$ cat ch4_1.cpu0.s 
  ...
    .set  nomacro
  # BB#0:                                 # %entry
    addiu $sp, $sp, -40
  $tmp1:
    .cfi_def_cfa_offset 40
    addiu $2, $zero, 5
    st  $2, 36($fp)
    addiu $2, $zero, 2
    st  $2, 32($fp)
    addiu $2, $zero, 0
    st  $2, 28($fp)
  ...
  
  [Gamma@localhost 3]$ /usr/local/llvm/test/cmake_debug_build/bin/
  llc -march=cpu0 -relocation-model=pic -filetype=obj ch4_1.bc -o ch4_1.cpu0.o
  [Gamma@localhost InputFiles]$ objdump -s ch4_1.cpu0.o 
  
  ch4_1.cpu0.o:     file format elf32-big 
  
  Contents of section .text: 
   0000 09ddffd8 09200005 022b0024 09200002  ..... ...+.$. ..
   0010 022b0020 09200000 022b001c 022b0018  .+. . ...+...+..
   0020 022b0010 0930fffb 023b000c 022b0008  .+...0...;...+..
   0030 022b0004 012b0020 013b0024 11232000  .+...+. .;.$.# .
   0040 022b001c 012b0020 013b0024 12232000  .+...+. .;.$.# .
   0050 022b0018 012b0020 013b0024 17232000  .+...+. .;.$.# .
   0060 022b0018 012b0024 1e220002 022b0014  .+...+.$."...+..
   0070 012b000c 1e220002 022b0008 012b0024  .+..."...+...+.$
   0080 1d220002 022b0010 012b000c 1f220002  ."...+...+..."..
   0090 022b0004 09200001 013b0024 21323000  .+... ...;.$!20.
   00a0 023b0014 013b000c 21223000 022b0008  .;...;..!"0..+..
   00b0 0f208000 013b0024 22223000 022b0010  . ...;.$""0..+..
   00c0 012b0024 013b0020 20232000 022b0004  .+.$.;.  # ..+..
   00d0 09dd0028 2c00000e                    ...(,...        
  Contents of section .eh_frame:
   0000 00000010 00000000 017a5200 017c0e01  .........zR..|..
   0010 1b0c0d00 00000010 00000018 00000000  ................
   0020 000000d8 00440e28                    .....D.(                    
   
  [Gamma@localhost InputFiles]$ /usr/local/llvm/test/
  cmake_debug_build/bin/llc -march=cpu0el -relocation-model=pic -filetype=obj 
  ch4_1.bc -o ch4_1.cpu0el.o 
  [Gamma@localhost InputFiles]$ objdump -s ch4_1.cpu0el.o 
  
  ch4_1.cpu0el.o:     file format elf32-little 
  
  Contents of section .text: 
   0000 d8ffdd09 05002009 24002b02 02002009  ...... .$.+... .
   0010 20002b02 00002009 1c002b02 18002b02   .+... ...+...+.
   0020 10002b02 fbff3009 0c003b02 08002b02  ..+...0...;...+.
   0030 04002b02 20002b01 24003b01 00202311  ..+. .+.$.;.. #.
   0040 1c002b02 20002b01 24003b01 00202312  ..+. .+.$.;.. #.
   0050 18002b02 20002b01 24003b01 00202317  ..+. .+.$.;.. #.
   0060 18002b02 24002b01 0200221e 14002b02  ..+.$.+..."...+.
   0070 0c002b01 0200221e 08002b02 24002b01  ..+..."...+.$.+.
   0080 0200221d 10002b02 0c002b01 0200221f  .."...+...+...".
   0090 04002b02 01002009 24003b01 00303221  ..+... .$.;..02!
   00a0 14003b02 0c003b01 00302221 08002b02  ..;...;..0"!..+.
   00b0 0080200f 24003b01 00302222 10002b02  .. .$.;..0""..+.
   00c0 24002b01 20003b01 00202320 04002b02  $.+. .;.. # ..+.
   00d0 2800dd09 0e00002c                    (......,        
  Contents of section .eh_frame:
   0000 10000000 00000000 017a5200 017c0e01  .........zR..|..
   0010 1b0c0d00 10000000 18000000 00000000  ................
   0020 d8000000 00440e28                    .....D.(        
         

The first instruction is **“addiu  $sp, -40”** and it's corresponding obj is 
0x09ddffd8. 
The addiu opcode is 0x09, 8 bits, $sp register number is 13(0xd), 4bits, and 
the immediate is 16 bits -40(=0xffd8), so it's correct. 
The third instruction **“st  $2, 36($fp)”** and it's corresponding obj 
is 0x022b0024. The **st** opcode is **0x02**, $2 is 0x2, $fp is 0xb and 
immediate is 36(0x0024). 
Thanks to cpu0 instruction format which opcode, register operand and 
offset(imediate value) size are multiple of 4 bits. 
Base on the 4 bits multiple, the obj format is easy to check by eyes. 
The big endian (B0, B1, B2, B3) = (09, dd, ff, d8), objdump from B0 to B3 as 
0x09ddffd8 and the little endian is (B3, B2, B1, B0) = (09, dd, ff, d8), 
objdump from B0 to B3 as 0xd8ffdd09. 


Backend Target Registration Structure
--------------------------------------

To support elf obj generated, the following code changed  and added to 
Chapter5_1.

.. rubric:: lbdex/Chapter5_1/MCTargetDesc/CMakeLists.txt

add_llvm_library(LLVMCpu0Desc
  Cpu0AsmBackend.cpp
  ...
  Cpu0MCCodeEmitter.cpp
  ...
  Cpu0ELFObjectWriter.cpp
  )

.. rubric:: lbdex/Chapter5_1/MCTargetDesc/Cpu0AsmBackend.cpp
.. literalinclude:: ../../../lib/Target/Cpu0/MCTargetDesc/Cpu0AsmBackend.cpp

.. rubric:: lbdex/Chapter5_1/MCTargetDesc/Cpu0BaseInfo.h
.. literalinclude:: ../../../lib/Target/Cpu0/MCTargetDesc/Cpu0BaseInfo.h

.. rubric:: lbdex/Chapter5_1/MCTargetDesc/Cpu0ELFObjectWriter.cpp
.. literalinclude:: ../../../lib/Target/Cpu0/MCTargetDesc/Cpu0ELFObjectWriter.cpp

.. rubric:: lbdex/Chapter5_1/MCTargetDesc/Cpu0MCCodeEmitter.cpp
.. literalinclude:: ../../../lib/Target/Cpu0/MCTargetDesc/Cpu0MCCodeEmitter.cpp
  :end-before: // getBranch16TargetOpValue - Return binary encoding of the branch
.. literalinclude:: ../../../lib/Target/Cpu0/MCTargetDesc/Cpu0MCCodeEmitter.cpp
  :start-after: // lbd document - mark - unsigned getJumpTargetOpValue
  :end-before: /// getBranch16TargetOpValue - Return binary encoding of the branch
.. literalinclude:: ../../../lib/Target/Cpu0/MCTargetDesc/Cpu0MCCodeEmitter.cpp
  :start-after: // lbd document - mark - getJumpTargetOpValue
  :end-before: Cpu0::Fixups FixupKind = Cpu0::Fixups(0);
.. literalinclude:: ../../../lib/Target/Cpu0/MCTargetDesc/Cpu0MCCodeEmitter.cpp
  :start-after: Fixups.push_back(MCFixup::Create(0, MO.getExpr(), MCFixupKind(FixupKind)));

.. rubric:: lbdex/Chapter5_1/MCTargetDesc/Cpu0MCTargetDesc.cpp
.. literalinclude:: ../../../lib/Target/Cpu0/MCTargetDesc/Cpu0MCTargetDesc.cpp
  :start-after: // lbd document - mark - createCpu0MCInstPrinter
  :end-before: // Register the MC asm info.
.. literalinclude:: ../../../lib/Target/Cpu0/MCTargetDesc/Cpu0MCTargetDesc.cpp
  :start-after: TargetRegistry::RegisterMCRegInfo(TheCpu0elTarget, createCpu0MCRegisterInfo);
  :end-before: // Register the MC subtarget info.
.. literalinclude:: ../../../lib/Target/Cpu0/MCTargetDesc/Cpu0MCTargetDesc.cpp
  :start-after: // lbd document - mark - RegisterMCInstPrinter

Now, let's examine Cpu0MCTargetDesc.cpp.
Cpu0MCTargetDesc.cpp do the target registration as mentioned in 
"section Target Registration" [#]_ of the last chapter. 
Drawing the register function and those class it registered in 
:num:`Figure #genobj-f1` to :num:`Figure #genobj-f9` for explanation.

.. _genobj-f1:
.. figure:: ../Fig/genobj/1.png
	:height: 634 px
	:width: 731 px
	:scale: 100 %
	:align: center

	Register Cpu0MCAsmInfo


.. _genobj-f2:
.. figure:: ../Fig/genobj/2.png
	:height: 450 px
	:width: 685 px
	:scale: 100 %
	:align: center

	Register MCCodeGenInfo


.. _genobj-f3:
.. figure:: ../Fig/genobj/3.png
	:height: 313 px
	:width: 606 px
	:scale: 100 %
	:align: center

	Register MCInstrInfo


.. _genobj-f4:
.. figure:: ../Fig/genobj/4.png
	:height: 678 px
	:width: 615 px
	:scale: 100 %
	:align: center

	Register MCRegisterInfo


.. _genobj-f5:
.. figure:: ../Fig/genobj/5.png
	:height: 635 px
	:width: 750 px
	:scale: 100 %
	:align: center

	Register Cpu0MCCodeEmitter


.. _genobj-f6:
.. figure:: ../Fig/genobj/6.png
	:height: 617 px
	:width: 776 px
	:scale: 100 %
	:align: center

	Register MCELFStreamer


.. _genobj-f7:
.. figure:: ../Fig/genobj/7.png
	:height: 570 px
	:width: 810 px
	:scale: 100 %
	:align: center

	Register Cpu0AsmBackend


.. _genobj-f8:
.. figure:: ../Fig/genobj/8.png
	:height: 483 px
	:width: 621 px
	:scale: 100 %
	:align: center

	Register Cpu0MCSubtargetInfo


.. _genobj-f9:
.. figure:: ../Fig/genobj/9.png
	:height: 569 px
	:width: 794 px
	:scale: 100 %
	:align: center

	Register Cpu0InstPrinter


.. _genobj-f10:
.. figure:: ../Fig/genobj/10.png
	:height: 596 px
	:width: 783 px
	:scale: 100 %
	:align: center

	MCELFStreamer inherit tree

In :num:`Figure #genobj-f1`, registering the object of class Cpu0AsmInfo for 
target TheCpu0Target and TheCpu0elTarget. 
TheCpu0Target is for big endian and TheCpu0elTarget is for little endian. 
Cpu0AsmInfo is derived from MCAsmInfo which is llvm built-in class. 
Most code is implemented in it's parent, back end reuse those code by inherit.

In :num:`Figure #genobj-f2`, instancing MCCodeGenInfo, and initialize it by 
pass 
Roloc::PIC because we use command ``llc -relocation-model=pic`` to tell ``llc`` 
compile using position-independent code mode. 
Recall the addressing mode in system program book has two mode, one is PIC 
mode, the other is absolute addressing mode. 
MC stands for Machine Code.

In :num:`Figure #genobj-f3`, instancing MCInstrInfo object X, and initialize it 
by InitCpu0MCInstrInfo(X). 
Since InitCpu0MCInstrInfo(X) is defined in Cpu0GenInstrInfo.inc, it will add 
the information from Cpu0InstrInfo.td we specified. 
:num:`Figure #genobj-f4` is similar to :num:`Figure #genobj-f3`, but it 
initialize the register information specified in Cpu0RegisterInfo.td. 
They share a lot of code with instruction/register td description.

:num:`Figure #genobj-f5`, instancing two objects Cpu0MCCodeEmitter, one is for 
big endian and the other is for little endian. 
They take care the obj format generated. 
So, it's not defined in Chapter4_2/ which support assembly code only.

:num:`Figure #genobj-f6`, MCELFStreamer take care the obj format also. 
:num:`Figure #genobj-f5` Cpu0MCCodeEmitter take care code emitter while 
MCELFStreamer take care the obj output streamer. 
:num:`Figure #genobj-f10` is MCELFStreamer inherit tree. 
You can find a lot of operations in that inherit tree.

Reader maybe has the question for what are the actual arguments in 
createCpu0MCCodeEmitterEB(const MCInstrInfo &MCII,  const MCSubtargetInfo &STI, 
MCContext &Ctx) and at when they are assigned. 
Yes, we didn't assign it, we register the createXXX() function by function 
pointer only (according C, TargetRegistry::RegisterXXX(TheCpu0Target, 
createXXX()) where createXXX is function pointer). 
LLVM keep a function pointer to createXXX() when we call target registry, and 
will call these createXXX() function back at proper time with arguments 
assigned during the target registration process, RegisterXXX().

:num:`Figure #genobj-f7`, Cpu0AsmBackend class is the bridge for asm to obj. 
Two objects take care big endian and little endian also. 
It derived from MCAsmBackend. 
Most of code for object file generated is implemented by MCELFStreamer and it's 
parent, MCAsmBackend.

:num:`Figure #genobj-f8`, instancing MCSubtargetInfo object and initialize with 
Cpu0.td information. 
:num:`Figure #genobj-f9`, instancing Cpu0InstPrinter to take care printing 
function for instructions. 
Like :num:`Figure #genobj-f1` to :num:`Figure #genobj-f4`, it has been defined 
in Chapter4_2/ code for assembly file generated support.



.. [#] http://jonathan2251.github.com/lbd/llvmstructure.html#target-registration
