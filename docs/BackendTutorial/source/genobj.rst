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
   0000 09ddffc8 09200005 022d0034 09200002  ..... ...-.4. ..
   0010 022d0030 0920fffb 022d002c 012d0030  .-.0. ...-.,.-.0
   0020 013d0034 11232000 022d0028 012d0030  .=.4.# ..-.(.-.0
   0030 013d0034 12232000 022d0024 012d0030  .=.4.# ..-.$.-.0
   0040 013d0034 17232000 022d0020 012d0034  .=.4.# ..-. .-.4
   0050 1e220002 022d001c 012d002c 1e220001  ."...-...-.,."..
   0060 022d000c 012d0034 1d220002 022d0018  .-...-.4."...-..
   0070 012d002c 1f22001e 022d0008 09200001  .-.,."...-... ..
   0080 013d0034 21323000 023d0014 013d0030  .=.4!20..=...=.0
   0090 21223000 022d0004 09200080 013d0034  !"0..-... ...=.4
   00a0 22223000 022d0010 012d0034 013d0030  ""0..-...-.4.=.0
   00b0 20232000 022d0000 09dd0038 3ce00000   # ..-.....8<...     
   
  [Gamma@localhost InputFiles]$ /usr/local/llvm/test/
  cmake_debug_build/bin/llc -march=cpu0el -relocation-model=pic -filetype=obj 
  ch4_1.bc -o ch4_1.cpu0el.o 
  [Gamma@localhost InputFiles]$ objdump -s ch4_1.cpu0el.o 
  
  ch4_1.cpu0el.o:     file format elf32-little 
  
  Contents of section .text: 
   0000 c8ffdd09 05002009 34002d02 02002009  ...... .4.-... .
   0010 30002d02 fbff2009 2c002d02 30002d01  0.-... .,.-.0.-.
   0020 34003d01 00202311 28002d02 30002d01  4.=.. #.(.-.0.-.
   0030 34003d01 00202312 24002d02 30002d01  4.=.. #.$.-.0.-.
   0040 34003d01 00202317 20002d02 34002d01  4.=.. #. .-.4.-.
   0050 0200221e 1c002d02 2c002d01 0100221e  .."...-.,.-...".
   0060 0c002d02 34002d01 0200221d 18002d02  ..-.4.-..."...-.
   0070 2c002d01 1e00221f 08002d02 01002009  ,.-..."...-... .
   0080 34003d01 00303221 14003d02 30003d01  4.=..02!..=.0.=.
   0090 00302221 04002d02 80002009 34003d01  .0"!..-... .4.=.
   00a0 00302222 10002d02 34002d01 30003d01  .0""..-.4.-.0.=.
   00b0 00202320 00002d02 3800dd09 0000e03c  . # ..-.8......<      
         

The first instruction is **“addiu  $sp, -56”** and it's corresponding obj is 
0x09ddffc8. 
The addiu opcode is 0x09, 8 bits, $sp register number is 13(0xd), 4bits, and 
the immediate is 16 bits -56(=0xffc8), so it's correct. 
The third instruction **“st  $2, 52($fp)”** and it's corresponding obj 
is 0x022b0034. The **st** opcode is **0x02**, $2 is 0x2, $fp is 0xb and 
immediate is 52(0x0034). 
Thanks to Cpu0 instruction format which opcode, register operand and 
offset(imediate value) size are multiple of 4 bits. 
Base on the 4 bits multiple, the obj format is easy to check by eyes. 
The big endian (B0, B1, B2, B3) = (09, dd, ff, c8), objdump from B0 to B3 as 
0x09ddffc8 and the little endian is (B3, B2, B1, B0) = (09, dd, ff, c8), 
objdump from B0 to B3 as 0xc8ffdd09. 


ELF obj related code
----------------------

To support elf obj generated, the following code changed and added to 
Chapter5_1.

.. rubric:: lbdex/Chapter5_1/MCTargetDesc/CMakeLists.txt
.. code-block:: c++

  add_llvm_library(LLVMCpu0Desc
    Cpu0AsmBackend.cpp
    ...
    Cpu0MCCodeEmitter.cpp
    ...
    Cpu0ELFObjectWriter.cpp
    )

.. rubric:: lbdex/Chapter5_1/MCTargetDesc/Cpu0AsmBackend.cpp
.. literalinclude:: ../../../lib/Target/Cpu0/MCTargetDesc/Cpu0AsmBackend.cpp
  :end-before: case Cpu0::fixup_Cpu0_CALL16:
