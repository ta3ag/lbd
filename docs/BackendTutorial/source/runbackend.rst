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

This section list all the AsmParser code for cpu0 backend but with only a few 
explanation. Please refer here [#]_ for more AsmParser explanation.

Run Chapter10_1/ with ch11_1.cpp will get the following error message.

.. rubric:: lbdex/InputFiles/ch11_1.cpp
.. literalinclude:: ../lbdex/InputFiles/ch11_1.cpp
    :start-after: /// start

.. code-block:: bash

  JonathantekiiMac:InputFiles Jonathan$ clang -c ch11_1.cpp -emit-llvm -o 
  ch11_1.bc
  JonathantekiiMac:InputFiles Jonathan$ /Users/Jonathan/llvm/test/cmake_debug_
  build/bin/Debug/llc -march=cpu0 -relocation-model=pic -filetype=obj ch11_1.bc 
  -o ch11_1.cpu0.o
  LLVM ERROR: Inline asm not supported by this streamer because we don't have 
  an asm parser for this target
  
Since we didn't implement cpu0 assembler, it has the error message as above. 
The cpu0 can translate LLVM IR into assembly and obj directly, but it cannot 
translate hand code assembly instructions into obj. 
Directory AsmParser handle the assembly to obj translation.
The Chapter11_1/ include AsmParser implementation as follows,

.. rubric:: lbdex/Chapter11_1/AsmParser/Cpu0AsmParser.cpp
.. literalinclude:: ../../../lib/Target/Cpu0/AsmParser/Cpu0AsmParser.cpp

.. rubric:: lbdex/Chapter11_1/AsmParser/CMakeLists.txt
.. literalinclude:: ../../../lib/Target/Cpu0/AsmParser/CMakeLists.txt

.. rubric:: lbdex/Chapter11_1/AsmParser/LLVMBuild.txt
.. literalinclude:: ../../../lib/Target/Cpu0/AsmParser/LLVMBuild.txt


The Cpu0AsmParser.cpp contains one thousand lines of code which do the assembly 
language parsing. You can understand it with a little patient only.
To let file directory of AsmParser be built, modify CMakeLists.txt and 
LLVMBuild.txt as follows,

.. rubric:: lbdex/Chapter11_1/CMakeLists.txt
.. code-block:: c++

  tablegen(LLVM Cpu0GenAsmMatcher.inc -gen-asm-matcher)
  ...
  add_subdirectory(AsmParser)
  
.. rubric:: lbdex/Chapter11_1/LLVMBuild.txt
.. code-block:: c++

  subdirectories = 
    AsmParser 
  ...
  has_asmparser = 1
  
  
.. rubric:: lbdex/Chapter11_1/Cpu0.td
.. code-block:: c++

  def Cpu0AsmParser : AsmParser {
    let ShouldEmitMatchRegisterName = 0;
  }
  
  def Cpu0AsmParserVariant : AsmParserVariant {
    int Variant = 0;
  
    // Recognize hard coded registers.
    string RegisterPrefix = "$";
  } // def Cpu0AsmParserVariant
  ...
  def Cpu0 : Target {
    ...
    let AssemblyParsers = [Cpu0AsmParser];
    ...
    let AssemblyParserVariants = [Cpu0AsmParserVariant];
  }
  
.. rubric:: lbdex/Chapter11_1/Cpu0InstrFormats.td
.. code-block:: c++

  // Pseudo-instructions for alternate assembly syntax (never used by codegen).
  // These are aliases that require C++ handling to convert to the target
  // instruction, while InstAliases can be handled directly by tblgen.
  class Cpu0AsmPseudoInst<dag outs, dag ins, string asmstr>:
    Cpu0Inst<outs, ins, asmstr, [], IIPseudo, Pseudo> {
    let isPseudo = 1;
    let Pattern = [];
  } // lbd document - mark - class Cpu0AsmPseudoInst
  
.. rubric:: lbdex/Chapter11_1/Cpu0InstrInfo.td
.. code-block:: c++

  // Cpu0InstrInfo.td
  def Cpu0MemAsmOperand : AsmOperandClass {
    let Name = "Mem";
    let ParserMethod = "parseMemOperand";
  } // lbd document - mark - def Cpu0MemAsmOperand
  
  // Address operand
  def mem : Operand<i32> {
    ...
    let ParserMatchClass = Cpu0MemAsmOperand;
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


Above declare the **ParserMethod = "parseMemOperand"** and implement the 
parseMemOperand() in Cpu0AsmParser.cpp to handle the **"mem"** operand which 
used in Cpu0 instructions ld and st. 
For example, ld $2, 4($sp), the **mem** operand is 4($sp). 
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


Above three Pseudo Instruction definitions in Cpu0InstrInfo.td such as 
LoadImm32Reg are handled by Cpu0AsmParser.cpp as follows,

.. rubric:: lbdex/Chapter11_1/AsmParser/Cpu0AsmParser.cpp
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

.. rubric:: lbdex/Chapter11_1/Cpu0RegisterInfo.td
.. literalinclude:: ../../../lib/Target/Cpu0/Cpu0RegisterInfo.td
    :start-after: //  Registers
    :end-before: // Hi/Lo Registers class


Run Chapter11_1/ with ch11_1.cpp to get the correct result as follows,

.. code-block:: bash

  JonathantekiiMac:InputFiles Jonathan$ /Users/Jonathan/llvm/test/cmake_debug_
  build/bin/Debug/llc -march=cpu0 -relocation-model=pic -filetype=obj ch11_1.bc -o 
  ch11_1.cpu0.o
  JonathantekiiMac:InputFiles Jonathan$ /Users/Jonathan/llvm/test/cmake_debug_
  build/bin/Debug/llvm-objdump -d ch11_1.cpu0.o
  
  ch11_1.cpu0.o:  file format ELF32-unknown
  
  Disassembly of section .text:
  .text:
         0:	01 2d 00 08                                  	ld	$2, 8($sp)
         4:	02 0d 00 04                                  	st	$zero, 4($sp)
         8:	09 30 00 00                                  	addiu	$3, $zero, 0
         c:	13 31 20 00                                  	add	$3, $1, $2
        10:	14 32 30 00                                  	sub	$3, $2, $3
        14:	17 21 30 00                                  	mul	$2, $1, $3
        18:	43 32 00 00                                  	div	$3, $2
        1c:	44 23 00 00                                  	divu	$2, $3
        20:	18 21 30 00                                  	and	$2, $1, $3
        24:	19 31 20 00                                  	or	$3, $1, $2
        28:	1a 12 30 00                                  	xor	$1, $2, $3
        2c:	41 43 00 00                                  	mult	$4, $3
        30:	42 32 00 00                                  	multu	$3, $2
        34:	46 30 00 00                                  	mfhi	$3
        38:	47 20 00 00                                  	mflo	$2
        3c:	48 20 00 00                                  	mthi	$2
        40:	49 20 00 00                                  	mtlo	$2
        44:	1d 22 00 02                                  	sra	$2, $2, 2
        48:	1b 21 00 03                                  	rol	$2, $1, 3
        4c:	1c 33 00 04                                  	ror	$3, $3, 4
        50:	1e 22 00 02                                  	shl	$2, $2, 2
        54:	1f 23 00 05                                  	shr	$2, $3, 5
        58:	10 23 00 00                                  	cmp	$sw, $2, $3
        5c:	30 00 00 14                                  	jeq	$sw, 20
        60:	31 00 00 10                                  	jne	$sw, 16
        64:	32 ff ff ec                                  	jlt	$sw, -20
        68:	34 ff ff f0                                  	jle	$sw, -16
        6c:	33 ff ff fc                                  	jgt	$sw, -4
        70:	35 ff ff f4                                  	jge	$sw, -12
        74:	3a 00 04 00                                  	swi	1024
        78:	3b 01 00 00                                  	jsub	65536
        7c:	3c e0 00 00                                  	ret	$lr
        80:	3e e6 00 00                                  	jalr	$t9
        84:	0f 30 00 70                                  	lui	$3, 112
        88:	0d 33 00 00                                  	ori	$3, $3, 0
        8c:	0f 30 00 80                                  	lui	$3, 128
        90:	0d 33 00 00                                  	ori	$3, $3, 0
        94:	13 33 60 00                                  	add	$3, $3, $t9
        98:	0f 30 00 90                                  	lui	$3, 144
        9c:	0d 33 00 00                                  	ori	$3, $3, 0


The instructions cmp and jeg printed with explicit $sw displayed in assembly 
and disassembly. You can change code in AsmParser and Dissassembly (the last 
chapter) to hide the $sw printed in these instructions (set $sw to implicit 
and not displayed, such as "jeq 20" rather than "jeq $sw, 20").


Inline assembly
------------------

Run Chapter11_1 with ch11_2 will get the following error.

.. rubric:: lbdex/InputFiles/ch11_2.cpp
.. literalinclude:: ../lbdex/InputFiles/ch11_2.cpp
    :start-after: /// start

.. code-block:: bash
  
  1-160-129-73:InputFiles Jonathan$ ~/llvm/test/cmake_debug_build/bin/Debug/llc 
  -march=cpu0 -relocation-model=static -filetype=asm ch11_2.bc -o -
    .section .mdebug.abi32
    .previous
    .file "ch11_2.bc"
  error: couldn't allocate output register for constraint 'r'

The ch11_2.cpp is the inline assembly example. The clang support inline 
assembly like gcc. The inline assembly is be used in C/C++ which need access 
the allocated register or memory for the C/C++ variable. For example, the 
variable foo of ch11_2.cpp can be allocated by compiler to register $2, $3 
or other. The inline assembly fill the gap between high level language and 
assembly language. Reference here [#]_. Chapter11_2 support inline assembly 
as follows,

.. rubric:: lbdex/Chapter11_2/Cpu0AsmPrinter.h
.. literalinclude:: ../../../lib/Target/Cpu0/Cpu0AsmPrinter.h
    :start-after: virtual void EmitFunctionBodyEnd();
    :end-before: void EmitStartOfAsmFile(Module &M);

.. rubric:: lbdex/Chapter11_2/Cpu0AsmPrinter.cpp
.. literalinclude:: ../../../lib/Target/Cpu0/Cpu0AsmPrinter.cpp
    :start-after: // Print out an operand for an inline asm expression.
    :end-before: MachineLocation

.. rubric:: lbdex/Chapter11_2/Cpu0InstrInfo.h
.. literalinclude:: ../../../lib/Target/Cpu0/Cpu0InstrInfo.h
    :start-after: bool KillSrc) const;
    :end-before: virtual void storeRegToStackSlot(MachineBasicBlock &MBB,

.. rubric:: lbdex/Chapter11_2/Cpu0InstrInfo.cpp
.. literalinclude:: ../../../lib/Target/Cpu0/Cpu0InstrInfo.cpp
    :start-after: /// Return the number of bytes of code the specified instruction may be.

.. rubric:: lbdex/Chapter11_2/Cpu0ISelDAGToDAG.cpp
.. literalinclude:: ../../../lib/Target/Cpu0/Cpu0ISelDAGToDAG.cpp
    :start-after: void InitGlobalBaseReg(MachineFunction &MF);
    :end-before: };
.. literalinclude:: ../../../lib/Target/Cpu0/Cpu0ISelDAGToDAG.cpp
    :start-after: // lbd document - mark - inlineasm begin
    :end-before: /// createCpu0ISelDag - This pass converts a legalized DAG into a

.. rubric:: lbdex/Chapter11_2/Cpu0ISelLowering.h
.. literalinclude:: ../../../lib/Target/Cpu0/Cpu0ISelLowering.h
    :start-after: SDLoc DL, SelectionDAG &DAG) const;
    :end-before: virtual bool isOffsetFoldingLegal(const GlobalAddressSDNode *GA) const;

.. rubric:: lbdex/Chapter11_2/Cpu0ISelLowering.cpp
.. literalinclude:: ../../../lib/Target/Cpu0/Cpu0ISelLowering.cpp
    :start-after: // lbd document - mark - inlineasm begin
    :end-before: bool // lbd document - mark - isOffsetFoldingLegal


Verilog of CPU0
------------------

Verilog language is an IEEE standard in IC design. There are a lot of book and 
documents for this language. Web site [#]_ has a pdf [#]_ in this. 
Example code lbdex/cpu0_verilog/cpu0.v is the 
cpu0 design in Verilog. In Appendix A, we have downloaded and installed Icarus 
Verilog tool both on iMac and Linux. The cpu0.v and cpu0Is.v is a simple design 
with only few hundreds lines of code. 
Alough it has not the pipeline features, we can assume the 
cpu0 backend code run on the pipeline machine because the pipeline version  
use the same machine instructions. Verilog is C like language in syntex and 
this book is a compiler book, so we list the cpu0.v as well as the building 
command directly as below. We expect 
readers can understand the Verilog code just with a little patient and no need 
further explanation. According computer architecture, there are two type of I/O. 
One is memory mapped I/O, the other is instruction I/O. 
CPU0 use memory mapped I/O, we set the memory address 0x7000 as the output port. 
When meet the instruction **"st $ra, cx($rb)"**, where cx($rb) is 
0x7000 (28672), CPU0 display the content as follows,

.. code-block:: c++

        ST :
          if (R[b]+c16 == 28672)
            $display("%4dns %8x : %8x OUTPUT=%-d", $stime, pc0, ir, R[a]);


.. rubric:: lbdex/cpu0_verilog/cpu0.v
.. literalinclude:: ../lbdex/cpu0_verilog/cpu0.v

.. rubric:: lbdex/cpu0_verilog/cpu0Is.v
.. literalinclude:: ../lbdex/cpu0_verilog/cpu0Is.v


.. code-block:: bash

  JonathantekiiMac:raw Jonathan$ pwd
  /Users/Jonathan/test/2/lbd/lbdex/cpu0_verilog/raw
  JonathantekiiMac:raw Jonathan$ iverilog -o cpu0Is cpu0Is.v 


Run program on CPU0 machine
---------------------------

Now let's compile ch_run_backend.cpp as below. Since code size grows up from low to high 
address and stack grows up from high to low address. We set $sp at 0x6ffc because 
cpu0.v use 0x7000 bytes of memory.

.. rubric:: lbdex/InputFiles/InitRegs.cpp
.. literalinclude:: ../lbdex/InputFiles/InitRegs.cpp
    :start-after: /// start

.. rubric:: lbdex/InputFiles/print.h
.. literalinclude:: ../lbdex/InputFiles/print.h
    :start-after: /// start

.. rubric:: lbdex/InputFiles/print.cpp
.. literalinclude:: ../lbdex/InputFiles/print.cpp
    :start-after: /// start

.. rubric:: lbdex/InputFiles/ch_nolld.h
.. literalinclude:: ../lbdex/InputFiles/ch_nolld.h
    :start-after: /// start

.. rubric:: lbdex/InputFiles/ch_nolld.cpp
.. literalinclude:: ../lbdex/InputFiles/ch_nolld.cpp
    :start-after: /// start

.. rubric:: lbdex/InputFiles/ch_run_backend.cpp
.. literalinclude:: ../lbdex/InputFiles/ch_run_backend.cpp
    :start-after: /// start

.. rubric:: lbdex/InputFiles/build-run_backend.sh
.. literalinclude:: ../lbdex/InputFiles/build-run_backend.sh

Let's run Chapter11_2/ with ``llvm-objdump -d`` for input files 
ch_run_backend.cpp to generate the hex file of inputing to cpu0Is Verilog 
simulator to get the output result as below. 
Remind ch_run_backend.cpp have to be compiled with option 
``clang -target mips-unknown-linux-gnu`` and use the clang of
your build instead of download from Xcode on iMac. The ~/llvm/release/
cmake_debug_build/bin/Debug/ is my build clang from source code.


.. code-block:: bash

  JonathantekiiMac:InputFiles Jonathan$ bash build-run_backend.sh
  JonathantekiiMac:InputFiles Jonathan$ cd ../cpu0_verilog
  JonathantekiiMac:cpu0_verilog Jonathan$ ./cpu0Is
  WARNING: cpu0Is.v:386: $readmemh(cpu0.hex): Not enough words in the file for the 
  taskInterrupt(001)
  74
  253
  3
  1
  14
  3
  -126
  130
  -32766
  32770
  393307
  16777222
  51
  2147483647
  -21474836487
  7
  15
  RET to PC < 0, finished!


From the result as below, you can find the print_integer() which implemented by C 
language has more instructions while the print1_integer() which implemented by 
assembly has less instructions. But the C version is better in portability since 
the assembly version is binding with machine assembly language and make the 
assumption that the stack size of print1_integer() is 8.

.. code-block:: bash

  JonathantekiiMac:raw Jonathan$ cd ../../InputFiles/
  JonathantekiiMac:InputFiles Jonathan$ /Users/Jonathan/llvm/test/cmake_debug_build
  /bin/Debug/llvm-objdump -d ch_run_backend.cpu0.o 
  ...
  _Z13print_integeri:
       ................                                   addiu $sp, $sp, -24
       ................                                   st  $lr, 20($sp)
       ................                                   st  $fp, 16($sp)
       ................                                   add $fp, $sp, $zero
       ................                                   ld  $2, 24($fp)
       ................                                   st  $2, 12($fp)
       ................                                   st  $2, 0($sp)
       ................                                   jsub  716
       ................                                   st  $2, 8($fp)
       ................                                   st  $2, 0($sp)
       ................                                   jsub  1084
       ................                                   add $sp, $fp, $zero
       ................                                   ld  $fp, 16($sp)
       ................                                   ld  $lr, 20($sp)
       ................                                   addiu $sp, $sp, 24
       ................                                   ret $lr
  ...
  _Z14print1_integeri:
       ................                                   addiu $sp, $sp, -8
       ................                                   st  $fp, 4($sp)
       ................                                   add $fp, $sp, $zero
       ................                                   ld  $2, 8($fp)
       ................                                   st  $2, 0($fp)
       ................                                   ld  $1, 8($sp)
       ................                                   st  $1, 28672($zero)
       ................                                   add $sp, $fp, $zero
       ................                                   ld  $fp, 4($sp)
       ................                                   addiu $sp, $sp, 8
       ................                                   ret $lr


You can trace the memory binary code and destination
register changed at every instruction execution by the following change and
get the result as below,

.. rubric:: lbdex/cpu0_verilog/cpu0Is.v

.. code-block:: c++

  `define TRACE 

.. rubric:: lbdex/cpu0_verilog/cpu0.v

.. code-block:: c++

      ...
      `D = 1; // Trace register content at beginning

.. code-block:: bash

  JonathantekiiMac:raw Jonathan$ ./cpu0Is
  WARNING: cpu0.v:386: $readmemh(cpu0.hex): Not enough words in the file for the 
  requested range [0:28671].
  00000000: 2600000c
  00000004: 26000004
  00000008: 26000004
  0000000c: 26fffffc
  00000010: 09100000
  00000014: 09200000
  ...
  taskInterrupt(001)
  1530ns 00000054 : 02ed002c m[28620+44  ]=-1          SW=00000000
  1610ns 00000058 : 02bd0028 m[28620+40  ]=0           SW=00000000
  ...                     
  RET to PC < 0, finished!


As above result, cpu0.v dump the memory first after read input cpu0.hex. 
Next, it run instructions from address 0 and print each destination 
register value in the fourth column. 
The first column is the nano seconds of timing. The second 
is instruction address. The third is instruction content. 
We have checked many example code is correct by print the variable with 
print_integer().

This chapter shows Verilog PC output by display the I/O memory mapped address but
didn't implementing the output hardware interface or port. 
The real output hardware 
interface/port is hardware output device dependent, such as RS232, speaker, 
LED, .... You should implement the I/O interface/port when you want to program 
FPGA and wire I/O device to the I/O port.


.. [#] http://www.embecosm.com/appnotes/ean10/ean10-howto-llvmas-1.0.html

.. [#] http://www.ibiblio.org/gferg/ldp/GCC-Inline-Assembly-HOWTO.html

.. [#] http://www.ece.umd.edu/courses/enee359a/

.. [#] http://www.ece.umd.edu/courses/enee359a/verilog_tutorial.pdf
