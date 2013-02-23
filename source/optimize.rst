.. _sec-optimize:

Backend Optimization
====================

This chapter introduce how to do backend optimization in LLVM first. 
Next we do optimization via redesign instruction sets with hardware level to 
do optimization by create a efficient RISC CPU which aim to C/C++ high level 
language.

Cpu0 backend Optimization: Remove useless JMP
---------------------------------------------

LLVM use functional pass in code generation and optimization. 
Following the 3 tiers of compiler architecture, LLVM did much optimization in 
middle tier of which is LLVM IR, SSA form. 
In spite of this middle tier optimization, there are opportunities in 
optimization which depend on backend features. 
Mips fill delay slot is an example of backend optimization used in pipeline 
RISC machine.
You can modify from Mips this part if your backend is a pipeline RISC with 
delay slot.
We apply the "delete useless jmp" unconditional branch instruction in Cpu0 
backend optimization in this section. 
This algorithm is simple and effective as a perfect tutorial in optimization. 
You can understand how to add a optimization pass and design your complicate 
optimization algorithm on your backend in real project.

11/1/Cpu0 support this optimization algorithm include the added codes as follows,

.. code-block:: c++

  // CMakeLists.txt
  add_llvm_target(Cpu0CodeGen
    ...
    Cpu0DelUselessJMP.cpp
    ...
    )
  
  // Cpu0.h
  ...
    FunctionPass *createCpu0DelJmpPass(Cpu0TargetMachine &TM);
  
  // Cpu-TargetMachine.cpp
  class Cpu0PassConfig : public TargetPassConfig {
    ...
    virtual bool addPreEmitPass();
  };
  ...
  // Implemented by targets that want to run passes immediately before
  // machine code is emitted. return true if -print-machineinstrs should
  // print out the code after the passes.
  bool Cpu0PassConfig::addPreEmitPass() {
    Cpu0TargetMachine &TM = getCpu0TargetMachine();
    addPass(createCpu0DelJmpPass(TM));
    return true;
  }
  
  // Cpu0DelUselessJMP.cpp
  //===-- Cpu0DelUselessJMP.cpp - Cpu0 DelJmp -------------------------------===//
  //
  //                     The LLVM Compiler Infrastructure
  //
  // This file is distributed under the University of Illinois Open Source
  // License. See LICENSE.TXT for details.
  //
  //===----------------------------------------------------------------------===//
  //
  // Simple pass to fills delay slots with useful instructions.
  //
  //===----------------------------------------------------------------------===//
  
  #define DEBUG_TYPE "del-jmp"
  
  #include "Cpu0.h"
  #include "Cpu0TargetMachine.h"
  #include "llvm/CodeGen/MachineFunctionPass.h"
  #include "llvm/Support/CommandLine.h"
  #include "llvm/Target/TargetMachine.h"
  #include "llvm/Target/TargetInstrInfo.h"
  #include "llvm/ADT/SmallSet.h"
  #include "llvm/ADT/Statistic.h"
  
  using namespace llvm;
  
  STATISTIC(NumDelJmp, "Number of useless jmp deleted");
  
  static cl::opt<bool> EnableDelJmp(
    "enable-cpu0-del-useless-jmp",
    cl::init(true),
    cl::desc("Delete useless jmp instructions: jmp 0."),
    cl::Hidden);
  
  namespace {
    struct DelJmp : public MachineFunctionPass {
  
      TargetMachine &TM;
      const TargetInstrInfo *TII;
  
      static char ID;
      DelJmp(TargetMachine &tm)
        : MachineFunctionPass(ID), TM(tm), TII(tm.getInstrInfo()) { }
  
      virtual const char *getPassName() const {
        return "Cpu0 Del Useless jmp";
      }
  
      bool runOnMachineBasicBlock(MachineBasicBlock &MBB, MachineBasicBlock &MBBN);
      bool runOnMachineFunction(MachineFunction &F) {
        bool Changed = false;
        if (EnableDelJmp) {
          MachineFunction::iterator FJ = F.begin();
          if (FJ != F.end())
            FJ++;
          if (FJ == F.end())
            return Changed;
          for (MachineFunction::iterator FI = F.begin(), FE = F.end();
               FJ != FE; ++FI, ++FJ)
            // In STL style, F.end() is the dummy BasicBlock() like '\0' in 
            //  C string. 
            // FJ is the next BasicBlock of FI; When FI range from F.begin() to 
            //  the PreviousBasicBlock of F.end() call runOnMachineBasicBlock().
            Changed |= runOnMachineBasicBlock(*FI, *FJ);
        }
        return Changed;
      }
  
    };
    char DelJmp::ID = 0;
  } // end of anonymous namespace
  
  /// runOnMachineBasicBlock - Fill in delay slots for the given basic block.
  /// We assume there is only one delay slot per delayed instruction.
  bool DelJmp::
  runOnMachineBasicBlock(MachineBasicBlock &MBB, MachineBasicBlock &MBBN) {
    bool Changed = false;
  
    MachineBasicBlock::iterator I = MBB.end();
    I--;	// set I to the last instruction
    
    if (I->getOpcode() == Cpu0::JMP && I->getOperand(0).getMBB() == &MBBN) {
      // I is the instruction of "jmp #offset=0", as follows,
      //     jmp	$BB0_3
      // $BB0_3:
      //     ld	$4, 28($sp)
      ++NumDelJmp;
      MBB.erase(I);	// delete the "JMP 0" instruction
      Changed = true;	// Notify LLVM kernel Changed
    }
    return Changed;
  
  }
  
  /// createCpu0DelJmpPass - Returns a pass that DelJmp in Cpu0 MachineFunctions
  FunctionPass *llvm::createCpu0DelJmpPass(Cpu0TargetMachine &tm) {
    return new DelJmp(tm);
  }


As above code, except Cpu0DelUselessJMP.cpp, other files changed for register 
class DelJmp as a functional pass. As comment of above code, MBB is the current 
block and MBBN is the next block. For the last instruction of every MBB, we 
check if it is the JMP instruction as well as 
its Operand is the next basic block. 
By getMBB() in MachineOperand, you can get the MBB address. 
For the member function of MachineOperand, please check 
include/llvm/CodeGen/MachineOperand.h
Let's run 11/1/Cpu0 with ch11_2.cpp to explain it easier.

.. code-block:: c++

  // ch11_2.cpp
  int main()
  {
    int a = 0;
    int b = 1;
    int c = 2;
    
    if (a == 0) {
      a++;
    }
    if (b == 0) {
      a = a + b;
    } else if (b < 0) {
      a = a--;
    }
    if (c > 0) {
      c++;
    }
    
    return a;
  }

.. code-block:: bash

  118-165-78-10:InputFiles Jonathan$ clang -c ch11_1.cpp -emit-llvm -o ch11_1.bc
  118-165-78-10:InputFiles Jonathan$ clang -target `llvm-config --host-target` 
  -c ch11_1.cpp -emit-llvm -o ch11_1.bc
  118-165-78-10:InputFiles Jonathan$ /Users/Jonathan/llvm/test/cmake_debug_build/
  bin/Debug/llc -march=cpu0 -relocation-model=static -filetype=asm -stats 
  ch11_1.bc -o ch11_1.cpu0.s
  ===-------------------------------------------------------------------------===
                            ... Statistics Collected ...
  ===-------------------------------------------------------------------------===
   ...
   2 del-jmp        - Number of useless jmp deleted
   ...
  
  118-165-78-10:InputFiles Jonathan$ cat ch11_1.cpu0.s 
  	.section .mdebug.abi32
  	.previous
  	.file	"ch11_1.bc"
  	.text
  	.globl	main
  	.align	2
  	.type	main,@function
  	.ent	main                    # @main
  main:
  	.frame	$sp,16,$lr
  	.mask 	0x00000000,0
  	.set	noreorder
  	.set	nomacro
  # BB#0:
  	addiu	$sp, $sp, -16
  	addiu	$3, $zero, 0
  	st	$3, 12($sp)
  	st	$3, 8($sp)
  	addiu	$2, $zero, 1
  	st	$2, 4($sp)
  	addiu	$4, $zero, 2
  	st	$4, 0($sp)
  	ld	$4, 8($sp)
  	cmp	$sw, $4, $3
  	jne	$sw, $BB0_2
  # BB#1:
  	ld	$4, 8($sp)
  	addiu	$4, $4, 1
  	st	$4, 8($sp)
  $BB0_2:
  	ld	$4, 4($sp)
  	cmp	$sw, $4, $3
  	jne	$sw, $BB0_4
  	jmp	$BB0_3
  $BB0_4:
  	addiu	$3, $zero, -1
  	ld	$4, 4($sp)
  	cmp	$sw, $4, $3
  	jgt	$sw, $BB0_6
  	jmp	$BB0_5
  $BB0_3:
  	ld	$3, 4($sp)
  	ld	$4, 8($sp)
  	add	$3, $4, $3
  	st	$3, 8($sp)
  	jmp	$BB0_6
  $BB0_5:
  	ld	$3, 8($sp)
  	addiu	$4, $3, -1
  	st	$4, 8($sp)
  	st	$3, 8($sp)
  $BB0_6:
  	ld	$3, 0($sp)
  	cmp	$sw, $3, $2
  	jlt	$sw, $BB0_8
  # BB#7:
  	ld	$2, 0($sp)
  	addiu	$2, $2, 1
  	st	$2, 0($sp)
  $BB0_8:
  	ld	$2, 8($sp)
  	addiu	$sp, $sp, 16
  	ret	$lr
  	.set	macro
  	.set	reorder
  	.end	main
  $tmp1:
  	.size	main, ($tmp1)-main

The terminal display "Number of useless jmp deleted" by ``llc -stats`` option 
because we set the "STATISTIC(NumDelJmp, "Number of useless jmp deleted")" in 
code. It delete 2 jmp instructions from block "# BB#0" and "$BB0_6".
You can check it by ``llc -enable-cpu0-del-useless-jmp=false`` option to see 
the difference from no optimization version.
If you run with ch7_1_1.cpp, will find 10 jmp instructions are deleted in 100 
lines of assembly code, which meaning 10% enhance in speed and code size.


Cpu0 Optimization: Redesign instruction sets
---------------------------------------------

If you compare the cpu0 and Mips instruction sets, you will find the following,

1. Mips has **addu** and **add** two different instructions for No Trigger 
Exception and Trigger Exception.

2. Mips use SLT, BEQ and set the status in explicit/general register while Cpu0 
use CMP, JEQ and set status in implicit/specific register.

According RISC spirits, this section will replace CMP, JEQ with Mips style 
instructions and support both Trigger and No Trigger Exception operators.
Mips style BEQ instructions will reduce the number of branch instructions too. 
Which means optimization in speed and code size.

Cpu0 new instruction sets table
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Redesign Cpu0 instruction set and remap OP code as follows (OP code 
0x00 is reserved for NOP operation in pipeline architecture),

