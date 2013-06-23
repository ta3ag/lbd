.. _sec-runbackend:

Run backend
===========

This chapter will add LLVM AsmParser support first. 
With AsmParser support, we can hand code the assembly language in C/C++ file 
and translate it into obj (elf format). 
We can write a C++ main 
function as well as the boot code by assembly hand code, and translate this 
main()+bootcode() into obj file.
Combined with llvm-objdump support in last chapter, 
this main()+bootcode() elf can be translated into hex file format which 
include the disassemble code as comment. 
Furthermore, we can design the Cpu0 with Verilog language tool and run the Cpu0 
backend on PC by feed the hex file and see the Cpu0 instructions execution 
result.


AsmParser support
------------------

Run Chapter9_1/ with ch10_1.cpp will get the following error message.

.. rubric:: LLVMBackendTutorialExampleCode/InputFiles/ch10_1.cpp
.. literalinclude:: ../LLVMBackendTutorialExampleCode/InputFiles/ch10_1.cpp
    :lines: 4-
    :linenos:

.. code-block:: bash

  JonathantekiiMac:InputFiles Jonathan$ clang -c ch10_1.cpp -emit-llvm -o 
  ch10_1.bc
  JonathantekiiMac:InputFiles Jonathan$ /Users/Jonathan/llvm/test/cmake_debug_
  build/bin/Debug/llc -march=cpu0 -relocation-model=pic -filetype=obj ch10_1.bc 
  -o ch10_1.cpu0.o
  LLVM ERROR: Inline asm not supported by this streamer because we don't have 
  an asm parser for this target
  
Since we didn't implement cpu0 assembly, it has the error message as above. 
The cpu0 can translate LLVM IR into assembly and obj directly, but it cannot 
translate hand code assembly into obj. 
Directory AsmParser handle the assembly to obj translation.
The Chapter10_1/ include AsmParser implementation as follows,

.. rubric:: LLVMBackendTutorialExampleCode/Chapter10_1/AsmParser/Cpu0AsmParser.cpp
.. literalinclude:: ../LLVMBackendTutorialExampleCode/Chapter10_1/AsmParser/Cpu0AsmParser.cpp
    :linenos:

.. rubric:: LLVMBackendTutorialExampleCode/Chapter10_1/AsmParser/CMakeLists.txt
.. literalinclude:: ../LLVMBackendTutorialExampleCode/Chapter10_1/AsmParser/CMakeLists.txt
    :linenos:

.. rubric:: LLVMBackendTutorialExampleCode/Chapter10_1/AsmParser/LLVMBuild.txt
.. literalinclude:: ../LLVMBackendTutorialExampleCode/Chapter10_1/AsmParser/LLVMBuild.txt
    :linenos:


The Cpu0AsmParser.cpp contains one thousand of code which do the assembly 
language parsing. You can understand it with a little patient only.
To let directory AsmParser be built, modify CMakeLists.txt and LLVMBuild.txt as 
follows,

.. rubric:: LLVMBackendTutorialExampleCode/Chapter10_1/CMakeLists.txt
.. code-block:: c++

  tablegen(LLVM Cpu0GenAsmMatcher.inc -gen-asm-matcher)
  ...
  add_subdirectory(AsmParser)
  
.. rubric:: LLVMBackendTutorialExampleCode/Chapter10_1/LLVMBuild.txt
.. code-block:: c++

  subdirectories = AsmParser ...
  ...
  has_asmparser = 1
  
  
The other files change as follows,

.. rubric:: LLVMBackendTutorialExampleCode/Chapter10_1/MCTargetDesc/Cpu0MCCodeEmitter.cpp
.. code-block:: c++

  unsigned Cpu0MCCodeEmitter::
  getBranchTargetOpValue(const MCInst &MI, unsigned OpNo,
               SmallVectorImpl<MCFixup> &Fixups) const {
    ...
    // If the destination is an immediate, we have nothing to do.
    if (MO.isImm()) return MO.getImm();
    ...
  }
  
  /// getJumpAbsoluteTargetOpValue - Return binary encoding of the jump
  /// target operand. Such as SWI.
  unsigned Cpu0MCCodeEmitter::
  getJumpAbsoluteTargetOpValue(const MCInst &MI, unsigned OpNo,
             SmallVectorImpl<MCFixup> &Fixups) const {
    ...
    // If the destination is an immediate, we have nothing to do.
    if (MO.isImm()) return MO.getImm();
    ...
  }
  
.. rubric:: LLVMBackendTutorialExampleCode/Chapter10_1/Cpu0.td
.. code-block:: c++

  def Cpu0AsmParser : AsmParser {
    let ShouldEmitMatchRegisterName = 0;
  }
  
  def Cpu0AsmParserVariant : AsmParserVariant {
    int Variant = 0;
  
    // Recognize hard coded registers.
    string RegisterPrefix = "$";
  }
  
  def Cpu0 : Target {
    ...
    let AssemblyParsers = [Cpu0AsmParser];
    ...
    let AssemblyParserVariants = [Cpu0AsmParserVariant];
  }
  
.. rubric:: LLVMBackendTutorialExampleCode/Chapter10_1/Cpu0InstrFormats.td
.. code-block:: c++

  // Pseudo-instructions for alternate assembly syntax (never used by codegen).
  // These are aliases that require C++ handling to convert to the target
  // instruction, while InstAliases can be handled directly by tblgen.
  class Cpu0AsmPseudoInst<dag outs, dag ins, string asmstr>:
    Cpu0Inst<outs, ins, asmstr, [], IIPseudo, Pseudo> {
    let isPseudo = 1;
    let Pattern = [];
  }
  
.. rubric:: LLVMBackendTutorialExampleCode/Chapter10_1/Cpu0InstrInfo.td
.. code-block:: c++

  // Cpu0InstrInfo.td
  def Cpu0MemAsmOperand : AsmOperandClass {
    let Name = "Mem";
    let ParserMethod = "parseMemOperand";
  }
  
  // Address operand
  def mem : Operand<i32> {
    ...
    let ParserMatchClass = Cpu0MemAsmOperand;
  }
  ...
  class CmpInstr<...
     !strconcat(instr_asm, "\t$rc, $ra, $rb"), [], itin> {
    ...
  }
  ...
  class CBranch<...
         !strconcat(instr_asm, "\t$ra, $addr"), ...> {
    ...
  }
  ...
  //===----------------------------------------------------------------------===//
  // Pseudo Instruction definition
  //===----------------------------------------------------------------------===//
  
  class LoadImm32< string instr_asm, Operand Od, RegisterClass RC> :
    Cpu0AsmPseudoInst<(outs RC:$ra), (ins Od:$imm32),
             !strconcat(instr_asm, "\t$ra, $imm32")> ;
  def LoadImm32Reg : LoadImm32<"li", shamt, CPURegs>;
  
  class LoadAddress<string instr_asm, Operand MemOpnd, RegisterClass RC> :
    Cpu0AsmPseudoInst<(outs RC:$ra), (ins MemOpnd:$addr),
             !strconcat(instr_asm, "\t$ra, $addr")> ;
  def LoadAddr32Reg : LoadAddress<"la", mem, CPURegs>;
  
  class LoadAddressImm<string instr_asm, Operand Od, RegisterClass RC> :
    Cpu0AsmPseudoInst<(outs RC:$ra), (ins Od:$imm32),
             !strconcat(instr_asm, "\t$ra, $imm32")> ;
  def LoadAddr32Imm : LoadAddressImm<"la", shamt, CPURegs>;