.. literalinclude:: ../../../lib/Target/Cpu0/MCTargetDesc/Cpu0AsmBackend.cpp
  :start-after: case Cpu0::fixup_Cpu0_CALL16:
  :end-before: case Cpu0::fixup_Cpu0_GOT_LO16:
.. literalinclude:: ../../../lib/Target/Cpu0/MCTargetDesc/Cpu0AsmBackend.cpp
  :start-after: case Cpu0::fixup_Cpu0_PC16:
  :end-before: case Cpu0::fixup_Cpu0_GOT_HI16:
.. literalinclude:: ../../../lib/Target/Cpu0/MCTargetDesc/Cpu0AsmBackend.cpp
  :start-after: case Cpu0::fixup_Cpu0_GOT_HI16:
  :end-before: { "fixup_Cpu0_PC16",           0,     16,  MCFixupKindInfo::FKF_IsPCRel },
.. literalinclude:: ../../../lib/Target/Cpu0/MCTargetDesc/Cpu0AsmBackend.cpp
  :start-after: { "fixup_Cpu0_PC16",           0,     16,  MCFixupKindInfo::FKF_IsPCRel },
  :end-before: { "fixup_Cpu0_CALL16",         0,     16,   0 },
.. literalinclude:: ../../../lib/Target/Cpu0/MCTargetDesc/Cpu0AsmBackend.cpp
  :start-after: { "fixup_Cpu0_CALL16",         0,     16,   0 },

.. rubric:: lbdex/Chapter5_1/MCTargetDesc/Cpu0BaseInfo.h
.. literalinclude:: ../../../lib/Target/Cpu0/MCTargetDesc/Cpu0BaseInfo.h
  :start-after: #define CPU0BASEINFO_H
  :end-before: #include "Cpu0MCTargetDesc.h"