.. list-table:: Cpu0 Instruction Set
	:widths: 3 4 3 11 7 10
	:header-rows: 1

	* - Format
	  - Mnemonic
	  - Opcode
	  - Meaning
	  - Syntax
	  - Operation
	* - L
	  - LD
	  - 01
	  - Load word
	  - LD Ra, [Rb+Cx]
	  - Ra <= [Rb+Cx]
	* - L
	  - ST
	  - 02
	  - Store word
	  - ST Ra, [Rb+Cx]
	  - [Rb+Cx] <= Ra
	* - L
	  - LDB
	  - 03
	  - Load byte
	  - LDB Ra, [Rb+Cx]
	  - Ra <= (byte)[Rb+Cx]
	* - L
	  - STB
	  - 04
	  - Store byte
	  - STB Ra, [Rb+Cx]
	  - [Rb+Cx] <= (byte)Ra
	* - A
	  - LDR
	  - 05
	  - Load word (w/ register index)
	  - LDR Ra, [Rb+Rc]
	  - Ra <= [Rb+Rc]
	* - A
	  - STR
	  - 06
	  - Store word (w/ register index)
	  - STR Ra, [Rb+Rc]
	  - [Rb+Rc] <= Ra
	* - A
	  - LBR
	  - 07
	  - Load byte (w/ register index)
	  - LBR Ra, [Rb+Rc]
	  - Ra <= (byte)[Rb+Rc]
	* - A
	  - SBR
	  - 08
	  - Store byte (w/ register index)
	  - SBR Ra, [Rb+Cx]
	  - [Rb+Rc] <= (byte)Ra
	* - L
	  - ADDiu
	  - 09
	  - Add immediate
	  - ADDiu Ra, Rb, Cx
	  - Ra <= (Rb + Cx)
	* - L
	  - SLTi
	  - 0A
	  - Set less Then
	  - SLTi Ra, Rb, Cx
	  - Ra <= (Rb < Cx)
	* - L
	  - SLTiu
	  - 0B
	  - SLTi unsigned 
	  - SLTiu Ra, Rb, Cx
	  - Ra <= (Rb < Cx)
	* - L
	  - ANDi
	  - 0C
	  - AND imm
	  - ANDi Ra, Rb, Cx
	  - Ra <= (Rb & Cx)
	* - L
	  - ORi
	  - 0D
	  - OR
	  - ORi Ra, Rb, Cx
	  - Ra <= (Rb | Cx)
	* - L
	  - XORi
	  - 0E
	  - XOR
	  - XORi Ra, Rb, Cx
	  - Ra <= (Rb ^ Cx)
	* - L
	  - LUi
	  - 0F
	  - Load upper
	  - LUi Ra, Cx
	  - Ra <= (Cx||0x0000)
	* - A
	  - ADDu
	  - 11
	  - Add unsigned
	  - ADD Ra, Rb, Rc
	  - Ra <= Rb + Rc
	* - A
	  - SUBu
	  - 12
	  - Sub unsigned
	  - SUB Ra, Rb, Rc
	  - Ra <= Rb - Rc
	* - A
	  - ADD
	  - 13
	  - Add
	  - ADD Ra, Rb, Rc
	  - Ra <= Rb + Rc
	* - A
	  - SUB
	  - 14
	  - Subtract
	  - SUB Ra, Rb, Rc
	  - Ra <= Rb - Rc
	* - A
	  - MUL
	  - 15
	  - Multiply
	  - MUL Ra, Rb, Rc
	  - Ra <= Rb * Rc
	* - A
	  - DIV
	  - 16
	  - Divide
	  - DIV Ra, Rb
	  - HI<=Ra%Rb, LO<=Ra/Rb
	* - A
	  - DIVu
	  - 16
	  - Div unsigned
	  - DIVu Ra, Rb
	  - HI<=Ra%Rb, LO<=Ra/Rb
	* - A
	  - AND
	  - 18
	  - Bitwise and
	  - AND Ra, Rb, Rc
	  - Ra <= Rb & Rc
	* - A
	  - OR
	  - 19
	  - Bitwise or
	  - OR Ra, Rb, Rc
	  - Ra <= Rb | Rc
	* - A
	  - XOR
	  - 1A
	  - Bitwise exclusive or
	  - XOR Ra, Rb, Rc
	  - Ra <= Rb ^ Rc
  	* - A
	  - ROL
	  - 1C
	  - Rotate left
	  - ROL Ra, Rb, Cx
	  - Ra <= Rb rol Cx
  	* - A
	  - ROR
	  - 1D
	  - Rotate right
	  - ROR Ra, Rb, Cx
	  - Ra <= Rb ror Cx
   	* - A
	  - SHL
	  - 1E
	  - Shift left
	  - SHL Ra, Rb, Cx
	  - Ra <= Rb << Cx
   	* - A
	  - SHR
	  - 1F
	  - Shift right
	  - SHR Ra, Rb, Cx
	  - Ra <= Rb >> Cx
	* - A
	  - SLT
	  - 20
	  - Set less Then
	  - SLT Ra, Rb, Rc
	  - Ra <= (Rb < Rc)
	* - A
	  - SLT
	  - 21
	  - SLT unsigned
	  - SLTu Ra, Rb, Rc
	  - Ra <= (Rb < Rc)
   	* - L
	  - MFHI
	  - 22
	  - Move HI to GPR
	  - MFHI Ra
	  - Ra <= HI
   	* - L
	  - MFLO
	  - 23
	  - Move LO to GPR
	  - MFLO Ra
	  - Ra <= LO
   	* - L
	  - MTHI
	  - 24
	  - Move GPR to HI
	  - MTHI Ra
	  - HI <= Ra
   	* - L
	  - MTLO
	  - 25
	  - Move GPR to LO
	  - MTLO Ra
	  - LO <= Ra
   	* - L
	  - MULT
	  - 26
	  - Multiply for 64 bits result
	  - MULT Ra, Rb
	  - (HI,LO) <= MULT(Ra,Rb)
   	* - L
	  - MULTU
	  - 27
	  - MULT for unsigned 64 bits
	  - MULTU Ra, Rb
	  - (HI,LO) <= MULTU(Ra,Rb)
   	* - J
	  - JMP
	  - 26
	  - Jump (unconditional)
	  - JMP Cx
	  - PC <= PC + Cx
   	* - L
	  - BEQ
	  - 27
	  - Jump if equal
	  - BEQ Ra, Rb, Cx
	  - if (Ra==Rb), PC <= PC + Cx
   	* - L
	  - BNE
	  - 28
	  - Jump if not equal
	  - BNE Ra, Rb, Cx
	  - if (Ra!=Rb), PC <= PC + Cx
   	* - J
	  - SWI
	  - 2A
	  - Software interrupt
	  - SWI Cx
	  - LR <= PC; PC <= Cx
   	* - J
	  - JSUB
	  - 2B
	  - Jump to subroutine
	  - JSUB Cx
	  - LR <= PC; PC <= PC + Cx
   	* - J
	  - RET
	  - 2C
	  - Return from subroutine
	  - RET Cx
	  - PC <= LR
   	* - J
	  - IRET
	  - 2D
	  - Return from interrupt handler
	  - IRET
	  - PC <= LR; INT 0
   	* - J
	  - JR
	  - 2E
	  - Jump to subroutine
	  - JR Rb
	  - LR <= PC; PC <= Rb
	  

As above, the OPu, such as ADDu is for unsigned integer or No Trigger 
Exception. The LUi for example, "LUi $2, 0x7000", load 0x700 to high 16 bits 
of $2 and fill the low 16 bits of $2 to 0x0000. 


Cpu0 code changes
~~~~~~~~~~~~~~~~~

11/2/Cpu0 include the changes for new instruction sets as follows,