Above define the **ParserMethod = "parseMemOperand"** and implement the 
parseMemOperand() in Cpu0AsmParser.cpp to handle the **"mem"** operand which 
used in ld and st. For example, ld $2, 4($sp), the **mem** operand is 4($sp). 
Accompany with **"let ParserMatchClass = Cpu0MemAsmOperand;"**, 
LLVM will call parseMemOperand() of Cpu0AsmParser.cpp when it meets the assembly 
**mem** operand 4($sp). With above **"let"** assignment, TableGen will generate 
the following structure and functions in Cpu0GenAsmMatcher.inc.

.. rubric:: cmake_debug_build/lib/Target/Cpu0/Cpu0GenAsmMatcher.inc
.. code-block:: c++
  
    enum OperandMatchResultTy {
      MatchOperand_Success,    // operand matched successfully
      MatchOperand_NoMatch,    // operand did not match
      MatchOperand_ParseFail   // operand matched but had errors
    };
    OperandMatchResultTy MatchOperandParserImpl(
      SmallVectorImpl<MCParsedAsmOperand*> &Operands,
      StringRef Mnemonic);
    OperandMatchResultTy tryCustomParseOperand(
      SmallVectorImpl<MCParsedAsmOperand*> &Operands,
      unsigned MCK);
  
  Cpu0AsmParser::OperandMatchResultTy Cpu0AsmParser::
  tryCustomParseOperand(SmallVectorImpl<MCParsedAsmOperand*> &Operands,
              unsigned MCK) {
  
    switch(MCK) {
    case MCK_Mem:
      return parseMemOperand(Operands);
    default:
      return MatchOperand_NoMatch;
    }
    return MatchOperand_NoMatch;
  }
  
  Cpu0AsmParser::OperandMatchResultTy Cpu0AsmParser::
  MatchOperandParserImpl(SmallVectorImpl<MCParsedAsmOperand*> &Operands,
               StringRef Mnemonic) {
    ...
  }
  
  /// MatchClassKind - The kinds of classes which participate in
  /// instruction matching.
  enum MatchClassKind {
    ...
    MCK_Mem, // user defined class 'Cpu0MemAsmOperand'
    ...
  };


Above 3 Pseudo Instruction definitions in Cpu0InstrInfo.td such as 
LoadImm32Reg are handled by Cpu0AsmParser.cpp as follows,