.. literalinclude:: ../../../lib/Target/Cpu0/MCTargetDesc/Cpu0BaseInfo.h
  :start-after: namespace Cpu0II {
  :end-before:  /// MO_GOT_HI16/LO16 - Relocations used for large GOTs.
.. literalinclude:: ../../../lib/Target/Cpu0/MCTargetDesc/Cpu0BaseInfo.h
  :start-after: MO_GOT_LO16,
  :end-before:  enum {
.. code-block:: c++

  inline static std::pair<const MCSymbolRefExpr*, int64_t>
  Cpu0GetSymAndOffset(const MCFixup &Fixup) {
    MCFixupKind FixupKind = Fixup.getKind();

    if ((FixupKind < FirstTargetFixupKind) ||
        (FixupKind >= MCFixupKind(Cpu0::LastTargetFixupKind)))
      return std::make_pair((const MCSymbolRefExpr*)0, (int64_t)0);

    const MCExpr *Expr = Fixup.getValue();
    MCExpr::ExprKind Kind = Expr->getKind();

    if (Kind == MCExpr::Binary) {
      const MCBinaryExpr *BE = static_cast<const MCBinaryExpr*>(Expr);
      const MCExpr *LHS = BE->getLHS();
      const MCConstantExpr *CE = dyn_cast<MCConstantExpr>(BE->getRHS());

      if ((LHS->getKind() != MCExpr::SymbolRef) || !CE)
        return std::make_pair((const MCSymbolRefExpr*)0, (int64_t)0);

      return std::make_pair(cast<MCSymbolRefExpr>(LHS), CE->getValue());
    }

    if (Kind != MCExpr::SymbolRef)
      return std::make_pair((const MCSymbolRefExpr*)0, (int64_t)0);

    return std::make_pair(cast<MCSymbolRefExpr>(Expr), 0);
  } // Cpu0GetSymAndOffset

.. rubric:: lbdex/Chapter5_1/MCTargetDesc/Cpu0ELFObjectWriter.cpp
.. literalinclude:: ../../../lib/Target/Cpu0/MCTargetDesc/Cpu0ELFObjectWriter.cpp
  :end-before: case Cpu0::fixup_Cpu0_CALL16:
.. code-block:: c++

    case Cpu0::fixup_Cpu0_GOT_Global:
    case Cpu0::fixup_Cpu0_GOT_Local:
      Type = ELF::R_CPU0_GOT16;

.. literalinclude:: ../../../lib/Target/Cpu0/MCTargetDesc/Cpu0ELFObjectWriter.cpp
  :start-after: Type = ELF::R_CPU0_GOT16;

.. rubric:: lbdex/Chapter5_1/MCTargetDesc/Cpu0FixupKinds.h
.. literalinclude:: ../../../lib/Target/Cpu0/MCTargetDesc/Cpu0FixupKinds.h
  :end-before: // PC relative branch fixup resulting in - R_CPU0_PC16.
.. literalinclude:: ../../../lib/Target/Cpu0/MCTargetDesc/Cpu0FixupKinds.h
  :start-after:  fixup_Cpu0_PC16,
  :end-before: // resulting in - R_CPU0_CALL16.
.. literalinclude:: ../../../lib/Target/Cpu0/MCTargetDesc/Cpu0FixupKinds.h
  :start-after:  fixup_Cpu0_CALL16,

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

.. rubric:: lbdex/Chapter5_1/MCTargetDesc/Cpu0MCTargetDesc.h
.. literalinclude:: ../../../lib/Target/Cpu0/MCTargetDesc/Cpu0MCTargetDesc.h
  :start-after: class MCObjectWriter;
  :end-before: class MCSubtargetInfo;
.. literalinclude:: ../../../lib/Target/Cpu0/MCTargetDesc/Cpu0MCTargetDesc.h
  :start-after: extern Target TheCpu0elTarget;
  :end-before: MCAsmBackend *createCpu0AsmBackendEB32
.. literalinclude:: ../../../lib/Target/Cpu0/MCTargetDesc/Cpu0MCTargetDesc.h
  :start-after: // lbd document - mark - createCpu0AsmBackendEL32
  :end-before: // Defines symbolic names for Cpu0 registers.


The applyFixup() of Cpu0AsmBackend.cpp will fix up the **jeq**, **jub**, ... 
instruction address in control flow statement or function call statement. 
Although they are not used in here, part of code are added in here of Chapter5_1.

When emit elf obj format instruction, the EncodeInstruction() of 
Cpu0MCCodeEmitter.cpp will be called since it override the same name of 
function in parent class MCCodeEmitter. 
The getMemEncoding() of Cpu0MCCodeEmitter.cpp will be called when generate Cpu0 
**ld** or **st** instructions since the following code defined in 
Cpu0InstrInfo.td.

.. code-block:: c++

  // Address operand
  def mem : Operand<i32> {
    let PrintMethod = "printMemOperand";
    let MIOperandInfo = (ops CPURegs, simm16);
    let EncoderMethod = "getMemEncoding";
  }
  ...
  // Memory Load/Store
  let canFoldAsLoad = 1 in
  class LoadM<bits<8> op, string instr_asm, PatFrag OpNode, RegisterClass RC,
              Operand MemOpnd, bit Pseudo>:
    FMem<op, (outs RC:$ra), (ins MemOpnd:$addr),
       !strconcat(instr_asm, "\t$ra, $addr"),
       [(set RC:$ra, (OpNode addr:$addr))], IILoad> {
    let isPseudo = Pseudo;
  }

  class StoreM<bits<8> op, string instr_asm, PatFrag OpNode, RegisterClass RC,
               Operand MemOpnd, bit Pseudo>:
    FMem<op, (outs), (ins RC:$ra, MemOpnd:$addr),
       !strconcat(instr_asm, "\t$ra, $addr"),
       [(OpNode RC:$ra, addr:$addr)], IIStore> {
    let isPseudo = Pseudo;
  }

  // 32-bit load.
  multiclass LoadM32<bits<8> op, string instr_asm, PatFrag OpNode,
                     bit Pseudo = 0> {
    def #NAME# : LoadM<op, instr_asm, OpNode, CPURegs, mem, Pseudo>;
  }

  // 32-bit store.
  multiclass StoreM32<bits<8> op, string instr_asm, PatFrag OpNode,
                      bit Pseudo = 0> {
    def #NAME# : StoreM<op, instr_asm, OpNode, CPURegs, mem, Pseudo>;
  }

The "let EncoderMethod = "getMemEncoding";" in Cpu0InstrInfo.td as above will 
make the getMemEncoding() is called when Cpu0 **ld** or **st** instructions 
are issued in elf obj since these two instructions use **mem** Operand.

The other functions in Cpu0MCCodeEmitter.cpp are called by these two functions.


Backend Target Registration Structure
--------------------------------------

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

:num:`Figure #genobj-f1` to :num:`Figure #genobj-f4` have been defined 
before in Chapter4_2/ for assembly file generated support.

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
before in Chapter4_2/ code for assembly file generated support.



.. [#] http://jonathan2251.github.io/lbd/llvmstructure.html#target-registration