.. code-block:: c++

  // Cpu0AsmParser.cpp
  void Cpu0AsmParser::expandLoadImm(MCInst &Inst, SMLoc IDLoc,
                                    SmallVectorImpl<MCInst> &Instructions){
    MCInst tmpInst;
    const MCOperand &ImmOp = Inst.getOperand(1);
    assert(ImmOp.isImm() && "expected immediate operand kind");
    const MCOperand &RegOp = Inst.getOperand(0);
    assert(RegOp.isReg() && "expected register operand kind");
  
    int ImmValue = ImmOp.getImm();
    tmpInst.setLoc(IDLoc);
    if ( 0 <= ImmValue && ImmValue <= 65535) {
      // for 0 <= j <= 65535.
      // li d,j => ori d,$zero,j
      tmpInst.setOpcode(Cpu0::ORi);
      tmpInst.addOperand(MCOperand::CreateReg(RegOp.getReg()));
      tmpInst.addOperand(
                MCOperand::CreateReg(Cpu0::ZERO));
      tmpInst.addOperand(MCOperand::CreateImm(ImmValue));
      Instructions.push_back(tmpInst);
    } else if ( ImmValue < 0 && ImmValue >= -32768) {
      // for -32768 <= j < 0.
      // li d,j => addiu d,$zero,j
      tmpInst.setOpcode(Cpu0::ADDiu); //TODO:no ADDiu64 in td files?
      tmpInst.addOperand(MCOperand::CreateReg(RegOp.getReg()));
      tmpInst.addOperand(
                MCOperand::CreateReg(Cpu0::ZERO));
      tmpInst.addOperand(MCOperand::CreateImm(ImmValue));
      Instructions.push_back(tmpInst);
    } else {
      // for any other value of j that is representable as a 32-bit integer.
      // li d,j => lui d,hi16(j)
      //           ori d,d,lo16(j)
      tmpInst.setOpcode(Cpu0::LUi);
      tmpInst.addOperand(MCOperand::CreateReg(RegOp.getReg()));
      tmpInst.addOperand(MCOperand::CreateImm((ImmValue & 0xffff0000) >> 16));
      Instructions.push_back(tmpInst);
      tmpInst.clear();
      tmpInst.setOpcode(Cpu0::ORi);
      tmpInst.addOperand(MCOperand::CreateReg(RegOp.getReg()));
      tmpInst.addOperand(MCOperand::CreateReg(RegOp.getReg()));
      tmpInst.addOperand(MCOperand::CreateImm(ImmValue & 0xffff));
      tmpInst.setLoc(IDLoc);
      Instructions.push_back(tmpInst);
    }
  }
  
  void Cpu0AsmParser::expandLoadAddressReg(MCInst &Inst, SMLoc IDLoc,
                                           SmallVectorImpl<MCInst> &Instructions){
    MCInst tmpInst;
    const MCOperand &ImmOp = Inst.getOperand(2);
    assert(ImmOp.isImm() && "expected immediate operand kind");
    const MCOperand &SrcRegOp = Inst.getOperand(1);
    assert(SrcRegOp.isReg() && "expected register operand kind");
    const MCOperand &DstRegOp = Inst.getOperand(0);
    assert(DstRegOp.isReg() && "expected register operand kind");
    int ImmValue = ImmOp.getImm();
    if ( -32768 <= ImmValue && ImmValue <= 32767) {
      // for -32768 <= j < 32767.
      //la d,j(s) => addiu d,s,j
      tmpInst.setOpcode(Cpu0::ADDiu); //TODO:no ADDiu64 in td files?
      tmpInst.addOperand(MCOperand::CreateReg(DstRegOp.getReg()));
      tmpInst.addOperand(MCOperand::CreateReg(SrcRegOp.getReg()));
      tmpInst.addOperand(MCOperand::CreateImm(ImmValue));
      Instructions.push_back(tmpInst);
    } else {
      // for any other value of j that is representable as a 32-bit integer.
      // la d,j(s) => lui d,hi16(j)
      //              ori d,d,lo16(j)
      //              add d,d,s
      tmpInst.setOpcode(Cpu0::LUi);
      tmpInst.addOperand(MCOperand::CreateReg(DstRegOp.getReg()));
      tmpInst.addOperand(MCOperand::CreateImm((ImmValue & 0xffff0000) >> 16));
      Instructions.push_back(tmpInst);
      tmpInst.clear();
      tmpInst.setOpcode(Cpu0::ORi);
      tmpInst.addOperand(MCOperand::CreateReg(DstRegOp.getReg()));
      tmpInst.addOperand(MCOperand::CreateReg(DstRegOp.getReg()));
      tmpInst.addOperand(MCOperand::CreateImm(ImmValue & 0xffff));
      Instructions.push_back(tmpInst);
      tmpInst.clear();
      tmpInst.setOpcode(Cpu0::ADD);
      tmpInst.addOperand(MCOperand::CreateReg(DstRegOp.getReg()));
      tmpInst.addOperand(MCOperand::CreateReg(DstRegOp.getReg()));
      tmpInst.addOperand(MCOperand::CreateReg(SrcRegOp.getReg()));
      Instructions.push_back(tmpInst);
    }
  }
  
  void Cpu0AsmParser::expandLoadAddressImm(MCInst &Inst, SMLoc IDLoc,
                                           SmallVectorImpl<MCInst> &Instructions){
    MCInst tmpInst;
    const MCOperand &ImmOp = Inst.getOperand(1);
    assert(ImmOp.isImm() && "expected immediate operand kind");
    const MCOperand &RegOp = Inst.getOperand(0);
    assert(RegOp.isReg() && "expected register operand kind");
    int ImmValue = ImmOp.getImm();
    if ( -32768 <= ImmValue && ImmValue <= 32767) {
      // for -32768 <= j < 32767.
      //la d,j => addiu d,$zero,j
      tmpInst.setOpcode(Cpu0::ADDiu);
      tmpInst.addOperand(MCOperand::CreateReg(RegOp.getReg()));
      tmpInst.addOperand(
                MCOperand::CreateReg(Cpu0::ZERO));
      tmpInst.addOperand(MCOperand::CreateImm(ImmValue));
      Instructions.push_back(tmpInst);
    } else {
      // for any other value of j that is representable as a 32-bit integer.
      // la d,j => lui d,hi16(j)
      //           ori d,d,lo16(j)
      tmpInst.setOpcode(Cpu0::LUi);
      tmpInst.addOperand(MCOperand::CreateReg(RegOp.getReg()));
      tmpInst.addOperand(MCOperand::CreateImm((ImmValue & 0xffff0000) >> 16));
      Instructions.push_back(tmpInst);
      tmpInst.clear();
      tmpInst.setOpcode(Cpu0::ORi);
      tmpInst.addOperand(MCOperand::CreateReg(RegOp.getReg()));
      tmpInst.addOperand(MCOperand::CreateReg(RegOp.getReg()));
      tmpInst.addOperand(MCOperand::CreateImm(ImmValue & 0xffff));
      Instructions.push_back(tmpInst);
    }
  }
  
  int Cpu0AsmParser::matchRegisterName(StringRef Name) {
    ...
        .Case("t0",  Cpu0::T0)
    ...
  }
  
  // Cpu0Disassembler.cpp
  // Decoder tables for Cpu0 register
  static const unsigned CPURegsTable[] = {
  // Change SW to T0 which is a caller saved
    Cpu0::T0, ...
  };
  
  // DecodeCMPInstruction() function is removed since No CMP instruction.
  /*static DecodeStatus DecodeCMPInstruction(MCInst &Inst,
                                         unsigned Insn,
                                         uint64_t Address,
                                         const void *Decoder) {
    int Reg_a = (int)fieldFromInstruction(Insn, 20, 4);
    int Reg_b = (int)fieldFromInstruction(Insn, 16, 4);
    int Reg_c = (int)fieldFromInstruction(Insn, 12, 4);
  
    Inst.addOperand(MCOperand::CreateReg(CPURegsTable[Reg_c]));
    Inst.addOperand(MCOperand::CreateReg(CPURegsTable[Reg_a]));
    Inst.addOperand(MCOperand::CreateReg(CPURegsTable[Reg_b]));
    return MCDisassembler::Success;
  }*/
  
  // Change DecodeBranchTarget() to following for 16 bit offset
  static DecodeStatus DecodeBranchTarget(MCInst &Inst,
                                         unsigned Insn,
                                         uint64_t Address,
                                         const void *Decoder) {
    int BranchOffset = fieldFromInstruction(Insn, 0, 16);
    if (BranchOffset > 0x8fff)
    	BranchOffset = -1*(0x10000 - BranchOffset);
    Inst.addOperand(MCOperand::CreateImm(BranchOffset));
    return MCDisassembler::Success;
  }
  
  // Cpu0AsmBackend.cpp
  static unsigned adjustFixupValue(unsigned Kind, uint64_t Value) {
  ...
    // Add/subtract and shift
    switch (Kind) {
    ...
    case Cpu0::fixup_Cpu0_PC16:
    case Cpu0::fixup_Cpu0_PC24:
      // So far we are only using this type for branches.
      // For branches we start 1 instruction after the branch
      // so the displacement will be one instruction size less.
      Value -= 4;
      break;
    ...
  }
  ...
    const MCFixupKindInfo &getFixupKindInfo(MCFixupKind Kind) const {
      const static MCFixupKindInfo Infos[Cpu0::NumTargetFixupKinds] = {
        // This table *must* be in same the order of fixup_* kinds in
        // Cpu0FixupKinds.h.
        //
        // name                    offset  bits  flags
        ...
        { "fixup_Cpu0_PC16",         0,     16,  MCFixupKindInfo::FKF_IsPCRel },
  ...
  
  // Cpu0BaseInfo.h
  inline static unsigned getCpu0RegisterNumbering(unsigned RegEnum)
  {
    switch (RegEnum) {
    ...
    case Cpu0::T0:
    ...
    }
  }
  
  // Cpu0FixupKinds.h
    enum Fixups {
      ...
      // PC relative branch fixup resulting in - R_CPU0_PC16.
      // cpu0 PC16, e.g. beq
      fixup_Cpu0_PC16,
      ...
    };
  
  // Cpu0 MC CodeEmitter.cpp
  unsigned Cpu0MCCodeEmitter::
  getBranchTargetOpValue(const MCInst &MI, unsigned OpNo,
                         SmallVectorImpl<MCFixup> &Fixups) const {
    ...
    Fixups.push_back(MCFixup::Create(0, Expr,
                                     MCFixupKind(Cpu0::fixup_Cpu0_PC16)));
    return 0;
  }
  
  // Cpu0InstrInfo.cpp
  // Immediate can be loaded with LUi (32-bit int with lower 16-bit cleared).
  def immLow16Zero : PatLeaf<(imm), [{
    int64_t Val = N->getSExtValue();
    return isInt<32>(Val) && !(Val & 0xffff);
  }]>;
  
  class ArithOverflowR<bits<8> op, string instr_asm,
                      InstrItinClass itin, RegisterClass RC, bit isComm = 0>:
    FA<op, (outs RC:$ra), (ins RC:$rb, RC:$rc),
       !strconcat(instr_asm, "\t$ra, $rb, $rc"), [], itin> {
    let shamt = 0;
    let isCommutable = isComm;
  }
  // Conditional Branch
  class CBranch<bits<8> op, string instr_asm, PatFrag cond_op, RegisterClass RC>:
    FL<op, (outs), (ins RC:$ra, RC:$rb, brtarget:$imm16),
               !strconcat(instr_asm, "\t$ra, $rb, $imm16"),
               [(brcond (i32 (cond_op RC:$ra, RC:$rb)), bb:$imm16)], IIBranch> {
    let isBranch = 1;
    let isTerminator = 1;
    let hasDelaySlot = 1;
    let Defs = [AT];
  }
  
  // SetCC
  class SetCC_R<bits<8> op, string instr_asm, PatFrag cond_op,
                RegisterClass RC>:
    FA<op, (outs CPURegs:$ra), (ins RC:$rb, RC:$rc),
       !strconcat(instr_asm, "\t$ra, $rb, $rc"),
       [(set CPURegs:$ra, (cond_op RC:$rb, RC:$rc))],
       IIAlu> {
    let shamt = 0;
  }
  
  class SetCC_I<bits<8> op, string instr_asm, PatFrag cond_op, Operand Od,
                PatLeaf imm_type, RegisterClass RC>:
    FL<op, (outs CPURegs:$ra), (ins RC:$rb, Od:$imm16),
       !strconcat(instr_asm, "\t$ra, $rb, $imm16"),
       [(set CPURegs:$ra, (cond_op RC:$rb, imm_type:$imm16))],
       IIAlu>;
  /// Load and Store Instructions
  ///  aligned
  defm LD     : LoadM32<0x01,  "ld",  load_a>;
  defm ST     : StoreM32<0x02, "st",  store_a>;
  
  /// Arithmetic Instructions (ALU Immediate)
  // add defined in include/llvm/Target/TargetSelectionDAG.td, line 315 (def add).
  def ADDiu   : ArithLogicI<0x09, "addiu", add, simm16, immSExt16, CPURegs>;
  def SLTi    : SetCC_I<0x0a, "slti", setlt, simm16, immSExt16, CPURegs>;
  def SLTiu   : SetCC_I<0x0b, "sltiu", setult, simm16, immSExt16, CPURegs>;
  def ANDi    : ArithLogicI<0x0c, "andi", and, uimm16, immZExt16, CPURegs>;
  def ORi     : ArithLogicI<0x0d, "ori", or, uimm16, immZExt16, CPURegs>;
  def XORi    : ArithLogicI<0x0e, "xori", xor, uimm16, immZExt16, CPURegs>;
  def LUi     : LoadUpper<0x0f, "lui", CPURegs, uimm16>;
  
  /// Arithmetic Instructions (3-Operand, R-Type)
  def ADDu    : ArithLogicR<0x11, "addu", add, IIAlu, CPURegs, 1>;
  def SUBu    : ArithLogicR<0x12, "subu", sub, IIAlu, CPURegs>;
  def ADD     : ArithOverflowR<0x13, "add", IIAlu, CPURegs, 1>;
  def SUB     : ArithOverflowR<0x14, "sub", IIAlu, CPURegs>;
  def MUL     : ArithLogicR<0x15, "mul", mul, IIImul, CPURegs, 1>;
  def DIV     : Div32<Cpu0DivRem, 0x16, "div", IIIdiv>;
  def DIVu    : Div32<Cpu0DivRemU, 0x17, "divu", IIIdiv>;
  def AND     : ArithLogicR<0x18, "and", and, IIAlu, CPURegs, 1>;
  def OR      : ArithLogicR<0x19, "or", or, IIAlu, CPURegs, 1>;
  def XOR     : ArithLogicR<0x1A, "xor", xor, IIAlu, CPURegs, 1>;
  
  def SLT     : SetCC_R<0x20, "slt", setlt, CPURegs>;
  def SLTu    : SetCC_R<0x21, "sltu", setult, CPURegs>;
  
  def MFHI    : MoveFromLOHI<0x22, "mfhi", CPURegs, [HI]>;
  def MFLO    : MoveFromLOHI<0x23, "mflo", CPURegs, [LO]>;
  def MTHI    : MoveToLOHI<0x24, "mthi", CPURegs, [HI]>;
  def MTLO    : MoveToLOHI<0x25, "mtlo", CPURegs, [LO]>;
  
  def MULT    : Mult32<0x26, "mult", IIImul>;
  def MULTu   : Mult32<0x27, "multu", IIImul>;
  
  /// Shift Instructions
  // work, sra for ashr llvm IR instruction
  def SRA     : shift_rotate_imm32<0x1B, 0x00, "sra", sra>;
  def ROL     : shift_rotate_imm32<0x1C, 0x01, "rol", rotl>;
  def ROR     : shift_rotate_imm32<0x1D, 0x01, "ror", rotr>;
  def SHL     : shift_rotate_imm32<0x1E, 0x00, "shl", shl>;
  // work, srl for lshr llvm IR instruction
  def SHR     : shift_rotate_imm32<0x1F, 0x00, "shr", srl>;
  
  /// Jump and Branch Instructions
  def BEQ     : CBranch<0x27, "beq", seteq, CPURegs>;
  def BNE     : CBranch<0x28, "bne", setne, CPURegs>;
  
  def JMP     : UncondBranch<0x26, "jmp">;
  
  /// Jump and Branch Instructions
  def SWI     : JumpLink<0x2A, "swi">;
  def JSUB    : JumpLink<0x2B, "jsub">;
  def JR      : JumpFR<0x2C, "ret", CPURegs>;
  
  let isReturn=1, isTerminator=1, hasDelaySlot=1, isCodeGenOnly=1,
      isBarrier=1, hasCtrlDep=1, addr=0 in
    def RET   : FJ <0x2C, (outs), (ins CPURegs:$target),
                  "ret\t$target", [(Cpu0Ret CPURegs:$target)], IIBranch>;
  
  def IRET    : JumpFR<0x2D, "iret", CPURegs>;
  def JALR    : JumpLinkReg<0x2E, "jalr", CPURegs>;
  
  /// No operation
  let addr=0 in
    def NOP   : FJ<0, (outs), (ins), "nop", [], IIAlu>;
  
  // FrameIndexes are legalized when they are operands from load/store
  // instructions. The same not happens for stack address copies, so an
  // add op with mem ComplexPattern is used and the stack address copy
  // can be matched. It's similar to Sparc LEA_ADDRi
  def LEA_ADDiu : EffectiveAddress<"addiu\t$ra, $addr", CPURegs, mem_ea> {
    let isCodeGenOnly = 1;
  }
  
  //===----------------------------------------------------------------------===//
  //  Arbitrary patterns that map to one or more instructions
  //===----------------------------------------------------------------------===//
  
  // Small immediates
  def : Pat<(i32 immSExt16:$in),
            (ADDiu ZERO, imm:$in)>;
  def : Pat<(i32 immZExt16:$in),
            (ORi ZERO, imm:$in)>;
  def : Pat<(i32 immLow16Zero:$in),
            (LUi (HI16 imm:$in))>;
  
  // Arbitrary immediates
  def : Pat<(i32 imm:$imm),
            (ORi (LUi (HI16 imm:$imm)), (LO16 imm:$imm))>;
  
  def : Pat<(Cpu0JmpLink (i32 tglobaladdr:$dst)),
            (JSUB tglobaladdr:$dst)>;
  
  // hi/lo relocs
  def : Pat<(Cpu0Hi tglobaladdr:$in), (LUi tglobaladdr:$in)>;
  def : Pat<(Cpu0Lo tglobaladdr:$in), (ADDiu ZERO, tglobaladdr:$in)>;
  
  def : Pat<(add CPURegs:$hi, (Cpu0Lo tglobaladdr:$lo)),
            (ADDiu CPURegs:$hi, tglobaladdr:$lo)>;
  
  // gp_rel relocs
  def : Pat<(add CPURegs:$gp, (Cpu0GPRel tglobaladdr:$in)),
            (ADDiu CPURegs:$gp, tglobaladdr:$in)>;
  
  def : Pat<(not CPURegs:$in),
            (XORi CPURegs:$in, 1)>;
  
  
  // brcond patterns
  multiclass BrcondPats<RegisterClass RC, Instruction BEQOp, Instruction BNEOp,
                        Instruction SLTOp, Instruction SLTuOp, Instruction SLTiOp,
                        Instruction SLTiuOp, Register ZEROReg> {
  def : Pat<(brcond (i32 (setne RC:$lhs, 0)), bb:$dst),
                (BNEOp RC:$lhs, ZEROReg, bb:$dst)>;
  def : Pat<(brcond (i32 (seteq RC:$lhs, 0)), bb:$dst),
                (BEQOp RC:$lhs, ZEROReg, bb:$dst)>;
  
  def : Pat<(brcond (i32 (setge RC:$lhs, RC:$rhs)), bb:$dst),
                (BEQ (SLTOp RC:$lhs, RC:$rhs), ZERO, bb:$dst)>;
  def : Pat<(brcond (i32 (setuge RC:$lhs, RC:$rhs)), bb:$dst),
                (BEQ (SLTuOp RC:$lhs, RC:$rhs), ZERO, bb:$dst)>;
  def : Pat<(brcond (i32 (setge RC:$lhs, immSExt16:$rhs)), bb:$dst),
                (BEQ (SLTiOp RC:$lhs, immSExt16:$rhs), ZERO, bb:$dst)>;
  def : Pat<(brcond (i32 (setuge RC:$lhs, immSExt16:$rhs)), bb:$dst),
                (BEQ (SLTiuOp RC:$lhs, immSExt16:$rhs), ZERO, bb:$dst)>;
  
  def : Pat<(brcond (i32 (setle RC:$lhs, RC:$rhs)), bb:$dst),
                (BEQ (SLTOp RC:$rhs, RC:$lhs), ZERO, bb:$dst)>;
  def : Pat<(brcond (i32 (setule RC:$lhs, RC:$rhs)), bb:$dst),
                (BEQ (SLTuOp RC:$rhs, RC:$lhs), ZERO, bb:$dst)>;
  
  def : Pat<(brcond RC:$cond, bb:$dst),
                (BNEOp RC:$cond, ZEROReg, bb:$dst)>;
  }
  
  defm : BrcondPats<CPURegs, BEQ, BNE, SLT, SLTu, SLTi, SLTiu, ZERO>;
  
  // setcc patterns
  multiclass SeteqPats<RegisterClass RC, Instruction SLTiuOp, Instruction XOROp,
                       Instruction SLTuOp, Register ZEROReg> {
    def : Pat<(seteq RC:$lhs, RC:$rhs),
                  (SLTiuOp (XOROp RC:$lhs, RC:$rhs), 1)>;
    def : Pat<(setne RC:$lhs, RC:$rhs),
                  (SLTuOp ZEROReg, (XOROp RC:$lhs, RC:$rhs))>;
  }
  
  multiclass SetlePats<RegisterClass RC, Instruction SLTOp, Instruction SLTuOp> {
    def : Pat<(setle RC:$lhs, RC:$rhs),
                  (XORi (SLTOp RC:$rhs, RC:$lhs), 1)>;
    def : Pat<(setule RC:$lhs, RC:$rhs),
                  (XORi (SLTuOp RC:$rhs, RC:$lhs), 1)>;
  }
  
  multiclass SetgtPats<RegisterClass RC, Instruction SLTOp, Instruction SLTuOp> {
    def : Pat<(setgt RC:$lhs, RC:$rhs),
                  (SLTOp RC:$rhs, RC:$lhs)>;
    def : Pat<(setugt RC:$lhs, RC:$rhs),
                  (SLTuOp RC:$rhs, RC:$lhs)>;
  }
  
  multiclass SetgePats<RegisterClass RC, Instruction SLTOp, Instruction SLTuOp> {
    def : Pat<(setge RC:$lhs, RC:$rhs),
                  (XORi (SLTOp RC:$lhs, RC:$rhs), 1)>;
    def : Pat<(setuge RC:$lhs, RC:$rhs),
                  (XORi (SLTuOp RC:$lhs, RC:$rhs), 1)>;
  }
  
  multiclass SetgeImmPats<RegisterClass RC, Instruction SLTiOp,
                          Instruction SLTiuOp> {
    def : Pat<(setge RC:$lhs, immSExt16:$rhs),
                  (XORi (SLTiOp RC:$lhs, immSExt16:$rhs), 1)>;
    def : Pat<(setuge RC:$lhs, immSExt16:$rhs),
                  (XORi (SLTiuOp RC:$lhs, immSExt16:$rhs), 1)>;
  }
  
  defm : SeteqPats<CPURegs, SLTiu, XOR, SLTu, ZERO>;
  defm : SetlePats<CPURegs, SLT, SLTu>;
  defm : SetgtPats<CPURegs, SLT, SLTu>;
  defm : SetgePats<CPURegs, SLT, SLTu>;
  defm : SetgeImmPats<CPURegs, SLTi, SLTiu>;
  
  // Cpu0MCInstLower.cpp
  / Lower ".cpload $reg" to
  //  "lui   $gp, %hi(_gp_disp)"
  //  "addiu $gp, $gp, %lo(_gp_disp)"
  //  "addu  $gp, $gp, $t9"
  void Cpu0MCInstLower::LowerCPLOAD(SmallVector<MCInst, 4>& MCInsts) {
    ...
    CreateMCInst(MCInsts[0], Cpu0::LUi, GPReg, ZEROReg, SymHi);
    ...
  }
  
  // Lower ".cprestore offset" to "st $gp, offset($sp)".
  void Cpu0MCInstLower::LowerCPRESTORE(int64_t Offset,
                                       SmallVector<MCInst, 4>& MCInsts) {
      ...
      // lui   at,hi
      // add   at,at,sp
      MCInsts.resize(3);
      CreateMCInst(MCInsts[0], Cpu0::LUi, ATReg, ZEROReg, MCOperand::CreateImm(Hi));
      CreateMCInst(MCInsts[2], Cpu0::ADD, ATReg, ATReg, SPReg);
    }
  
  
  // Cpu0RegisterInfo.cpp
  let Namespace = "Cpu0" in {
    ...
    def T0   : Cpu0GPRReg< 12, "t0">,   DwarfRegNum<[12]>;
    ...
  }
  
  def CPURegs : RegisterClass<"Cpu0", [i32], 32, (add 
    T0, 
    // Reserved
    SP, LR, PC)>;
  
  // Remove SR RegisterClass since no SW in General register
  // Status Registers
  /* def SR   : RegisterClass<"Cpu0", [i32], 32, (add SW)>;*/


As modified from above, it remove the CMP instruction, SW register and 
related code from 11/1/Cpu0, and change from JEQ 24bits offset to BEQ 16 bits 
offset. And more, replace "ADDiu, SHL 16" with the efficient LUi instruction.


Cpu0 Verilog language changes
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. code-block:: c++

  // Operand width
  `define INT32 2'b11     // 32 bits
  `define INT24 2'b10     // 24 bits
  `define INT16 2'b01     // 16 bits
  `define BYTE  2'b00     // 8  bits
  
  // Reference web: http://ccckmit.wikidot.com/ocs:cpu0
  module cpu0(input clock, reset, output reg [2:0] tick, 
              output reg [31:0] ir, pc, mar, mdr, inout [31:0] dbus, 
              output reg m_en, m_rw, output reg [1:0] m_size);
    reg signed [31:0] R [0:15], HI, LO, SW; 
    // HI, LO: High and Low part of 64 bit result
    // SW: Status Word
    reg [7:0] op;
    reg [3:0] a, b, c;
    reg [4:0] c5;
    reg signed [31:0] c12, c16, uc16, c24, Ra, Rb, Rc, pc0; // pc0 : instruction pc
  
    // register name
    `define PC   R[15]   // Program Counter
    `define LR   R[14]   // Link Register
    `define SP   R[13]   // Stack Pointer
    // SW Flage
    `define N    SW[31] // Negative flag
    `define Z    SW[30] // Zero
    `define C    SW[29] // Carry
    `define V    SW[28] // Overflow
    `define I    SW[7]  // Hardware Interrupt Enable
    `define T    SW[6]  // Software Interrupt Enable
    `define M    SW[0]  // Mode bit
    // Instruction Opcode 
    parameter [7:0] LD=8'h01,ST=8'h02,LDB=8'h03,STB=8'h04,LDR=8'h05,STR=8'h06,
    LBR=8'h07,SBR=8'h08,ADDiu=8'h09,SLTi=8'h0A,SLTiu=8'h0B,ANDi=8'h0C,ORi=8'h0D,
    XORi=8'h0E,LUi=8'h0F,
    ADDu=8'h11,SUBu=8'h12,ADD=8'h13,SUB=8'h14,MUL=8'h15,DIV=8'h16,DIVu=8'h17,
    AND=8'h18,OR=8'h19,XOR=8'h1A,
    SRA=8'h1B,ROL=8'h1C,ROR=8'h1D,SHL=8'h1E,SHR=8'h1F,
    SLT=8'h20,SLTu=8'h21,
    MFHI=8'h22,MFLO=8'h23,MTHI=8'h24,MTLO=8'h25,MULT=8'h26,MULTu=8'h27,
    JMP=8'h26,BEQ=8'h27,BNE=8'h28,
    SWI=8'h2A,JSUB=8'h2B,RET=8'h2C,IRET=8'h2D,JALR=8'h2E;
    
    reg [2:0] state, next_state;
    parameter Reset=3'h0, Fetch=3'h1, Decode=3'h2, Execute=3'h3, WriteBack=3'h4;
  
    task memReadStart(input [31:0] addr, input [1:0] size); begin // Read Memory Word
      mar = addr;     // read(m[addr])
      m_rw = 1;     // Access Mode: read 
      m_en = 1;     // Enable read
      m_size = size;
    end endtask
  
    task memReadEnd(output [31:0] data); begin // Read Memory Finish, get data
      mdr = dbus; // get momory, dbus = m[addr]
      data = mdr; // return to data
      m_en = 0; // read complete
    end endtask
  
    // Write memory -- addr: address to write, data: date to write
    task memWriteStart(input [31:0] addr, input [31:0] data, input [1:0] size); begin 
      mar = addr;    // write(m[addr], data)
      mdr = data;
      m_rw = 0;    // access mode: write
      m_en = 1;     // Enable write
      m_size  = size;
    end endtask
  
    task memWriteEnd; begin // Write Memory Finish
      m_en = 0; // write complete
    end endtask
  
    task regSet(input [3:0] i, input [31:0] data); begin
      if (i!=0) R[i] = data;
    end endtask
  
    task regHILOSet(input [31:0] data1, input [31:0] data2); begin
      HI = data1;
      LO = data2;
    end endtask
  
    always @(posedge clock or posedge reset) begin
      if (reset) state <= Reset; 
      else state <= next_state;
    end
    
    always @(state or reset) begin
      m_en = 0;
      case (state)    
      Reset: begin 
        `PC = 0; tick = 0; R[0] = 0; SW = 0; `LR = -1; 
        next_state = reset?Reset:Fetch;
      end
      Fetch: begin  // Tick 1 : instruction fetch, throw PC to address bus, 
                    // memory.read(m[PC])
        memReadStart(`PC, `INT32);
        pc0  = `PC;
        `PC = `PC+4;
        next_state = Decode;
      end
      Decode: begin  // Tick 2 : instruction decode, ir = m[PC]
        memReadEnd(ir); // IR = dbus = m[PC]
        {op,a,b,c} = ir[31:12];
        c24 = $signed(ir[23:0]);
        c16 = $signed(ir[15:0]);
        uc16 = ir[15:0];
        c12 = $signed(ir[11:0]);
        c5  = ir[4:0];
        Ra = R[a];
        Rb = R[b];
        Rc = R[c];
        next_state = Execute;
      end
      Execute: begin // Tick 3 : instruction execution
        case (op)
        // load and store instructions
        LD:  memReadStart(Rb+c16, `INT32);      // LD Ra,[Rb+Cx]; Ra<=[Rb+Cx]
        ST:  memWriteStart(Rb+c16, Ra, `INT32); // ST Ra,[Rb+Cx]; Ra=>[Rb+Cx]
        LDB: memReadStart(Rb+c16, `BYTE);     // LDB Ra,[Rb+Cx]; Ra<=(byte)[Rb+Cx]
        STB: memWriteStart(Rb+c16, Ra, `BYTE);// STB Ra,[Rb+Cx]; Ra=>(byte)[Rb+Cx]
        LDR: memReadStart(Rb+Rc, `INT32);       // LDR Ra, [Rb+Rc]; Ra<=[Rb+ Rc]
        STR: memWriteStart(Rb+Rc, Ra, `INT32);  // STR Ra, [Rb+Rc]; Ra=>[Rb+ Rc]
        LBR: memReadStart(Rb+Rc, `BYTE);      // LBR Ra,[Rb+Rc]; Ra<=(byte)[Rb+Rc]
        SBR: memWriteStart(Rb+Rc, Ra, `BYTE); // SBR Ra,[Rb+Rc]; Ra=>(byte)[Rb+Rc]
        // Mathematic 
        ADDiu:  R[a] = Rb+c16;                   // ADDiu Ra, Rb+Cx; Ra<=Rb+Cx
  //      CMP: begin `N=(Ra-Rb<0);`Z=(Ra-Rb==0); end // CMP Ra, Rb; SW=(Ra >=< Rb)
        ADDu:  regSet(a, Rb+Rc);               // ADD Ra,Rb,Rc; Ra<=Rb+Rc
        ADD:   begin regSet(a, Rb+Rc); if (a < Rb) `V = 1; else `V =0; end
                                               // ADD Ra,Rb,Rc; Ra<=Rb+Rc
        SUBu:  regSet(a, Rb-Rc);               // SUB Ra,Rb,Rc; Ra<=Rb-Rc
        SUB:   begin regSet(a, Rb-Rc); if (Rb < 0 && Rc > 0 && a >= 0) 
               `V = 1; else `V =0; end         // SUB Ra,Rb,Rc; Ra<=Rb-Rc
        MUL:   regSet(a, Rb*Rc);               // MUL Ra,Rb,Rc;     Ra<=Rb*Rc
        DIVu:  regHILOSet(Ra%Rb, Ra/Rb);       // DIV Ra,Rb; HI<=Ra%Rb; LO<=Ra/Rb
        DIV:   begin regHILOSet(Ra%Rb, Ra/Rb); 
               if ((Ra < 0 && Rb < 0) || (Ra == 0)) `V = 1; 
               else `V =0; end  // DIVu Ra,Rb; HI<=Ra%Rb; LO<=Ra/Rb; With overflow
                                             // with exception overflow
        AND:   regSet(a, Rb&Rc);               // AND Ra,Rb,Rc; Ra<=(Rb and Rc)
        ANDi:  regSet(a, Rb&uc16);             // AND Ra,Rb,c16; Ra<=(Rb and c16)
        OR:    regSet(a, Rb|Rc);               // OR Ra,Rb,Rc; Ra<=(Rb or Rc)
        ORi:   regSet(a, Rb|uc16);             // OR Ra,Rb,c16; Ra<=(Rb or c16)
        XOR:   regSet(a, Rb^Rc);               // XOR Ra,Rb,Rc; Ra<=(Rb xor Rc)
        XORi:  regSet(a, Rb^uc16);             // XOR Ra,Rb,c16; Ra<=(Rb xor c16)
        LUi:   regSet(a, uc16<<16);
        SHL:   regSet(a, Rb<<c5);     // Shift Left; SHL Ra,Rb,Cx; Ra<=(Rb << Cx)
        SRA:   regSet(a, (Rb&'h80000000)|(Rb>>c5)); 
                                      // Shift Right with signed bit fill;
                                      // SHR Ra,Rb,Cx; Ra<=(Rb&0x80000000)|(Rb>>Cx)
        SHR:   regSet(a, Rb>>c5);     // Shift Right with 0 fill; 
                                      // SHR Ra,Rb,Cx; Ra<=(Rb >> Cx)
        ROL:   regSet(a, (Rb<<c5)|(Rb>>(32-c5)));     // Rotate Left;
        ROR:   regSet(a, (Rb>>c5)|(Rb<<(32-c5)));     // Rotate Right;
        // set
        SLT:   if (Rb < Rc) R[a]=1; else R[a]=0;
        SLTu:  if (Rb < Rc) R[a]=1; else R[a]=0;
        SLTi:  if (Rb < c16) R[a]=1; else R[a]=0;
        SLTiu: if (Rb < c16) R[a]=1; else R[a]=0;
        // Branch Instructions
        BEQ:   if (Ra==Rb) `PC=`PC+c16; 
        BNE:   if (Ra!=Rb) `PC=`PC+c16;
        MFLO:  regSet(a, LO);            // MFLO Ra; Ra<=LO
        MFHI:  regSet(a, HI);            // MFHI Ra; Ra<=HI
        MTLO:  LO = Ra;             // MTLO Ra; LO<=Ra
        MTHI:  HI = Ra;             // MTHI Ra; HI<=Ra
        MULT:  {HI, LO}=Ra*Rb; // MULT Ra,Rb; HI<=((Ra*Rb)>>32); 
                              // LO<=((Ra*Rb) and 0x00000000ffffffff);
                              // with exception overflow
        MULTu: {HI, LO}=Ra*Rb; // MULT Ra,Rb; HI<=((Ra*Rb)>>32); 
                              // LO<=((Ra*Rb) and 0x00000000ffffffff);
                              // without exception overflow
        // Jump Instructions
        JMP: `PC = `PC+c24;                  // JMP Cx; PC <= PC+Cx
        SWI: begin 
          `LR=`PC;`PC= c24; `I = 1'b1; 
        end // Software Interrupt; SWI Cx; LR <= PC; PC <= Cx; INT<=1
        JSUB:begin `LR=`PC;`PC=`PC + c24; end // JSUB Cx; LR<=PC; PC<=PC+Cx
        JALR:begin `LR=`PC;`PC=Ra; end // JALR Ra,Rb; Ra<=PC; PC<=Rb
        RET: begin `PC=`LR; end               // RET; PC <= LR
        IRET:begin 
          `PC=`LR;`I = 1'b0; 
        end // Interrupt Return; IRET; PC <= LR; INT<=0
        endcase
        next_state = WriteBack;
      end
      WriteBack: begin // Read/Write finish, close memory
        case (op)
          LD, LDB, LDR, LBR  : memReadEnd(R[a]); 
                                            //read memory complete
          ST, STB, STR, SBR  : memWriteEnd(); 
                                            // write memory complete
        endcase
        case (op)
        MULT, MULTu, DIV, DIVu, MTHI, MTLO :
          $display("%4dns %8x : %8x HI=%8x LO=%8x SW=%8x", $stime, pc0, ir, HI, 
          LO, SW);
        ST :
          if (R[b]+c16 == 28672)
            $display("%4dns %8x : %8x OUTPUT=%-d", $stime, pc0, ir, R[a]);
          else
            $display("%4dns %8x : %8x m[%-04d+%-04d]=%-d   SW=%8x", $stime, pc0, ir, 
            R[b], c16, R[a], SW);
        default : 
          $display("%4dns %8x : %8x R[%02d]=%-8x=%-d SW=%8x", $stime, pc0, ir, a, 
          R[a], R[a], SW);
        endcase
        SW = 0; // clear SW
        if (op==RET && `PC < 0) begin
          $display("RET to PC < 0, finished!");
          $finish;
        end
        next_state = Fetch;
      end                
      endcase
      pc = `PC;
    end
  
  endmodule
  
  module memory0(input clock, reset, en, rw, input [1:0] m_size, 
                  input [31:0] abus, dbus_in, output [31:0] dbus_out);
    reg [7:0] m [0:1536];
    reg [31:0] data;
  
    integer i;
    initial begin
      $readmemh("cpu0s.hex", m);
      for (i=0; i < 1024; i=i+4) begin
         $display("%8x: %8x", i, {m[i], m[i+1], m[i+2], m[i+3]});
      end
    end
  
    always @(clock or abus or en or rw or dbus_in) 
    begin
      if (abus >=0 && abus <= 1535) begin
        if (en == 1 && rw == 0) begin // r_w==0:write
          data = dbus_in;
          case (m_size)
          `BYTE:  {m[abus]} = dbus_in[7:0];
          `INT16: {m[abus], m[abus+1] } = dbus_in[15:0];
          `INT24: {m[abus], m[abus+1], m[abus+2]} = dbus_in[24:0];
          `INT32: {m[abus], m[abus+1], m[abus+2], m[abus+3]} = dbus_in;
          endcase
        end else if (en == 1 && rw == 1) begin// r_w==1:read
          case (m_size)
          `BYTE:  data = {8'h00  , 8'h00,   8'h00,   m[abus]      };
          `INT16: data = {8'h00  , 8'h00,   m[abus], m[abus+1]    };
          `INT24: data = {8'h00  , m[abus], m[abus+1], m[abus+2]  };
          `INT32: data = {m[abus], m[abus+1], m[abus+2], m[abus+3]};
          endcase
        end else
          data = 32'hZZZZZZZZ;
      end else
        data = 32'hZZZZZZZZ;
    end
    assign dbus_out = data;
  endmodule
  
  module main;
    reg clock, reset;
    wire [2:0] tick;
    wire [31:0] pc, ir, mar, mdr, dbus;
    wire m_en, m_rw;
    wire [1:0] m_size;
  
    cpu0 cpu(.clock(clock), .reset(reset), .pc(pc), .tick(tick), .ir(ir),
    .mar(mar), .mdr(mdr), .dbus(dbus), .m_en(m_en), .m_rw(m_rw), .m_size(m_size));
  
    memory0 mem(.clock(clock), .reset(reset), .en(m_en), .rw(m_rw), .m_size(m_size), 
    .abus(mar), .dbus_in(mdr), .dbus_out(dbus));
  
    initial
    begin
      clock = 0;
      reset = 1;
      #20 reset = 0;
      #30000 $finish;
    end
  
    always #10 clock=clock+1;
  endmodule


Run the redesigned Cpu0
~~~~~~~~~~~~~~~~~~~~~~~~

Run 11/2Cpu0 with ch11_2.cpp to get result as below. 
It match the expect value as comment in ch11_2.cpp.

.. code-block:: c++

  // ch11_2.cpp
  #include "InitRegs.h"
  
  #define OUT_MEM 0x7000 // 28672
  
  asm("addiu $sp, $zero, 1532");
  
  void print_integer(int x);
  int test_operators();
  void test_operators_asm();
  int test_control();
  
  int main()
  {
    int a = 0;
    a = test_operators(); // a = 13
    print_integer(a);
    a += test_control();	// a = 31
    print_integer(a);
    test_operators_asm();
  
    return a;
  }
  
  // For memory mapped I/O
  void print_integer(int x)
  {
    int *p = (int*)OUT_MEM;
    *p = x;
   return;
  }
  
  int test_operators()
  {
    int a = 11;
    int b = 2;
    int c = 0;
    int d = 0;
    int e, f, g, h, i, j, k, l = 0;
    unsigned int a1 = -5, k1 = 0;
    unsigned int b1 = 0xf0000001;
    unsigned int c1 = 0x000fffff;
  
    a1 = b1 + c1;
    c = a + b;
  //  c = 0x7fff0000 + 0x10000000;
    d = a - b;
    e = a * b;
    f = a / b;
    b = (a+1)%12;
    g = (a & b);
    h = (a | b);
    i = (a ^ b);
    j = (a << 2);
    k = (a >> 2);
    print_integer(k);
    k1 = (a1 >> 2);
    print_integer((int)k1);
  
    b = !a;
    int* p = &b;
    
    return c; // 13
  }
  
  void test_operators_asm()
  {
    asm("addiu $sp, $sp, -12");
    asm("st $2, 8($sp)");
    asm("st $3, 4($sp)");
    asm("st $4, 0($sp)");
    asm("lui $2, 0x7fff");
    asm("lui $3, 0x1000");
    asm("addu $4, $2, $3");
    asm("lui $2, 0x7fff");
    asm("lui $3, 0x1000");
    asm("add $4, $2, $3");	// overflow
    asm("lui $2, 0x8fff");
    asm("lui $3, 0x7000");
    asm("sub $4, $2, $3");	// overflow
    asm("lui $2, 0x0");
    asm("addiu $3, $0, -1");
    asm("sub $4, $2, $3");	// $4=1, no overflow
    asm("lui $2, -1");
    asm("ori $2, $2, 0xffff");	// $2=0xffffffff=-1
    asm("andi $2, $2, 0xffff");	// $2=0x0000ffff
    asm("shl $2, $2, 16");	// $2=0xffff0000
    asm("xori $2, $2, 0xffff");	// $2=0xffffffff=-1
    asm("addiu $3, $0, -1");	// $3=0xffffffff=-1
    asm("divu $2, $3");		// HI=0, LO=1
    asm("div $2, $3");		// HI=0, LO=1, overflow
    asm("xori $2, $2, 1");	// $2 = 0xffffffffe
    asm("rol $4, $2, 4");		// $2 = 0xfffffffef
    asm("ror $4, $2, 8");		// $2 = 0xfefffffff
    asm("ld $2, 8($sp)");
    asm("ld $3, 4($sp)");
    asm("ld $4, 0($sp)");
    asm("addiu $sp, $sp, 12");
  }
  
  int test_control()
  {
    int b = 1;
    int c = 2;
    int d = 3;
    int e = 4;
    int f = 5;
    
    if (b != 0) {
      b++;
    }
    if (c > 0) {
      c++;
    }
    if (d >= 0) {
      d++;
    }
    if (e < 0) {
      e++;
    }
    if (f <= 0) {
      f++;
    }
    
    return (b+c+d+e+f); // (2+3+4+4+5)=18
  }

.. code-block:: bash

  118-165-77-203:InputFiles Jonathan$ clang -target `llvm-config --host-target` 
  -c ch11_2.cpp -emit-llvm -o ch11_2.bc
  118-165-77-203:InputFiles Jonathan$ /Users/Jonathan/llvm/test/cmake_debug_build/
  bin/Debug/llc -march=cpu0 -relocation-model=static -filetype=obj -stats 
  ch11_2.bc -o ch11_2.cpu0.o
  ===-------------------------------------------------------------------------===
                            ... Statistics Collected ...
  ===-------------------------------------------------------------------------===
    ...
     5 del-jmp     - Number of useless jmp deleted
    ...
  
  118-165-77-203:InputFiles Jonathan$ /Users/Jonathan/llvm/test/cmake_debug_build/
  bin/Debug/llvm-objdump -d ch11_2.cpu0.o | tail -n +6| awk '{print "/* " $1 
  " */\t" $2 " " $3 " " $4 " " $5 "\t/* " $6"\t" $7" " $8" " $9" " $10 "\t

  118-165-77-203:redesign Jonathan$ ./cpu0s
  WARNING: cpu0s.v:227: $readmemh(cpu0s.hex): Not enough words in the file for 
  the requested range [0:1536].
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
  00000034: 09d005fc
  00000038: 09ddffe0
  0000003c: 02ed001c
  00000040: 09200000
  00000044: 022d0018
  00000048: 022d0014
  0000004c: 2b000038
  00000050: 022d0014
  00000054: 022d0000
  00000058: 2b000190
  0000005c: 2b0001b0
  00000060: 013d0014
  00000064: 11232000
  00000068: 022d0014
  0000006c: 022d0000
  00000070: 2b000178
  00000074: 2b00026c
  00000078: 012d0014
  0000007c: 01ed001c
  00000080: 09dd0020
  00000084: 2c000000
  00000088: 09ddffa0
  0000008c: 02ed005c
  00000090: 027d0058
  00000094: 0920000b
  00000098: 022d0054
  0000009c: 09200002
  000000a0: 022d0050
  000000a4: 09700000
  000000a8: 027d004c
  000000ac: 027d0048
  000000b0: 027d0028
  000000b4: 0920fffb
  000000b8: 022d0024
  000000bc: 027d0020
  000000c0: 0f20f000
  000000c4: 0d220001
  000000c8: 022d001c
  000000cc: 0f20000f
  000000d0: 0d22ffff
  000000d4: 022d0018
  000000d8: 013d001c
  000000dc: 11232000
  000000e0: 022d0024
  000000e4: 012d0050
  000000e8: 013d0054
  000000ec: 11232000
  000000f0: 022d004c
  000000f4: 012d0050
  000000f8: 013d0054
  000000fc: 12232000
  00000100: 022d0048
  00000104: 012d0050
  00000108: 013d0054
  0000010c: 15232000
  00000110: 022d0044
  00000114: 012d0050
  00000118: 013d0054
  0000011c: 16320000
  00000120: 23200000
  00000124: 022d0040
  00000128: 0f202aaa
  0000012c: 0d32aaab
  00000130: 012d0054
  00000134: 09220001
  00000138: 26230000
  0000013c: 22300000
  00000140: 1f43001f
  00000144: 1b330001
  00000148: 11334000
  0000014c: 0940000c
  00000150: 15334000
  00000154: 12223000
  00000158: 022d0050
  0000015c: 013d0054
  00000160: 18232000
  00000164: 022d003c
  00000168: 012d0050
  0000016c: 013d0054
  00000170: 19232000
  00000174: 022d0038
  00000178: 012d0050
  0000017c: 013d0054
  00000180: 1a232000
  00000184: 022d0034
  00000188: 012d0054
  0000018c: 1e220002
  00000190: 022d0030
  00000194: 012d0054
  00000198: 1b220002
  0000019c: 022d002c
  000001a0: 022d0000
  000001a4: 2b000044
  000001a8: 012d0024
  000001ac: 1f220002
  000001b0: 022d0020
  000001b4: 022d0000
  000001b8: 2b000030
  000001bc: 012d0054
  000001c0: 1a227000
  000001c4: 0b220001
  000001c8: 0c220001
  000001cc: 022d0050
  000001d0: 092d0050
  000001d4: 022d0014
  000001d8: 012d004c
  000001dc: 017d0058
  000001e0: 01ed005c
  000001e4: 09dd0060
  000001e8: 2c000000
  000001ec: 09ddfff8
  000001f0: 012d0008
  000001f4: 022d0004
  000001f8: 09207000
  000001fc: 022d0000
  00000200: 013d0004
  00000204: 02320000
  00000208: 09dd0008
  0000020c: 2c000000
  00000210: 09ddffe8
  00000214: 09200001
  00000218: 022d0014
  0000021c: 09200002
  00000220: 022d0010
  00000224: 09200003
  00000228: 022d000c
  0000022c: 09200004
  00000230: 022d0008
  00000234: 09200005
  00000238: 022d0004
  0000023c: 012d0014
  00000240: 2720000c
  00000244: 012d0014
  00000248: 09220001
  0000024c: 022d0014
  00000250: 012d0010
  00000254: 0a220001
  00000258: 2820000c
  0000025c: 012d0010
  00000260: 09220001
  00000264: 022d0010
  00000268: 012d000c
  0000026c: 0a220000
  00000270: 2820000c
  00000274: 012d000c
  00000278: 09220001
  0000027c: 022d000c
  00000280: 012d0008
  00000284: 0930ffff
  00000288: 20232000
  0000028c: 2820000c
  00000290: 012d0008
  00000294: 09220001
  00000298: 022d0008
  0000029c: 012d0004
  000002a0: 09300000
  000002a4: 20232000
  000002a8: 2820000c
  000002ac: 012d0004
  000002b0: 09220001
  000002b4: 022d0004
  000002b8: 012d0010
  000002bc: 013d0014
  000002c0: 11232000
  000002c4: 013d000c
  000002c8: 11223000
  000002cc: 013d0008
  000002d0: 11223000
  000002d4: 013d0004
  000002d8: 11223000
  000002dc: 09dd0018
  000002e0: 2c000000
  000002e4: 09ddfff4
  000002e8: 022d0008
  000002ec: 023d0004
  000002f0: 024d0000
  000002f4: 0f207fff
  000002f8: 0f301000
  000002fc: 11423000
  00000300: 0f207fff
  00000304: 0f301000
  00000308: 13423000
  0000030c: 0f208fff
  00000310: 0f307000
  00000314: 14423000
  00000318: 0f200000
  0000031c: 0930ffff
  00000320: 14423000
  00000324: 0f20ffff
  00000328: 0d22ffff
  0000032c: 0c22ffff
  00000330: 1e220010
  00000334: 0e22ffff
  00000338: 0930ffff
  0000033c: 17230000
  00000340: 16230000
  00000344: 0e220001
  00000348: 1c421004
  0000034c: 1d421008
  00000350: 012d0008
  00000354: 013d0004
  00000358: 014d0000
  0000035c: 09dd000c
  00000360: 2c000000
  00000364: xxxxxxxx
  00000368: xxxxxxxx
  0000036c: xxxxxxxx
  00000370: xxxxxxxx
  00000374: xxxxxxxx
  00000378: xxxxxxxx
  0000037c: xxxxxxxx
  00000380: xxxxxxxx
  00000384: xxxxxxxx
  00000388: xxxxxxxx
  0000038c: xxxxxxxx
  00000390: xxxxxxxx
  00000394: xxxxxxxx
  00000398: xxxxxxxx
  0000039c: xxxxxxxx
  000003a0: xxxxxxxx
  000003a4: xxxxxxxx
  000003a8: xxxxxxxx
  000003ac: xxxxxxxx
  000003b0: xxxxxxxx
  000003b4: xxxxxxxx
  000003b8: xxxxxxxx
  000003bc: xxxxxxxx
  000003c0: xxxxxxxx
  000003c4: xxxxxxxx
  000003c8: xxxxxxxx
  000003cc: xxxxxxxx
  000003d0: xxxxxxxx
  000003d4: xxxxxxxx
  000003d8: xxxxxxxx
  000003dc: xxxxxxxx
  000003e0: xxxxxxxx
  000003e4: xxxxxxxx
  000003e8: xxxxxxxx
  000003ec: xxxxxxxx
  000003f0: xxxxxxxx
  000003f4: xxxxxxxx
  000003f8: xxxxxxxx
  000003fc: xxxxxxxx
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
  1130ns 00000034 : 09d005fc R[13]=000005fc=1532       SW=00000000
  1210ns 00000038 : 09ddffe0 R[13]=000005dc=1500       SW=00000000
  1290ns 0000003c : 02ed001c m[1500+28  ]=-1           SW=00000000
  1370ns 00000040 : 09200000 R[02]=00000000=0          SW=00000000
  1450ns 00000044 : 022d0018 m[1500+24  ]=0            SW=00000000
  1530ns 00000048 : 022d0014 m[1500+20  ]=0            SW=00000000
  1610ns 0000004c : 2b000038 R[00]=00000000=0          SW=00000000
  1690ns 00000088 : 09ddffa0 R[13]=0000057c=1404       SW=00000000
  1770ns 0000008c : 02ed005c m[1404+92  ]=80           SW=00000000
  1850ns 00000090 : 027d0058 m[1404+88  ]=0            SW=00000000
  1930ns 00000094 : 0920000b R[02]=0000000b=11         SW=00000000
  2010ns 00000098 : 022d0054 m[1404+84  ]=11           SW=00000000
  2090ns 0000009c : 09200002 R[02]=00000002=2          SW=00000000
  2170ns 000000a0 : 022d0050 m[1404+80  ]=2            SW=00000000
  2250ns 000000a4 : 09700000 R[07]=00000000=0          SW=00000000
  2330ns 000000a8 : 027d004c m[1404+76  ]=0            SW=00000000
  2410ns 000000ac : 027d0048 m[1404+72  ]=0            SW=00000000
  2490ns 000000b0 : 027d0028 m[1404+40  ]=0            SW=00000000
  2570ns 000000b4 : 0920fffb R[02]=fffffffb=-5         SW=00000000
  2650ns 000000b8 : 022d0024 m[1404+36  ]=-5           SW=00000000
  2730ns 000000bc : 027d0020 m[1404+32  ]=0            SW=00000000
  2810ns 000000c0 : 0f20f000 R[02]=f0000000=-268435456 SW=00000000
  2890ns 000000c4 : 0d220001 R[02]=f0000001=-268435455 SW=00000000
  2970ns 000000c8 : 022d001c m[1404+28  ]=-268435455   SW=00000000
  3050ns 000000cc : 0f20000f R[02]=000f0000=983040     SW=00000000
  3130ns 000000d0 : 0d22ffff R[02]=000fffff=1048575    SW=00000000
  3210ns 000000d4 : 022d0018 m[1404+24  ]=1048575      SW=00000000
  3290ns 000000d8 : 013d001c R[03]=f0000001=-268435455 SW=00000000
  3370ns 000000dc : 11232000 R[02]=f0100000=-267386880 SW=00000000
  3450ns 000000e0 : 022d0024 m[1404+36  ]=-267386880   SW=00000000
  3530ns 000000e4 : 012d0050 R[02]=00000002=2          SW=00000000
  3610ns 000000e8 : 013d0054 R[03]=0000000b=11         SW=00000000
  3690ns 000000ec : 11232000 R[02]=0000000d=13         SW=00000000
  3770ns 000000f0 : 022d004c m[1404+76  ]=13           SW=00000000
  3850ns 000000f4 : 012d0050 R[02]=00000002=2          SW=00000000
  3930ns 000000f8 : 013d0054 R[03]=0000000b=11         SW=00000000
  4010ns 000000fc : 12232000 R[02]=00000009=9          SW=00000000
  4090ns 00000100 : 022d0048 m[1404+72  ]=9            SW=00000000
  4170ns 00000104 : 012d0050 R[02]=00000002=2          SW=00000000
  4250ns 00000108 : 013d0054 R[03]=0000000b=11         SW=00000000
  4330ns 0000010c : 15232000 R[02]=00000016=22         SW=00000000
  4410ns 00000110 : 022d0044 m[1404+68  ]=22           SW=00000000
  4490ns 00000114 : 012d0050 R[02]=00000002=2          SW=00000000
  4570ns 00000118 : 013d0054 R[03]=0000000b=11         SW=00000000
  4650ns 0000011c : 16320000 HI=00000001 LO=00000005 SW=00000000
  4730ns 00000120 : 23200000 R[02]=00000005=5          SW=00000000
  4810ns 00000124 : 022d0040 m[1404+64  ]=5            SW=00000000
  4890ns 00000128 : 0f202aaa R[02]=2aaa0000=715784192  SW=00000000
  4970ns 0000012c : 0d32aaab R[03]=2aaaaaab=715827883  SW=00000000
  5050ns 00000130 : 012d0054 R[02]=0000000b=11         SW=00000000
  5130ns 00000134 : 09220001 R[02]=0000000c=12         SW=00000000
  5210ns 00000138 : 26230000 HI=00000002 LO=00000004 SW=00000000
  5290ns 0000013c : 22300000 R[03]=00000002=2          SW=00000000
  5370ns 00000140 : 1f43001f R[04]=00000000=0          SW=00000000
  5450ns 00000144 : 1b330001 R[03]=00000001=1          SW=00000000
  5530ns 00000148 : 11334000 R[03]=00000001=1          SW=00000000
  5610ns 0000014c : 0940000c R[04]=0000000c=12         SW=00000000
  5690ns 00000150 : 15334000 R[03]=0000000c=12         SW=00000000
  5770ns 00000154 : 12223000 R[02]=00000000=0          SW=00000000
  5850ns 00000158 : 022d0050 m[1404+80  ]=0            SW=00000000
  5930ns 0000015c : 013d0054 R[03]=0000000b=11         SW=00000000
  6010ns 00000160 : 18232000 R[02]=00000000=0          SW=00000000
  6090ns 00000164 : 022d003c m[1404+60  ]=0            SW=00000000
  6170ns 00000168 : 012d0050 R[02]=00000000=0          SW=00000000
  6250ns 0000016c : 013d0054 R[03]=0000000b=11         SW=00000000
  6330ns 00000170 : 19232000 R[02]=0000000b=11         SW=00000000
  6410ns 00000174 : 022d0038 m[1404+56  ]=11           SW=00000000
  6490ns 00000178 : 012d0050 R[02]=00000000=0          SW=00000000
  6570ns 0000017c : 013d0054 R[03]=0000000b=11         SW=00000000
  6650ns 00000180 : 1a232000 R[02]=0000000b=11         SW=00000000
  6730ns 00000184 : 022d0034 m[1404+52  ]=11           SW=00000000
  6810ns 00000188 : 012d0054 R[02]=0000000b=11         SW=00000000
  6890ns 0000018c : 1e220002 R[02]=0000002c=44         SW=00000000
  6970ns 00000190 : 022d0030 m[1404+48  ]=44           SW=00000000
  7050ns 00000194 : 012d0054 R[02]=0000000b=11         SW=00000000
  7130ns 00000198 : 1b220002 R[02]=00000002=2          SW=00000000
  7210ns 0000019c : 022d002c m[1404+44  ]=2            SW=00000000
  7290ns 000001a0 : 022d0000 m[1404+0   ]=2            SW=00000000
  7370ns 000001a4 : 2b000044 R[00]=00000000=0          SW=00000000
  7450ns 000001ec : 09ddfff8 R[13]=00000574=1396       SW=00000000
  7530ns 000001f0 : 012d0008 R[02]=00000002=2          SW=00000000
  7610ns 000001f4 : 022d0004 m[1396+4   ]=2            SW=00000000
  7690ns 000001f8 : 09207000 R[02]=00007000=28672      SW=00000000
  7770ns 000001fc : 022d0000 m[1396+0   ]=28672        SW=00000000
  7850ns 00000200 : 013d0004 R[03]=00000002=2          SW=00000000
  7930ns 00000204 : 02320000 OUTPUT=2         
  8010ns 00000208 : 09dd0008 R[13]=0000057c=1404       SW=00000000
  8090ns 0000020c : 2c000000 R[00]=00000000=0          SW=00000000
  8170ns 000001a8 : 012d0024 R[02]=f0100000=-267386880 SW=00000000
  8250ns 000001ac : 1f220002 R[02]=3c040000=1006895104 SW=00000000
  8330ns 000001b0 : 022d0020 m[1404+32  ]=1006895104   SW=00000000
  8410ns 000001b4 : 022d0000 m[1404+0   ]=1006895104   SW=00000000
  8490ns 000001b8 : 2b000030 R[00]=00000000=0          SW=00000000
  8570ns 000001ec : 09ddfff8 R[13]=00000574=1396       SW=00000000
  8650ns 000001f0 : 012d0008 R[02]=3c040000=1006895104 SW=00000000
  8730ns 000001f4 : 022d0004 m[1396+4   ]=1006895104   SW=00000000
  8810ns 000001f8 : 09207000 R[02]=00007000=28672      SW=00000000
  8890ns 000001fc : 022d0000 m[1396+0   ]=28672        SW=00000000
  8970ns 00000200 : 013d0004 R[03]=3c040000=1006895104 SW=00000000
  9050ns 00000204 : 02320000 OUTPUT=1006895104
  9130ns 00000208 : 09dd0008 R[13]=0000057c=1404       SW=00000000
  9210ns 0000020c : 2c000000 R[00]=00000000=0          SW=00000000
  9290ns 000001bc : 012d0054 R[02]=0000000b=11         SW=00000000
  9370ns 000001c0 : 1a227000 R[02]=0000000b=11         SW=00000000
  9450ns 000001c4 : 0b220001 R[02]=00000000=0          SW=00000000
  9530ns 000001c8 : 0c220001 R[02]=00000000=0          SW=00000000
  9610ns 000001cc : 022d0050 m[1404+80  ]=0            SW=00000000
  9690ns 000001d0 : 092d0050 R[02]=000005cc=1484       SW=00000000
  9770ns 000001d4 : 022d0014 m[1404+20  ]=1484         SW=00000000
  9850ns 000001d8 : 012d004c R[02]=0000000d=13         SW=00000000
  9930ns 000001dc : 017d0058 R[07]=00000000=0          SW=00000000
  10010ns 000001e0 : 01ed005c R[14]=00000050=80         SW=00000000
  10090ns 000001e4 : 09dd0060 R[13]=000005dc=1500       SW=00000000
  10170ns 000001e8 : 2c000000 R[00]=00000000=0          SW=00000000
  10250ns 00000050 : 022d0014 m[1500+20  ]=13           SW=00000000
  10330ns 00000054 : 022d0000 m[1500+0   ]=13           SW=00000000
  10410ns 00000058 : 2b000190 R[00]=00000000=0          SW=00000000
  10490ns 000001ec : 09ddfff8 R[13]=000005d4=1492       SW=00000000
  10570ns 000001f0 : 012d0008 R[02]=0000000d=13         SW=00000000
  10650ns 000001f4 : 022d0004 m[1492+4   ]=13           SW=00000000
  10730ns 000001f8 : 09207000 R[02]=00007000=28672      SW=00000000
  10810ns 000001fc : 022d0000 m[1492+0   ]=28672        SW=00000000
  10890ns 00000200 : 013d0004 R[03]=0000000d=13         SW=00000000
  10970ns 00000204 : 02320000 OUTPUT=13        
  11050ns 00000208 : 09dd0008 R[13]=000005dc=1500       SW=00000000
  11130ns 0000020c : 2c000000 R[00]=00000000=0          SW=00000000
  11210ns 0000005c : 2b0001b0 R[00]=00000000=0          SW=00000000
  11290ns 00000210 : 09ddffe8 R[13]=000005c4=1476       SW=00000000
  11370ns 00000214 : 09200001 R[02]=00000001=1          SW=00000000
  11450ns 00000218 : 022d0014 m[1476+20  ]=1            SW=00000000
  11530ns 0000021c : 09200002 R[02]=00000002=2          SW=00000000
  11610ns 00000220 : 022d0010 m[1476+16  ]=2            SW=00000000
  11690ns 00000224 : 09200003 R[02]=00000003=3          SW=00000000
  11770ns 00000228 : 022d000c m[1476+12  ]=3            SW=00000000
  11850ns 0000022c : 09200004 R[02]=00000004=4          SW=00000000
  11930ns 00000230 : 022d0008 m[1476+8   ]=4            SW=00000000
  12010ns 00000234 : 09200005 R[02]=00000005=5          SW=00000000
  12090ns 00000238 : 022d0004 m[1476+4   ]=5            SW=00000000
  12170ns 0000023c : 012d0014 R[02]=00000001=1          SW=00000000
  12250ns 00000240 : 2720000c HI=00000002 LO=00000004 SW=00000000
  12330ns 00000244 : 012d0014 R[02]=00000001=1          SW=00000000
  12410ns 00000248 : 09220001 R[02]=00000002=2          SW=00000000
  12490ns 0000024c : 022d0014 m[1476+20  ]=2            SW=00000000
  12570ns 00000250 : 012d0010 R[02]=00000002=2          SW=00000000
  12650ns 00000254 : 0a220001 R[02]=00000000=0          SW=00000000
  12730ns 00000258 : 2820000c R[02]=00000000=0          SW=00000000
  12810ns 0000025c : 012d0010 R[02]=00000002=2          SW=00000000
  12890ns 00000260 : 09220001 R[02]=00000003=3          SW=00000000
  12970ns 00000264 : 022d0010 m[1476+16  ]=3            SW=00000000
  13050ns 00000268 : 012d000c R[02]=00000003=3          SW=00000000
  13130ns 0000026c : 0a220000 R[02]=00000000=0          SW=00000000
  13210ns 00000270 : 2820000c R[02]=00000000=0          SW=00000000
  13290ns 00000274 : 012d000c R[02]=00000003=3          SW=00000000
  13370ns 00000278 : 09220001 R[02]=00000004=4          SW=00000000
  13450ns 0000027c : 022d000c m[1476+12  ]=4            SW=00000000
  13530ns 00000280 : 012d0008 R[02]=00000004=4          SW=00000000
  13610ns 00000284 : 0930ffff R[03]=ffffffff=-1         SW=00000000
  13690ns 00000288 : 20232000 R[02]=00000001=1          SW=00000000
  13770ns 0000028c : 2820000c R[02]=00000001=1          SW=00000000
  13850ns 0000029c : 012d0004 R[02]=00000005=5          SW=00000000
  13930ns 000002a0 : 09300000 R[03]=00000000=0          SW=00000000
  14010ns 000002a4 : 20232000 R[02]=00000001=1          SW=00000000
  14090ns 000002a8 : 2820000c R[02]=00000001=1          SW=00000000
  14170ns 000002b8 : 012d0010 R[02]=00000003=3          SW=00000000
  14250ns 000002bc : 013d0014 R[03]=00000002=2          SW=00000000
  14330ns 000002c0 : 11232000 R[02]=00000005=5          SW=00000000
  14410ns 000002c4 : 013d000c R[03]=00000004=4          SW=00000000
  14490ns 000002c8 : 11223000 R[02]=00000009=9          SW=00000000
  14570ns 000002cc : 013d0008 R[03]=00000004=4          SW=00000000
  14650ns 000002d0 : 11223000 R[02]=0000000d=13         SW=00000000
  14730ns 000002d4 : 013d0004 R[03]=00000005=5          SW=00000000
  14810ns 000002d8 : 11223000 R[02]=00000012=18         SW=00000000
  14890ns 000002dc : 09dd0018 R[13]=000005dc=1500       SW=00000000
  14970ns 000002e0 : 2c000000 R[00]=00000000=0          SW=00000000
  15050ns 00000060 : 013d0014 R[03]=0000000d=13         SW=00000000
  15130ns 00000064 : 11232000 R[02]=0000001f=31         SW=00000000
  15210ns 00000068 : 022d0014 m[1500+20  ]=31           SW=00000000
  15290ns 0000006c : 022d0000 m[1500+0   ]=31           SW=00000000
  15370ns 00000070 : 2b000178 R[00]=00000000=0          SW=00000000
  15450ns 000001ec : 09ddfff8 R[13]=000005d4=1492       SW=00000000
  15530ns 000001f0 : 012d0008 R[02]=0000001f=31         SW=00000000
  15610ns 000001f4 : 022d0004 m[1492+4   ]=31           SW=00000000
  15690ns 000001f8 : 09207000 R[02]=00007000=28672      SW=00000000
  15770ns 000001fc : 022d0000 m[1492+0   ]=28672        SW=00000000
  15850ns 00000200 : 013d0004 R[03]=0000001f=31         SW=00000000
  15930ns 00000204 : 02320000 OUTPUT=31        
  16010ns 00000208 : 09dd0008 R[13]=000005dc=1500       SW=00000000
  16090ns 0000020c : 2c000000 R[00]=00000000=0          SW=00000000
  16170ns 00000074 : 2b00026c R[00]=00000000=0          SW=00000000
  16250ns 000002e4 : 09ddfff4 R[13]=000005d0=1488       SW=00000000
  16330ns 000002e8 : 022d0008 m[1488+8   ]=28672        SW=00000000
  16410ns 000002ec : 023d0004 m[1488+4   ]=31           SW=00000000
  16490ns 000002f0 : 024d0000 m[1488+0   ]=12           SW=00000000
  16570ns 000002f4 : 0f207fff R[02]=7fff0000=2147418112 SW=00000000
  16650ns 000002f8 : 0f301000 R[03]=10000000=268435456  SW=00000000
  16730ns 000002fc : 11423000 R[04]=8fff0000=-1879113728 SW=00000000
  16810ns 00000300 : 0f207fff R[02]=7fff0000=2147418112 SW=00000000
  16890ns 00000304 : 0f301000 R[03]=10000000=268435456  SW=00000000
  16970ns 00000308 : 13423000 R[04]=8fff0000=-1879113728 SW=10000000
  17050ns 0000030c : 0f208fff R[02]=8fff0000=-1879113728 SW=00000000
  17130ns 00000310 : 0f307000 R[03]=70000000=1879048192 SW=00000000
  17210ns 00000314 : 14423000 R[04]=1fff0000=536805376  SW=10000000
  17290ns 00000318 : 0f200000 R[02]=00000000=0          SW=00000000
  17370ns 0000031c : 0930ffff R[03]=ffffffff=-1         SW=00000000
  17450ns 00000320 : 14423000 R[04]=00000001=1          SW=00000000
  17530ns 00000324 : 0f20ffff R[02]=ffff0000=-65536     SW=00000000
  17610ns 00000328 : 0d22ffff R[02]=ffffffff=-1         SW=00000000
  17690ns 0000032c : 0c22ffff R[02]=0000ffff=65535      SW=00000000
  17770ns 00000330 : 1e220010 R[02]=ffff0000=-65536     SW=00000000
  17850ns 00000334 : 0e22ffff R[02]=ffffffff=-1         SW=00000000
  17930ns 00000338 : 0930ffff R[03]=ffffffff=-1         SW=00000000
  18010ns 0000033c : 17230000 HI=00000000 LO=00000001 SW=00000000
  18090ns 00000340 : 16230000 HI=00000000 LO=00000001 SW=10000000
  18170ns 00000344 : 0e220001 R[02]=fffffffe=-2         SW=00000000
  18250ns 00000348 : 1c421004 R[04]=ffffffef=-17        SW=00000000
  18330ns 0000034c : 1d421008 R[04]=feffffff=-16777217  SW=00000000
  18410ns 00000350 : 012d0008 R[02]=00007000=28672      SW=00000000
  18490ns 00000354 : 013d0004 R[03]=0000001f=31         SW=00000000
  18570ns 00000358 : 014d0000 R[04]=0000000c=12         SW=00000000
  18650ns 0000035c : 09dd000c R[13]=000005dc=1500       SW=00000000
  18730ns 00000360 : 2c000000 R[00]=00000000=0          SW=00000000
  18810ns 00000078 : 012d0014 R[02]=0000001f=31         SW=00000000
  18890ns 0000007c : 01ed001c R[14]=ffffffff=-1         SW=00000000
  18970ns 00000080 : 09dd0020 R[13]=000005fc=1532       SW=00000000
  19050ns 00000084 : 2c000000 R[00]=00000000=0          SW=00000000
  RET to PC < 0, finished!

Run with ch7_1_1.cpp, it reduce some branch from pair instructions "CMP, JXX" 
to 1 single instruction ether is BEQ or BNE, as follows,

.. code-block:: bash

  118-165-77-203:InputFiles Jonathan$ /Users/Jonathan/llvm/test/cmake_debug_build/bin/Debug/llc -march=cpu0 -relocation-model=static -filetype=asm ch7_1_1.bc -o ch7_1_1.cpu0.s
  118-165-77-203:InputFiles Jonathan$ cat ch7_1_1.cpu0.s 
  	.section .mdebug.abi32
  	.previous
  	.file	"ch7_1_1.bc"
  	.text
  	.globl	main
  	.align	2
  	.type	main,@function
  	.ent	main                    # @main
  main:
  	.frame	$sp,40,$lr
  	.mask 	0x00000000,0
  	.set	noreorder
  	.set	nomacro
  # BB#0:
  	addiu	$sp, $sp, -40
  	addiu	$3, $zero, 0
  	st	$3, 36($sp)
  	st	$3, 32($sp)
  	addiu	$2, $zero, 1
  	st	$2, 28($sp)
  	addiu	$4, $zero, 2
  	st	$4, 24($sp)
  	addiu	$4, $zero, 3
  	st	$4, 20($sp)
  	addiu	$4, $zero, 4
  	st	$4, 16($sp)
  	addiu	$4, $zero, 5
  	st	$4, 12($sp)
  	addiu	$4, $zero, 6
  	st	$4, 8($sp)
  	addiu	$4, $zero, 7
  	st	$4, 4($sp)
  	addiu	$4, $zero, 8
  	st	$4, 0($sp)
  	ld	$4, 32($sp)
  	bne	$4, $zero, $BB0_2
  # BB#1:
  	ld	$4, 32($sp)
  	addiu	$4, $4, 1
  	st	$4, 32($sp)
  $BB0_2:
  	ld	$4, 28($sp)
  	beq	$4, $zero, $BB0_4
  # BB#3:
  	ld	$4, 28($sp)
  	addiu	$4, $4, 1
  	st	$4, 28($sp)
  $BB0_4:
  	ld	$4, 24($sp)
  	slti	$4, $4, 1
  	bne	$4, $zero, $BB0_6
  # BB#5:
  	ld	$4, 24($sp)
  	addiu	$4, $4, 1
  	st	$4, 24($sp)
  $BB0_6:
  	ld	$4, 20($sp)
  	slti	$4, $4, 0
  	bne	$4, $zero, $BB0_8
  # BB#7:
  	ld	$4, 20($sp)
  	addiu	$4, $4, 1
  	st	$4, 20($sp)
  $BB0_8:
  	ld	$4, 16($sp)
  	addiu	$5, $zero, -1
  	slt	$4, $5, $4
  	bne	$4, $zero, $BB0_10
  # BB#9:
  	ld	$4, 16($sp)
  	addiu	$4, $4, 1
  	st	$4, 16($sp)
  $BB0_10:
  	ld	$4, 12($sp)
  	slt	$3, $3, $4
  	bne	$3, $zero, $BB0_12
  # BB#11:
  	ld	$3, 12($sp)
  	addiu	$3, $3, 1
  	st	$3, 12($sp)
  $BB0_12:
  	ld	$3, 8($sp)
  	slt	$2, $2, $3
  	bne	$2, $zero, $BB0_14
  # BB#13:
  	ld	$2, 8($sp)
  	addiu	$2, $2, 1
  	st	$2, 8($sp)
  $BB0_14:
  	ld	$2, 4($sp)
  	slti	$2, $2, 1
  	bne	$2, $zero, $BB0_16
  # BB#15:
  	ld	$2, 4($sp)
  	addiu	$2, $2, 1
  	st	$2, 4($sp)
  $BB0_16:
  	ld	$2, 4($sp)
  	ld	$3, 0($sp)
  	slt	$2, $3, $2
  	beq	$2, $zero, $BB0_18
  # BB#17:
  	ld	$2, 0($sp)
  	addiu	$2, $2, 1
  	st	$2, 0($sp)
  $BB0_18:
  	ld	$2, 28($sp)
  	ld	$3, 32($sp)
  	beq	$3, $2, $BB0_20
  # BB#19:
  	ld	$2, 32($sp)
  	addiu	$2, $2, 1
  	st	$2, 32($sp)
  $BB0_20:
  	ld	$2, 32($sp)
  	addiu	$sp, $sp, 40
  	ret	$lr
  	.set	macro
  	.set	reorder
  	.end	main
  $tmp1:
  	.size	main, ($tmp1)-main