.. rubric:: LLVMBackendTutorialExampleCode/Chapter10_1/AsmParser/Cpu0AsmParser.cpp
.. code-block:: c++
  
  bool Cpu0AsmParser::needsExpansion(MCInst &Inst) {
  
    switch(Inst.getOpcode()) {
    case Cpu0::LoadImm32Reg:
    case Cpu0::LoadAddr32Imm:
    case Cpu0::LoadAddr32Reg:
      return true;
    default:
      return false;
    }
  }
  
  void Cpu0AsmParser::expandInstruction(MCInst &Inst, SMLoc IDLoc,
              SmallVectorImpl<MCInst> &Instructions){
    switch(Inst.getOpcode()) {
    case Cpu0::LoadImm32Reg:
      return expandLoadImm(Inst, IDLoc, Instructions);
    case Cpu0::LoadAddr32Imm:
      return expandLoadAddressImm(Inst,IDLoc,Instructions);
    case Cpu0::LoadAddr32Reg:
      return expandLoadAddressReg(Inst,IDLoc,Instructions);
    }
  }
  
  bool Cpu0AsmParser::
  MatchAndEmitInstruction(SMLoc IDLoc, unsigned &Opcode,
              SmallVectorImpl<MCParsedAsmOperand*> &Operands,
              MCStreamer &Out, unsigned &ErrorInfo,
              bool MatchingInlineAsm) {
    MCInst Inst;
    unsigned MatchResult = MatchInstructionImpl(Operands, Inst, ErrorInfo,
                          MatchingInlineAsm);
  
    switch (MatchResult) {
    default: break;
    case Match_Success: {
    if (needsExpansion(Inst)) {
      SmallVector<MCInst, 4> Instructions;
      expandInstruction(Inst, IDLoc, Instructions);
      ...
    }
    ...
  }


Finally, remind the CPURegs as below must 
follow the order of register number because AsmParser use this when do register 
number encode.

.. rubric:: LLVMBackendTutorialExampleCode/Chapter10_1/Cpu0RegisterInfo.td
.. literalinclude:: ../LLVMBackendTutorialExampleCode/Chapter10_1/Cpu0RegisterInfo.td
    :start-after: //  Registers
    :end-before: // Hi/Lo Registers
    :linenos:


Run Chapter10_1/ with ch10_1.cpp to get the correct result as follows,

.. code-block:: bash

  JonathantekiiMac:InputFiles Jonathan$ /Users/Jonathan/llvm/test/cmake_debug_
  build/bin/Debug/llc -march=cpu0 -relocation-model=pic -filetype=obj ch10_1.bc -o 
  ch10_1.cpu0.o
  JonathantekiiMac:InputFiles Jonathan$ /Users/Jonathan/llvm/test/cmake_debug_
  build/bin/Debug/llvm-objdump -d ch10_1.cpu0.o
  
  ch10_1.cpu0.o:  file format ELF32-unknown
  
  Disassembly of section .text:
  .text:
       0: 00 2d 00 08                                   ld  $2, 8($sp)
       4: 01 0d 00 04                                   st  $zero, 4($sp)
       8: 09 30 00 00                                   addiu $3, $zero, 0
       c: 13 31 20 00                                   add $3, $at, $2
      10: 14 32 30 00                                   sub $3, $2, $3
      14: 15 21 30 00                                   mul $2, $at, $3
      18: 16 32 00 00                                   div $3, $2
      1c: 17 23 00 00                                   divu  $2, $3
      20: 18 21 30 00                                   and $2, $at, $3
      24: 19 31 20 00                                   or  $3, $at, $2
      28: 1a 12 30 00                                   xor $at, $2, $3
      2c: 50 43 00 00                                   mult  $4, $3
      30: 51 32 00 00                                   multu $3, $2
      34: 40 30 00 00                                   mfhi  $3
      38: 41 20 00 00                                   mflo  $2
      3c: 42 20 00 00                                   mthi  $2
      40: 43 20 00 00                                   mtlo  $2
      44: 1b 22 00 02                                   sra $2, $2, 2
      48: 1c 21 10 03                                   rol $2, $at, 3
      4c: 1d 33 10 04                                   ror $3, $3, 4
      50: 1e 22 00 02                                   shl $2, $2, 2
      54: 1f 23 00 05                                   shr $2, $3, 5
      58: 10 23 00 00                                   cmp $zero, $2, $3
      5c: 20 00 00 14                                   jeq $zero, 20
      60: 21 00 00 10                                   jne $zero, 16
      64: 22 ff ff ec                                   jlt $zero, -20
      68: 24 ff ff f0                                   jle $zero, -16
      6c: 23 ff ff fc                                   jgt $zero, -4
      70: 25 ff ff f4                                   jge $zero, -12
      74: 2a 00 04 00                                   swi 1024
      78: 2b 01 00 00                                   jsub  65536
      7c: 2c e0 00 00                                   ret $lr
      80: 2d e6 00 00                                   jalr  $6
      84: 09 30 00 70                                   addiu $3, $zero, 112
      88: 1e 33 00 10                                   shl $3, $3, 16
      8c: 09 10 00 00                                   addiu $at, $zero, 0
      90: 19 33 10 00                                   or  $3, $3, $at
      94: 09 30 00 80                                   addiu $3, $zero, 128
      98: 1e 36 00 10                                   shl $3, $6, 16
      9c: 09 10 00 00                                   addiu $at, $zero, 0
      a0: 19 36 10 00                                   or  $3, $6, $at
      a4: 13 33 60 00                                   add $3, $3, $6
      a8: 09 30 00 90                                   addiu $3, $zero, 144
      ac: 1e 33 00 10                                   shl $3, $3, 16
      b0: 09 10 00 00                                   addiu $at, $zero, 0
      b4: 19 33 10 00                                   or  $3, $3, $at


We replace cmp and jeg with explicit $sw in assembly and $zero in disassembly for 
AsmParser support. It's OK with just a little bad in readability and in assembly 
programing than implicit representation.


Verilog of CPU0
------------------

Verilog language is an IEEE standard in IC design. There are a lot of book and 
documents for this language. Web site [#]_ has a pdf [#]_ in this. 
Example code LLVMBackendTutorialExampleCode/cpu0s_verilog/raw/cpu0s.v is the 
cpu0 design in Verilog. In Appendix A, we have downloaded and installed Icarus 
Verilog tool both on iMac and Linux. The cpu0s.v is a simple design with only 
280 lines of code. Alough it has not the pipeline features, we can assume the 
cpu0 backend code run on the pipeline machine because the pipeline version  
use the same machine instructions. Verilog is C like language in syntex and 
this book is a compiler book, so we list the cpu0s.v as well as the building 
command directly as below. We expect 
readers can understand the Verilog code just with a little patient and no need 
further explanation. There are two type of I/O. One is memory mapped I/O, the 
other is instruction I/O. CPU0 use memory mapped I/O, we set the memory address 
0x7000 as the output port. When meet the instruction **"st $ra, cx($rb)"**, where 
cx($rb) is 0x7000 (28672), CPU0 display the content as follows,

.. code-block:: c++

        ST :
          if (R[b]+c16 == 28672)
            $display("%4dns %8x : %8x OUTPUT=%-d", $stime, pc0, ir, R[a]);


.. rubric:: LLVMBackendTutorialExampleCode/cpu0_verilog/raw/cpu0s.v
.. literalinclude:: ../LLVMBackendTutorialExampleCode/cpu0_verilog/raw/cpu0s.v
    :linenos:


.. code-block:: bash

  JonathantekiiMac:raw Jonathan$ pwd
  /Users/Jonathan/test/2/lbd/LLVMBackendTutorialExampleCode/cpu0_verilog/raw
  JonathantekiiMac:raw Jonathan$ iverilog -o cpu0s cpu0s.v 


Run program on CPU0 machine
---------------------------

Now let's compile ch10_2.cpp as below. Since code size grows up from low to high 
address and stack grows up from high to low address. We set $sp at 0x6ffc because 
cpu0s.v use 0x7000 bytes of memory.

.. rubric:: LLVMBackendTutorialExampleCode/InputFiles/InitRegs.h
.. literalinclude:: ../LLVMBackendTutorialExampleCode/InputFiles/InitRegs.h
    :lines: 5-
    :linenos:

.. rubric:: LLVMBackendTutorialExampleCode/InputFiles/ch10_2.cpp
.. literalinclude:: ../LLVMBackendTutorialExampleCode/InputFiles/ch10_2.cpp
    :lines: 5-
    :linenos:

.. code-block:: bash

  JonathantekiiMac:InputFiles Jonathan$ pwd
  /Users/Jonathan/test/2/lbd/LLVMBackendTutorialExampleCode/InputFiles
  JonathantekiiMac:InputFiles Jonathan$ clang -c ch10_2.cpp -emit-llvm -o 
  ch10_2.bc
  JonathantekiiMac:InputFiles Jonathan$ /Users/Jonathan/llvm/test/cmake_debug_
  build/bin/Debug/llc -march=cpu0 -relocation-model=static -filetype=obj 
  ch10_2.bc -o ch10_2.cpu0.o
  JonathantekiiMac:InputFiles Jonathan$ /Users/Jonathan/llvm/test/cmake_debug_
  build/bin/Debug/llvm-objdump -d ch10_2.cpu0.o | tail -n +6| awk '{print "/* " 
  $1 " */\t" $2 " " $3 " " $4 " " $5 "\t/* " $6"\t" $7" " $8" " $9" " $10 "\t*/"}'
   > ../cpu0_verilog/raw/cpu0s.hex
  
  118-165-81-39:raw Jonathan$ cat cpu0s.hex 
  ...
  /* 4c: */ 2b 00 00 20 /* jsub 0    */
  /* 50: */ 01 2d 00 04 /* st $2, 4($sp)    */
  /* 54: */ 2b 00 01 44 /* jsub 0    */
  
  
As above code the subroutine address for **"jsub #offset"** are 0. 
This is correct since C language support separate compile and the subroutine 
address is decided at link time for static address mode or at 
load time for PIC address mode.
Since our backend didn't implement the linker and loader, we change the  
**"jsub #offset"** encode in Chapter10_2/ as follow,

.. rubric:: LLVMBackendTutorialExampleCode/Chapter10_2/MCTargetDesc/Cpu0MCCodeEmitter.cpp
.. code-block:: c++

  unsigned Cpu0MCCodeEmitter::
  getJumpTargetOpValue(const MCInst &MI, unsigned OpNo,
             SmallVectorImpl<MCFixup> &Fixups) const {
  
    unsigned Opcode = MI.getOpcode();
    ...
    if (Opcode == Cpu0::JSUB)
      Fixups.push_back(MCFixup::Create(0, Expr,
                       MCFixupKind(Cpu0::fixup_Cpu0_PC24)));
    else if (Opcode == Cpu0::JSUB)
      Fixups.push_back(MCFixup::Create(0, Expr,
                       MCFixupKind(Cpu0::fixup_Cpu0_24)));
    else
      llvm_unreachable("unexpect opcode in getJumpAbsoluteTargetOpValue()");
    
    return 0;
  }

We change JSUB from Relocation Records fixup_Cpu0_24 to Non-Relocaton Records 
fixup_Cpu0_PC24 as the definition below. This change is fine since if call a 
outside defined subroutine, it will add a Relocation Record for this 
**"jsub #offset"**. At this point, we set it to Non-Relocaton Records for 
run on CPU0 Verilog machine. If one day, the CPU0 linker is appeared and the 
linker do the sections arrangement, we should adjust it back to Relocation 
Records. A good linker will reorder the sections for optimization in 
data/function access. In other word, 
keep the global variable access as close as possible to reduce cache miss 
possibility.

.. rubric:: LLVMBackendTutorialExampleCode/Chapter10_2/MCTargetDesc/Cpu0AsmBackend.cpp
.. code-block:: c++

    const MCFixupKindInfo &getFixupKindInfo(MCFixupKind Kind) const {
      const static MCFixupKindInfo Infos[Cpu0::NumTargetFixupKinds] = {
        // This table *must* be in same the order of fixup_* kinds in
        // Cpu0FixupKinds.h.
        //
        // name                    offset  bits  flags
        ...
        { "fixup_Cpu0_24",           0,     24,   0 },
        ...
        { "fixup_Cpu0_PC24",         0,     24,  MCFixupKindInfo::FKF_IsPCRel },
        ...
      }
      ...
    }

Let's run the Chapter10_2/ with ``llvm-objdump -d`` again, will get the hex file 
as follows,

.. code-block:: bash

  JonathantekiiMac:InputFiles Jonathan$ pwd
  /Users/Jonathan/test/2/lbd/LLVMBackendTutorialExampleCode/InputFiles
  JonathantekiiMac:InputFiles Jonathan$ clang -c ch10_2.cpp -emit-llvm -o 
  ch10_2.bc
  JonathantekiiMac:InputFiles Jonathan$ /Users/Jonathan/llvm/test/cmake_debug_
  build/bin/Debug/llc -march=cpu0 -relocation-model=static -filetype=obj 
  ch10_2.bc -o ch10_2.cpu0.o
  JonathantekiiMac:InputFiles Jonathan$ /Users/Jonathan/llvm/test/cmake_debug_
  build/bin/Debug/llvm-objdump -d ch10_2.cpu0.o | tail -n +6| awk '{print "/* " 
  $1 " */\t" $2 " " $3 " " $4 " " $5 "\t/* " $6"\t" $7" " $8" " $9" " $10 "\t*/"}'
   > ../cpu0_verilog/raw/cpu0s.hex

.. code-block:: c++
  
  118-165-64-234:raw Jonathan$ cat cpu0s.hex
  /* 0: */  09 10 00 00 /* addiu  $at, $zero, 0   */
  /* 4: */  09 20 00 00 /* addiu  $2, $zero, 0  */
  /* 8: */  09 30 00 00 /* addiu  $3, $zero, 0  */
  /* c: */  09 40 00 00 /* addiu  $4, $zero, 0  */
  /* 10: */ 09 50 00 00 /* addiu  $5, $zero, 0  */
  /* 14: */ 09 60 00 00 /* addiu  $6, $zero, 0  */
  /* 18: */ 09 70 00 00 /* addiu  $7, $zero, 0  */
  /* 1c: */ 09 80 00 00 /* addiu  $8, $zero, 0  */
  /* 20: */ 09 90 00 00 /* addiu  $9, $zero, 0  */
  /* 24: */ 09 a0 00 00 /* addiu  $gp, $zero, 0   */
  /* 28: */ 09 b0 00 00 /* addiu  $fp, $zero, 0   */
  /* 2c: */ 09 c0 00 00 /* addiu  $sw, $zero, 0   */
  /* 30: */ 09 e0 ff ff /* addiu  $lr, $zero, -1  */
  /* 34: */ 09 d0 03 fc /* addiu  $sp, $zero, 1020  */
  /* 38: */ 09 dd ff e0 /* addiu  $sp, $sp, -32   */
  /* 3c: */ 01 ed 00 1c /* st $lr, 28($sp)    */
  /* 40: */ 09 20 00 00 /* addiu  $2, $zero, 0  */
  /* 44: */ 01 2d 00 18 /* st $2, 24($sp)   */
  /* 48: */ 01 2d 00 14 /* st $2, 20($sp)   */
  /* 4c: */ 2b 00 00 34 /* jsub 52    */
  /* 50: */ 01 2d 00 14 /* st $2, 20($sp)   */
  /* 54: */ 01 2d 00 00 /* st $2, 0($sp)    */
  /* 58: */ 2b 00 01 74 /* jsub 372     */
  /* 5c: */ 2b 00 01 94 /* jsub 404     */
  /* 60: */ 00 3d 00 14 /* ld $3, 20($sp)   */
  /* 64: */ 13 23 20 00 /* add  $2, $3, $2  */
  /* 68: */ 01 2d 00 14 /* st $2, 20($sp)   */
  /* 6c: */ 01 2d 00 00 /* st $2, 0($sp)    */
  /* 70: */ 2b 00 01 5c /* jsub 348     */
  /* 74: */ 00 2d 00 14 /* ld $2, 20($sp)   */
  /* 78: */ 00 ed 00 1c /* ld $lr, 28($sp)    */
  /* 7c: */ 09 dd 00 20 /* addiu  $sp, $sp, 32  */
  /* 80: */ 2c 00 00 00 /* ret  $zero     */
  /* 84: */ 09 dd ff a8 /* addiu  $sp, $sp, -88   */
  /* 88: */ 01 ed 00 54 /* st $lr, 84($sp)    */
  /* 8c: */ 01 7d 00 50 /* st $7, 80($sp)   */
  /* 90: */ 09 20 00 0b /* addiu  $2, $zero, 11   */
  /* 94: */ 01 2d 00 4c /* st $2, 76($sp)   */
  /* 98: */ 09 20 00 02 /* addiu  $2, $zero, 2  */
  /* 9c: */ 01 2d 00 48 /* st $2, 72($sp)   */
  /* a0: */ 09 70 00 00 /* addiu  $7, $zero, 0  */
  /* a4: */ 01 7d 00 44 /* st $7, 68($sp)   */
  /* a8: */ 01 7d 00 40 /* st $7, 64($sp)   */
  /* ac: */ 01 7d 00 20 /* st $7, 32($sp)   */
  /* b0: */ 09 20 ff fb /* addiu  $2, $zero, -5   */
  /* b4: */ 01 2d 00 1c /* st $2, 28($sp)   */
  /* b8: */ 01 7d 00 18 /* st $7, 24($sp)   */
  /* bc: */ 00 2d 00 48 /* ld $2, 72($sp)   */
  /* c0: */ 00 3d 00 4c /* ld $3, 76($sp)   */
  /* c4: */ 13 23 20 00 /* add  $2, $3, $2  */
  /* c8: */ 01 2d 00 44 /* st $2, 68($sp)   */
  /* cc: */ 00 2d 00 48 /* ld $2, 72($sp)   */
  /* d0: */ 00 3d 00 4c /* ld $3, 76($sp)   */
  /* d4: */ 14 23 20 00 /* sub  $2, $3, $2  */
  /* d8: */ 01 2d 00 40 /* st $2, 64($sp)   */
  /* dc: */ 00 2d 00 48 /* ld $2, 72($sp)   */
  /* e0: */ 00 3d 00 4c /* ld $3, 76($sp)   */
  /* e4: */ 15 23 20 00 /* mul  $2, $3, $2  */
  /* e8: */ 01 2d 00 3c /* st $2, 60($sp)   */
  /* ec: */ 00 2d 00 48 /* ld $2, 72($sp)   */
  /* f0: */ 00 3d 00 4c /* ld $3, 76($sp)   */
  /* f4: */ 16 32 00 00 /* div  $3, $2    */
  /* f8: */ 41 20 00 00 /* mflo $2    */
  /* fc: */ 09 30 2a aa /* addiu  $3, $zero, 10922  */
  /* 100: */  1e 33 00 10 /* shl  $3, $3, 16  */
  /* 104: */  09 40 aa ab /* addiu  $4, $zero, -21845   */
  /* 108: */  19 33 40 00 /* or $3, $3, $4  */
  /* 10c: */  01 2d 00 38 /* st $2, 56($sp)   */
  /* 110: */  00 2d 00 4c /* ld $2, 76($sp)   */
  /* 114: */  09 22 00 01 /* addiu  $2, $2, 1   */
  /* 118: */  50 23 00 00 /* mult $2, $3    */
  /* 11c: */  40 30 00 00 /* mfhi $3    */
  /* 120: */  1f 43 00 1f /* shr  $4, $3, 31  */
  /* 124: */  1b 33 00 01 /* sra  $3, $3, 1   */
  /* 128: */  13 33 40 00 /* add  $3, $3, $4  */
  /* 12c: */  09 40 00 0c /* addiu  $4, $zero, 12   */
  /* 130: */  15 33 40 00 /* mul  $3, $3, $4  */
  /* 134: */  14 22 30 00 /* sub  $2, $2, $3  */
  /* 138: */  01 2d 00 48 /* st $2, 72($sp)   */
  /* 13c: */  00 3d 00 4c /* ld $3, 76($sp)   */
  /* 140: */  18 23 20 00 /* and  $2, $3, $2  */
  /* 144: */  01 2d 00 34 /* st $2, 52($sp)   */
  /* 148: */  00 2d 00 48 /* ld $2, 72($sp)   */
  /* 14c: */  00 3d 00 4c /* ld $3, 76($sp)   */
  /* 150: */  19 23 20 00 /* or $2, $3, $2  */
  /* 154: */  01 2d 00 30 /* st $2, 48($sp)   */
  /* 158: */  00 2d 00 48 /* ld $2, 72($sp)   */
  /* 15c: */  00 3d 00 4c /* ld $3, 76($sp)   */
  /* 160: */  1a 23 20 00 /* xor  $2, $3, $2  */
  /* 164: */  01 2d 00 2c /* st $2, 44($sp)   */
  /* 168: */  00 2d 00 4c /* ld $2, 76($sp)   */
  /* 16c: */  1e 22 00 02 /* shl  $2, $2, 2   */
  /* 170: */  01 2d 00 28 /* st $2, 40($sp)   */
  /* 174: */  00 2d 00 4c /* ld $2, 76($sp)   */
  /* 178: */  1b 22 00 02 /* sra  $2, $2, 2   */
  /* 17c: */  01 2d 00 24 /* st $2, 36($sp)   */
  /* 180: */  01 2d 00 00 /* st $2, 0($sp)    */
  /* 184: */  2b 00 00 48 /* jsub 72    */
  /* 188: */  00 2d 00 1c /* ld $2, 28($sp)   */
  /* 18c: */  1f 22 00 02 /* shr  $2, $2, 2   */
  /* 190: */  01 2d 00 18 /* st $2, 24($sp)   */
  /* 194: */  01 2d 00 00 /* st $2, 0($sp)    */
  /* 198: */  2b 00 00 34 /* jsub 52    */
  /* 19c: */  00 2d 00 4c /* ld $2, 76($sp)   */
  /* 1a0: */  1a 22 70 00 /* xor  $2, $2, $7  */
  /* 1a4: */  09 30 00 01 /* addiu  $3, $zero, 1  */
  /* 1a8: */  1a 22 30 00 /* xor  $2, $2, $3  */
  /* 1ac: */  18 22 30 00 /* and  $2, $2, $3  */
  /* 1b0: */  01 2d 00 48 /* st $2, 72($sp)   */
  /* 1b4: */  09 2d 00 48 /* addiu  $2, $sp, 72   */
  /* 1b8: */  01 2d 00 10 /* st $2, 16($sp)   */
  /* 1bc: */  00 2d 00 44 /* ld $2, 68($sp)   */
  /* 1c0: */  00 7d 00 50 /* ld $7, 80($sp)   */
  /* 1c4: */  00 ed 00 54 /* ld $lr, 84($sp)    */
  /* 1c8: */  09 dd 00 58 /* addiu  $sp, $sp, 88  */
  /* 1cc: */  2c 00 00 00 /* ret  $zero     */
  /* 1d0: */  09 dd ff f8 /* addiu  $sp, $sp, -8  */
  /* 1d4: */  00 2d 00 08 /* ld $2, 8($sp)    */
  /* 1d8: */  01 2d 00 04 /* st $2, 4($sp)    */
  /* 1dc: */  09 20 70 00 /* addiu  $2, $zero, 28672  */
  /* 1e0: */  01 2d 00 00 /* st $2, 0($sp)    */
  /* 1e4: */  00 3d 00 04 /* ld $3, 4($sp)    */
  /* 1e8: */  01 32 00 00 /* st $3, 0($2)   */
  /* 1ec: */  09 dd 00 08 /* addiu  $sp, $sp, 8   */
  /* 1f0: */  2c 00 00 00 /* ret  $zero     */
  /* 1f4: */  09 dd ff e8 /* addiu  $sp, $sp, -24   */
  /* 1f8: */  09 30 00 01 /* addiu  $3, $zero, 1  */
  /* 1fc: */  01 3d 00 14 /* st $3, 20($sp)   */
  /* 200: */  09 20 00 02 /* addiu  $2, $zero, 2  */
  /* 204: */  01 2d 00 10 /* st $2, 16($sp)   */
  /* 208: */  09 20 00 03 /* addiu  $2, $zero, 3  */
  /* 20c: */  01 2d 00 0c /* st $2, 12($sp)   */
  /* 210: */  09 20 00 04 /* addiu  $2, $zero, 4  */
  /* 214: */  01 2d 00 08 /* st $2, 8($sp)    */
  /* 218: */  09 20 00 05 /* addiu  $2, $zero, 5  */
  /* 21c: */  01 2d 00 04 /* st $2, 4($sp)    */
  /* 220: */  09 20 00 00 /* addiu  $2, $zero, 0  */
  /* 224: */  00 4d 00 14 /* ld $4, 20($sp)   */
  /* 228: */  10 42 00 00 /* cmp  $zero, $4, $2   */
  /* 22c: */  20 00 00 10 /* jeq  $zero, 16   */
  /* 230: */  26 00 00 00 /* jmp  0     */
  /* 234: */  00 4d 00 14 /* ld $4, 20($sp)   */
  /* 238: */  09 44 00 01 /* addiu  $4, $4, 1   */
  /* 23c: */  01 4d 00 14 /* st $4, 20($sp)   */
  /* 240: */  00 4d 00 10 /* ld $4, 16($sp)   */
  /* 244: */  10 43 00 00 /* cmp  $zero, $4, $3   */
  /* 248: */  22 00 00 10 /* jlt  $zero, 16   */
  /* 24c: */  26 00 00 00 /* jmp  0     */
  /* 250: */  00 3d 00 10 /* ld $3, 16($sp)   */
  /* 254: */  09 33 00 01 /* addiu  $3, $3, 1   */
  /* 258: */  01 3d 00 10 /* st $3, 16($sp)   */
  /* 25c: */  00 3d 00 0c /* ld $3, 12($sp)   */
  /* 260: */  10 32 00 00 /* cmp  $zero, $3, $2   */
  /* 264: */  22 00 00 10 /* jlt  $zero, 16   */
  /* 268: */  26 00 00 00 /* jmp  0     */
  /* 26c: */  00 3d 00 0c /* ld $3, 12($sp)   */
  /* 270: */  09 33 00 01 /* addiu  $3, $3, 1   */
  /* 274: */  01 3d 00 0c /* st $3, 12($sp)   */
  /* 278: */  09 30 ff ff /* addiu  $3, $zero, -1   */
  /* 27c: */  00 4d 00 08 /* ld $4, 8($sp)    */
  /* 280: */  10 43 00 00 /* cmp  $zero, $4, $3   */
  /* 284: */  23 00 00 10 /* jgt  $zero, 16   */
  /* 288: */  26 00 00 00 /* jmp  0     */
  /* 28c: */  00 3d 00 08 /* ld $3, 8($sp)    */
  /* 290: */  09 33 00 01 /* addiu  $3, $3, 1   */
  /* 294: */  01 3d 00 08 /* st $3, 8($sp)    */
  /* 298: */  00 3d 00 04 /* ld $3, 4($sp)    */
  /* 29c: */  10 32 00 00 /* cmp  $zero, $3, $2   */
  /* 2a0: */  23 00 00 10 /* jgt  $zero, 16   */
  /* 2a4: */  26 00 00 00 /* jmp  0     */
  /* 2a8: */  00 2d 00 04 /* ld $2, 4($sp)    */
  /* 2ac: */  09 22 00 01 /* addiu  $2, $2, 1   */
  /* 2b0: */  01 2d 00 04 /* st $2, 4($sp)    */
  /* 2b4: */  00 2d 00 10 /* ld $2, 16($sp)   */
  /* 2b8: */  00 3d 00 14 /* ld $3, 20($sp)   */
  /* 2bc: */  13 23 20 00 /* add  $2, $3, $2  */
  /* 2c0: */  00 3d 00 0c /* ld $3, 12($sp)   */
  /* 2c4: */  13 22 30 00 /* add  $2, $2, $3  */
  /* 2c8: */  00 3d 00 08 /* ld $3, 8($sp)    */
  /* 2cc: */  13 22 30 00 /* add  $2, $2, $3  */
  /* 2d0: */  00 3d 00 04 /* ld $3, 4($sp)    */
  /* 2d4: */  13 22 30 00 /* add  $2, $2, $3  */
  /* 2d8: */  09 dd 00 18 /* addiu  $sp, $sp, 24  */
  /* 2dc: */  2c 00 00 00 /* ret  $zero     */
  /* 2e0: */  09 dd ff f8 /* addiu  $sp, $sp, -8  */
  /* 2e4: */  00 2d 00 08 /* ld $2, 8($sp)    */
  /* 2e8: */  01 2d 00 04 /* st $2, 4($sp)    */
  /* 2ec: */  00 1d 00 08 /* ld $at, 8($sp)   */
  /* 2f0: */  01 10 70 00 /* st $at, 28672($zero)   */
  /* 2f4: */  09 dd 00 08 /* addiu  $sp, $sp, 8   */
  /* 2f8: */  2c 00 00 00 /* ret  $zero     */

From above result, you can find the print_integer() which implemented by C 
language has 8 instructions while the print1_integer() which implemented by 
assembly has 6 instructions. But the C version is better in portability since 
the assembly is binding with machine assembly language and make the 
assumption that the stack size of print1_integer() is 8.
Now, run the cpu0 backend to get the result as follows,

.. code-block:: bash

  118-165-64-234:raw Jonathan$ ./cpu0s
  WARNING: cpu0s.v:219: $readmemh(cpu0s.hex): Not enough words in the file for 
  the requested range [0:1024].
  00000000: 09100000
  00000004: 09200000
  00000008: 09300000
  0000000c: 09400000
  00000010: 09500000
  00000014: 09600000
  00000018: 09700000
  0000001c: 09800000
  00000020: 09900000
  00000024: 09a00000
  00000028: 09b00000
  0000002c: 09c00000
  00000030: 09e0ffff
  00000034: 09d003fc
  00000038: 09ddffe0
  0000003c: 01ed001c
  00000040: 09200000
  00000044: 012d0018
  00000048: 012d0014
  0000004c: 2b000034
  00000050: 012d0014
  00000054: 012d0000
  00000058: 2b000174
  0000005c: 2b000194
  00000060: 003d0014
  00000064: 13232000
  00000068: 012d0014
  0000006c: 012d0000
  00000070: 2b00015c
  00000074: 002d0014
  00000078: 00ed001c
  0000007c: 09dd0020
  00000080: 2c000000
  00000084: 09ddffa8
  00000088: 01ed0054
  0000008c: 017d0050
  00000090: 0920000b
  00000094: 012d004c
  00000098: 09200002
  0000009c: 012d0048
  000000a0: 09700000
  000000a4: 017d0044
  000000a8: 017d0040
  000000ac: 017d0020
  000000b0: 0920fffb
  000000b4: 012d001c
  000000b8: 017d0018
  000000bc: 002d0048
  000000c0: 003d004c
  000000c4: 13232000
  000000c8: 012d0044
  000000cc: 002d0048
  000000d0: 003d004c
  000000d4: 14232000
  000000d8: 012d0040
  000000dc: 002d0048
  000000e0: 003d004c
  000000e4: 15232000
  000000e8: 012d003c
  000000ec: 002d0048
  000000f0: 003d004c
  000000f4: 16320000
  000000f8: 41200000
  000000fc: 09302aaa
  00000100: 1e330010
  00000104: 0940aaab
  00000108: 19334000
  0000010c: 012d0038
  00000110: 002d004c
  00000114: 09220001
  00000118: 50230000
  0000011c: 40300000
  00000120: 1f43001f
  00000124: 1b330001
  00000128: 13334000
  0000012c: 0940000c
  00000130: 15334000
  00000134: 14223000
  00000138: 012d0048
  0000013c: 003d004c
  00000140: 18232000
  00000144: 012d0034
  00000148: 002d0048
  0000014c: 003d004c
  00000150: 19232000
  00000154: 012d0030
  00000158: 002d0048
  0000015c: 003d004c
  00000160: 1a232000
  00000164: 012d002c
  00000168: 002d004c
  0000016c: 1e220002
  00000170: 012d0028
  00000174: 002d004c
  00000178: 1b220002
  0000017c: 012d0024
  00000180: 012d0000
  00000184: 2b000048
  00000188: 002d001c
  0000018c: 1f220002
  00000190: 012d0018
  00000194: 012d0000
  00000198: 2b000034
  0000019c: 002d004c
  000001a0: 1a227000
  000001a4: 09300001
  000001a8: 1a223000
  000001ac: 18223000
  000001b0: 012d0048
  000001b4: 092d0048
  000001b8: 012d0010
  000001bc: 002d0044
  000001c0: 007d0050
  000001c4: 00ed0054
  000001c8: 09dd0058
  000001cc: 2c000000
  000001d0: 09ddfff8
  000001d4: 002d0008
  000001d8: 012d0004
  000001dc: 09207000
  000001e0: 012d0000
  000001e4: 003d0004
  000001e8: 01320000
  000001ec: 09dd0008
  000001f0: 2c000000
  000001f4: 09ddffe8
  000001f8: 09300001
  000001fc: 013d0014
  00000200: 09200002
  00000204: 012d0010
  00000208: 09200003
  0000020c: 012d000c
  00000210: 09200004
  00000214: 012d0008
  00000218: 09200005
  0000021c: 012d0004
  00000220: 09200000
  00000224: 004d0014
  00000228: 10420000
  0000022c: 20000010
  00000230: 26000000
  00000234: 004d0014
  00000238: 09440001
  0000023c: 014d0014
  00000240: 004d0010
  00000244: 10430000
  00000248: 22000010
  0000024c: 26000000
  00000250: 003d0010
  00000254: 09330001
  00000258: 013d0010
  0000025c: 003d000c
  00000260: 10320000
  00000264: 22000010
  00000268: 26000000
  0000026c: 003d000c
  00000270: 09330001
  00000274: 013d000c
  00000278: 0930ffff
  0000027c: 004d0008
  00000280: 10430000
  00000284: 23000010
  00000288: 26000000
  0000028c: 003d0008
  00000290: 09330001
  00000294: 013d0008
  00000298: 003d0004
  0000029c: 10320000
  000002a0: 23000010
  000002a4: 26000000
  000002a8: 002d0004
  000002ac: 09220001
  000002b0: 012d0004
  000002b4: 002d0010
  000002b8: 003d0014
  000002bc: 13232000
  000002c0: 003d000c
  000002c4: 13223000
  000002c8: 003d0008
  000002cc: 13223000
  000002d0: 003d0004
  000002d4: 13223000
  000002d8: 09dd0018
  000002dc: 2c000000
  000002e0: 09ddfff8
  000002e4: 002d0008
  000002e8: 012d0004
  000002ec: 001d0008
  000002f0: 01107000
  000002f4: 09dd0008
  000002f8: 2c000000
    90ns 00000000 : 09100000 R[01]=00000000=0          SW=00000000
   170ns 00000004 : 09200000 R[02]=00000000=0          SW=00000000
   250ns 00000008 : 09300000 R[03]=00000000=0          SW=00000000
   330ns 0000000c : 09400000 R[04]=00000000=0          SW=00000000
   410ns 00000010 : 09500000 R[05]=00000000=0          SW=00000000
   490ns 00000014 : 09600000 R[06]=00000000=0          SW=00000000
   570ns 00000018 : 09700000 R[07]=00000000=0          SW=00000000
   650ns 0000001c : 09800000 R[08]=00000000=0          SW=00000000
   730ns 00000020 : 09900000 R[09]=00000000=0          SW=00000000
   810ns 00000024 : 09a00000 R[10]=00000000=0          SW=00000000
   890ns 00000028 : 09b00000 R[11]=00000000=0          SW=00000000
   970ns 0000002c : 09c00000 R[12]=00000000=0          SW=00000000
  1050ns 00000030 : 09e0ffff R[14]=ffffffff=-1         SW=00000000
  1130ns 00000034 : 09d003fc R[13]=000003fc=1020       SW=00000000
  1210ns 00000038 : 09ddffe0 R[13]=000003dc=988        SW=00000000
  1370ns 00000040 : 09200000 R[02]=00000000=0          SW=00000000
  1610ns 0000004c : 2b000034 R[00]=00000000=0          SW=00000000
  1690ns 00000084 : 09ddffa8 R[13]=00000384=900        SW=00000000
  1930ns 00000090 : 0920000b R[02]=0000000b=11         SW=00000000
  2090ns 00000098 : 09200002 R[02]=00000002=2          SW=00000000
  2250ns 000000a0 : 09700000 R[07]=00000000=0          SW=00000000
  2570ns 000000b0 : 0920fffb R[02]=fffffffb=-5         SW=00000000
  2810ns 000000bc : 002d0048 R[02]=00000002=2          SW=00000000
  2890ns 000000c0 : 003d004c R[03]=0000000b=11         SW=00000000
  2970ns 000000c4 : 13232000 R[02]=0000000d=13         SW=00000000
  3130ns 000000cc : 002d0048 R[02]=00000002=2          SW=00000000
  3210ns 000000d0 : 003d004c R[03]=0000000b=11         SW=00000000
  3290ns 000000d4 : 14232000 R[02]=00000009=9          SW=00000000
  3450ns 000000dc : 002d0048 R[02]=00000002=2          SW=00000000
  3530ns 000000e0 : 003d004c R[03]=0000000b=11         SW=00000000
  3610ns 000000e4 : 15232000 R[02]=00000016=22         SW=00000000
  3770ns 000000ec : 002d0048 R[02]=00000002=2          SW=00000000
  3850ns 000000f0 : 003d004c R[03]=0000000b=11         SW=00000000
  3930ns 000000f4 : 16320000 HI=00000001 LO=00000005 SW=00000000
  4010ns 000000f8 : 41200000 R[02]=00000005=5          SW=00000000
  4090ns 000000fc : 09302aaa R[03]=00002aaa=10922      SW=00000000
  4170ns 00000100 : 1e330010 R[03]=2aaa0000=715784192  SW=00000000
  4250ns 00000104 : 0940aaab R[04]=ffffaaab=-21845     SW=00000000
  4330ns 00000108 : 19334000 R[03]=ffffaaab=-21845     SW=00000000
  4490ns 00000110 : 002d004c R[02]=0000000b=11         SW=00000000
  4570ns 00000114 : 09220001 R[02]=0000000c=12         SW=00000000
  4650ns 00000118 : 50230000 HI=ffffffff LO=fffc0004 SW=00000000
  4730ns 0000011c : 40300000 R[03]=ffffffff=-1         SW=00000000
  4810ns 00000120 : 1f43001f R[04]=00000001=1          SW=00000000
  4890ns 00000124 : 1b330001 R[03]=ffffffff=-1         SW=00000000
  4970ns 00000128 : 13334000 R[03]=00000000=0          SW=00000000
  5050ns 0000012c : 0940000c R[04]=0000000c=12         SW=00000000
  5130ns 00000130 : 15334000 R[03]=00000000=0          SW=00000000
  5210ns 00000134 : 14223000 R[02]=0000000c=12         SW=00000000
  5370ns 0000013c : 003d004c R[03]=0000000b=11         SW=00000000
  5450ns 00000140 : 18232000 R[02]=00000008=8          SW=00000000
  5610ns 00000148 : 002d0048 R[02]=0000000c=12         SW=00000000
  5690ns 0000014c : 003d004c R[03]=0000000b=11         SW=00000000
  5770ns 00000150 : 19232000 R[02]=0000000f=15         SW=00000000
  5930ns 00000158 : 002d0048 R[02]=0000000c=12         SW=00000000
  6010ns 0000015c : 003d004c R[03]=0000000b=11         SW=00000000
  6090ns 00000160 : 1a232000 R[02]=00000007=7          SW=00000000
  6250ns 00000168 : 002d004c R[02]=0000000b=11         SW=00000000
  6330ns 0000016c : 1e220002 R[02]=0000002c=44         SW=00000000
  6490ns 00000174 : 002d004c R[02]=0000000b=11         SW=00000000
  6570ns 00000178 : 1b220002 R[02]=00000002=2          SW=00000000
  6810ns 00000184 : 2b000048 R[00]=00000000=0          SW=00000000
  6890ns 000001d0 : 09ddfff8 R[13]=0000037c=892        SW=00000000
  6970ns 000001d4 : 002d0008 R[02]=00000002=2          SW=00000000
  7130ns 000001dc : 09207000 R[02]=00007000=28672      SW=00000000
  7290ns 000001e4 : 003d0004 R[03]=00000002=2          SW=00000000
  7370ns 000001e8 : 01320000 OUTPUT=2         
  7450ns 000001ec : 09dd0008 R[13]=00000384=900        SW=00000000
  7530ns 000001f0 : 2c000000 R[00]=00000000=0          SW=00000000
  7610ns 00000188 : 002d001c R[02]=fffffffb=-5         SW=00000000
  7690ns 0000018c : 1f220002 R[02]=3ffffffe=1073741822 SW=00000000
  7930ns 00000198 : 2b000034 R[00]=00000000=0          SW=00000000
  8010ns 000001d0 : 09ddfff8 R[13]=0000037c=892        SW=00000000
  8090ns 000001d4 : 002d0008 R[02]=3ffffffe=1073741822 SW=00000000
  8250ns 000001dc : 09207000 R[02]=00007000=28672      SW=00000000
  8410ns 000001e4 : 003d0004 R[03]=3ffffffe=1073741822 SW=00000000
  8490ns 000001e8 : 01320000 OUTPUT=1073741822
  8570ns 000001ec : 09dd0008 R[13]=00000384=900        SW=00000000
  8650ns 000001f0 : 2c000000 R[00]=00000000=0          SW=00000000
  8730ns 0000019c : 002d004c R[02]=0000000b=11         SW=00000000
  8810ns 000001a0 : 1a227000 R[02]=0000000b=11         SW=00000000
  8890ns 000001a4 : 09300001 R[03]=00000001=1          SW=00000000
  8970ns 000001a8 : 1a223000 R[02]=0000000a=10         SW=00000000
  9050ns 000001ac : 18223000 R[02]=00000000=0          SW=00000000
  9210ns 000001b4 : 092d0048 R[02]=000003cc=972        SW=00000000
  9370ns 000001bc : 002d0044 R[02]=0000000d=13         SW=00000000
  9450ns 000001c0 : 007d0050 R[07]=00000000=0          SW=00000000
  9530ns 000001c4 : 00ed0054 R[14]=00000050=80         SW=00000000
  9610ns 000001c8 : 09dd0058 R[13]=000003dc=988        SW=00000000
  9690ns 000001cc : 2c000000 R[00]=00000000=0          SW=00000000
  9930ns 00000058 : 2b000174 R[00]=00000000=0          SW=00000000
  10010ns 000001d0 : 09ddfff8 R[13]=000003d4=980        SW=00000000
  10090ns 000001d4 : 002d0008 R[02]=0000000d=13         SW=00000000
  10250ns 000001dc : 09207000 R[02]=00007000=28672      SW=00000000
  10410ns 000001e4 : 003d0004 R[03]=0000000d=13         SW=00000000
  10490ns 000001e8 : 01320000 OUTPUT=13        
  10570ns 000001ec : 09dd0008 R[13]=000003dc=988        SW=00000000
  10650ns 000001f0 : 2c000000 R[00]=00000000=0          SW=00000000
  10730ns 0000005c : 2b000194 R[00]=00000000=0          SW=00000000
  10810ns 000001f4 : 09ddffe8 R[13]=000003c4=964        SW=00000000
  10890ns 000001f8 : 09300001 R[03]=00000001=1          SW=00000000
  11050ns 00000200 : 09200002 R[02]=00000002=2          SW=00000000
  11210ns 00000208 : 09200003 R[02]=00000003=3          SW=00000000
  11370ns 00000210 : 09200004 R[02]=00000004=4          SW=00000000
  11530ns 00000218 : 09200005 R[02]=00000005=5          SW=00000000
  11690ns 00000220 : 09200000 R[02]=00000000=0          SW=00000000
  11770ns 00000224 : 004d0014 R[04]=00000001=1          SW=00000000
  11850ns 00000228 : 10420000 R[04]=00000001=1          SW=00000000
  11930ns 0000022c : 20000010 R[00]=00000000=0          SW=00000000
  12010ns 00000230 : 26000000 R[00]=00000000=0          SW=00000000
  12090ns 00000234 : 004d0014 R[04]=00000001=1          SW=00000000
  12170ns 00000238 : 09440001 R[04]=00000002=2          SW=00000000
  12330ns 00000240 : 004d0010 R[04]=00000002=2          SW=00000000
  12410ns 00000244 : 10430000 R[04]=00000002=2          SW=00000000
  12490ns 00000248 : 22000010 R[00]=00000000=0          SW=00000000
  12570ns 0000024c : 26000000 R[00]=00000000=0          SW=00000000
  12650ns 00000250 : 003d0010 R[03]=00000002=2          SW=00000000
  12730ns 00000254 : 09330001 R[03]=00000003=3          SW=00000000
  12890ns 0000025c : 003d000c R[03]=00000003=3          SW=00000000
  12970ns 00000260 : 10320000 R[03]=00000003=3          SW=00000000
  13050ns 00000264 : 22000010 R[00]=00000000=0          SW=00000000
  13130ns 00000268 : 26000000 R[00]=00000000=0          SW=00000000
  13210ns 0000026c : 003d000c R[03]=00000003=3          SW=00000000
  13290ns 00000270 : 09330001 R[03]=00000004=4          SW=00000000
  13450ns 00000278 : 0930ffff R[03]=ffffffff=-1         SW=00000000
  13530ns 0000027c : 004d0008 R[04]=00000004=4          SW=00000000
  13610ns 00000280 : 10430000 R[04]=00000004=4          SW=00000000
  13690ns 00000284 : 23000010 R[00]=00000000=0          SW=00000000
  13770ns 00000298 : 003d0004 R[03]=00000005=5          SW=00000000
  13850ns 0000029c : 10320000 R[03]=00000005=5          SW=00000000
  13930ns 000002a0 : 23000010 R[00]=00000000=0          SW=00000000
  14010ns 000002b4 : 002d0010 R[02]=00000003=3          SW=00000000
  14090ns 000002b8 : 003d0014 R[03]=00000002=2          SW=00000000
  14170ns 000002bc : 13232000 R[02]=00000005=5          SW=00000000
  14250ns 000002c0 : 003d000c R[03]=00000004=4          SW=00000000
  14330ns 000002c4 : 13223000 R[02]=00000009=9          SW=00000000
  14410ns 000002c8 : 003d0008 R[03]=00000004=4          SW=00000000
  14490ns 000002cc : 13223000 R[02]=0000000d=13         SW=00000000
  14570ns 000002d0 : 003d0004 R[03]=00000005=5          SW=00000000
  14650ns 000002d4 : 13223000 R[02]=00000012=18         SW=00000000
  14730ns 000002d8 : 09dd0018 R[13]=000003dc=988        SW=00000000
  14810ns 000002dc : 2c000000 R[00]=00000000=0          SW=00000000
  14890ns 00000060 : 003d0014 R[03]=0000000d=13         SW=00000000
  14970ns 00000064 : 13232000 R[02]=0000001f=31         SW=00000000
  15210ns 00000070 : 2b00015c R[00]=00000000=0          SW=00000000
  15290ns 000001d0 : 09ddfff8 R[13]=000003d4=980        SW=00000000
  15370ns 000001d4 : 002d0008 R[02]=0000001f=31         SW=00000000
  15530ns 000001dc : 09207000 R[02]=00007000=28672      SW=00000000
  15690ns 000001e4 : 003d0004 R[03]=0000001f=31         SW=00000000
  15770ns 000001e8 : 01320000 OUTPUT=31        
  15850ns 000001ec : 09dd0008 R[13]=000003dc=988        SW=00000000
  15930ns 000001f0 : 2c000000 R[00]=00000000=0          SW=00000000
  16010ns 00000074 : 002d0014 R[02]=0000001f=31         SW=00000000
  16090ns 00000078 : 00ed001c R[14]=ffffffff=-1         SW=00000000
  16170ns 0000007c : 09dd0020 R[13]=000003fc=1020       SW=00000000
  16250ns 00000080 : 2c000000 R[00]=00000000=0          SW=00000000
  RET to PC < 0, finished!

As above result, cpu0s.v dump the memory first after read input cpu0s.hex. 
Next, it run instructions from address 0 and print each destination 
register value in the fourth column. 
The first column is the nano seconds of timing. The second 
is instruction address. The third is instruction content. 
We have checked the **">>"** is correct on both signed and unsigned int type 
, and tracking the variable **a** value by print_integer(). You can verify it 
with the **OUTPUT=xxx** in Verilog output.

Now, let's run ch_10_3.cpp to verify the result as follows,

.. rubric:: LLVMBackendTutorialExampleCode/InputFiles/ch10_3.cpp
.. literalinclude:: ../LLVMBackendTutorialExampleCode/InputFiles/ch10_3.cpp
    :lines: 6-
    :linenos:

.. code-block:: bash

  118-165-75-175:InputFiles Jonathan$ clang -target `llvm-config --host-target` 
  -c ch10_3.cpp -emit-llvm -o ch10_3.bc
  118-165-75-175:InputFiles Jonathan$ /Users/Jonathan/llvm/test/cmake_debug_build
  /bin/Debug/llc -march=cpu0 -relocation-model=static -filetype=obj ch10_3.bc 
  -o ch10_3.cpu0.o
  118-165-75-175:InputFiles Jonathan$ /Users/Jonathan/llvm/test/cmake_debug_build
  /bin/Debug/llvm-objdump -d ch10_3.cpu0.o | tail -n +6| awk '{print "/* " $1 " 
  */\t" $2 " " $3 " " $4 " " $5 "\t/* " $6"\t" $7" " $8" " $9" " $10 "\t*/"}' 
  > ../cpu0_verilog/raw/cpu0s.hex
  
  118-165-75-175:raw Jonathan$ ./cpu0s
  ...
  12890ns 0000012c : 01320000 OUTPUT=15        
  ...

We show Verilog PC output by display the I/O memory mapped address but we 
didn't implement the output hardware interface or port. The output hardware 
interface/port is dependent on hardware output device, such as RS232, speaker, 
LED, .... You should implement the I/O interface/port when you want to program 
FPGA and wire I/O device to the I/O port.



.. [#] http://www.ece.umd.edu/courses/enee359a/

.. [#] http://www.ece.umd.edu/courses/enee359a/verilog_tutorial.pdf
