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

Chapter12_1/ support this optimization algorithm include the added codes as follows,

.. rubric:: LLVMBackendTutorialExampleCode/Chapter12_1/CMakeLists.txt
.. code-block:: c++

  add_llvm_target(Cpu0CodeGen
    ...
    Cpu0DelUselessJMP.cpp
    ...
    )
  
.. rubric:: LLVMBackendTutorialExampleCode/Chapter12_1/Cpu0.h
.. code-block:: c++

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

.. rubric:: LLVMBackendTutorialExampleCode/Chapter12_1/Cpu0DelUselessJMP.cpp
.. literalinclude:: ../LLVMBackendTutorialExampleCode/Chapter12_1/Cpu0DelUselessJMP.cpp


As above code, except Cpu0DelUselessJMP.cpp, other files changed for register 
class DelJmp as a functional pass. As comment of above code, MBB is the current 
block and MBBN is the next block. For the last instruction of every MBB, we 
check if it is the JMP instruction as well as 
its Operand is the next basic block. 
By getMBB() in MachineOperand, you can get the MBB address. 
For the member function of MachineOperand, please check 
include/llvm/CodeGen/MachineOperand.h
Let's run Chapter12_1/ with ch12_1.cpp to explain it easier.

.. rubric:: LLVMBackendTutorialExampleCode/InputFiles/ch12_1.cpp
.. literalinclude:: ../LLVMBackendTutorialExampleCode/InputFiles/ch12_1.cpp
    :start-after: /// start

.. code-block:: bash

  118-165-78-10:InputFiles Jonathan$ clang -target mips-unknown-linux-gnu 
  -c ch12_1.cpp -emit-llvm -o ch12_1.bc
  118-165-78-10:InputFiles Jonathan$ /Users/Jonathan/llvm/test/cmake_debug_build/
  bin/Debug/llc -march=cpu0 -relocation-model=static -filetype=asm -stats 
  ch12_1.bc -o ch12_1.cpu0.s
  ===-------------------------------------------------------------------------===
                            ... Statistics Collected ...
  ===-------------------------------------------------------------------------===
   ...
   2 del-jmp        - Number of useless jmp deleted
   ...
  
    .section .mdebug.abi32
    .previous
    .file "ch12_1.bc"
    .text
    .globl  main
    .align  2
    .type main,@function
    .ent  main                    # @main
  main:
    .frame  $sp,16,$lr
    .mask   0x00000000,0
    .set  noreorder
    .set  nomacro
  # BB#0:
    addiu $sp, $sp, -16
    addiu $2, $zero, 0
    st  $2, 12($sp)
    st  $2, 8($sp)
    addiu $2, $zero, 1
    st  $2, 4($sp)
    addiu $2, $zero, 2
    st  $2, 0($sp)
    ld  $2, 8($sp)
    bne $2, $zero, $BB0_2
  # BB#1:
    ld  $2, 8($sp)
    addiu $2, $2, 1
    st  $2, 8($sp)
  $BB0_2:
    ld  $2, 4($sp)
    bne $2, $zero, $BB0_4
    jmp $BB0_3
  $BB0_4:
    ld  $2, 4($sp)
    addiu $3, $zero, -1
    slt $2, $3, $2
    bne $2, $zero, $BB0_6
    jmp $BB0_5
  $BB0_3:
    ld  $2, 4($sp)
    ld  $3, 8($sp)
    addu  $2, $3, $2
    st  $2, 8($sp)
    jmp $BB0_6
  $BB0_5:
    ld  $2, 8($sp)
    addiu $3, $2, -1
    st  $3, 8($sp)
    st  $2, 8($sp)
  $BB0_6:
    ld  $2, 0($sp)
    slti  $2, $2, 1
    bne $2, $zero, $BB0_8
  # BB#7:
    ld  $2, 0($sp)
    addiu $2, $2, 1
    st  $2, 0($sp)
  $BB0_8:
    ld  $2, 8($sp)
    addiu $sp, $sp, 16
    ret $lr
    .set  macro
    .set  reorder
    .end  main
  $tmp1:
    .size main, ($tmp1)-main


The terminal display "Number of useless jmp deleted" by ``llc -stats`` option 
because we set the "STATISTIC(NumDelJmp, "Number of useless jmp deleted")" in 
code. It delete 2 jmp instructions from block "# BB#0" and "$BB0_6".
You can check it by ``llc -enable-cpu0-del-useless-jmp=false`` option to see 
the difference from no optimization version.
If you run with ch8_1_1.cpp, will find 10 jmp instructions are deleted in 100 
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

- First column F\.: meaning Format.

.. list-table:: Cpu0 Instruction Set
	:widths: 1 4 3 11 7 10
	:header-rows: 1


  * - F\.
    - Mnemonic
    - Opcode
    - Meaning
    - Syntax
    - Operation
  * - L
    - NOP
    - 00
    - No Operation
    - 
    - 
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
    - LB
    - 03
    - Load byte
    - LB Ra, [Rb+Cx]
    - Ra <= (byte)[Rb+Cx]
  * - L
    - LBu
    - 04
    - Load byte unsigned
    - LBu Ra, [Rb+Cx]
    - Ra <= (byte)[Rb+Cx]
  * - L
    - SB
    - 05
    - Store byte
    - SB Ra, [Rb+Cx]
    - [Rb+Cx] <= (byte)Ra
  * - A
    - LH
    - 06
    - Load half word unsigned
    - LH Ra, [Rb+Cx]
    - Ra <= (2bytes)[Rb+Cx]
  * - A
    - LHu
    - 07
    - Load half word
    - LHu Ra, [Rb+Cx]
    - Ra <= (2bytes)[Rb+Cx]
  * - A
    - SH
    - 08
    - Store half word
    - SH Ra, [Rb+Cx]
    - [Rb+Rc] <= Ra
  * - L
    - ADDiu
    - 09
    - Add immediate
    - ADDiu Ra, Rb, Cx
    - Ra <= (Rb + Cx)
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
    - Ra <= (Rb \^ Cx)
  * - L
    - LUi
    - 0F
    - Load upper
    - LUi Ra, Cx
    - Ra <= (Cx << 16)
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
    - 17
    - Multiply
    - MUL Ra, Rb, Rc
    - Ra <= Rb * Rc
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
    - 1B
    - Rotate left
    - ROL Ra, Rb, Cx
    - Ra <= Rb rol Cx
  * - A
    - ROR
    - 1C
    - Rotate right
    - ROR Ra, Rb, Cx
    - Ra <= Rb ror Cx
  * - A
    - SRA
    - 1D
    - Shift right
    - SRA Ra, Rb, Cx
    - Ra <= Rb '>> Cx [#sra-note]_
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
    - SRAV
    - 20
    - Shift right
    - SRAV Ra, Rb, Rc
    - Ra <= Rb '>> Rc [#sra-note]_
  * - A
    - SHLV
    - 21
    - Shift left
    - SHLV Ra, Rb, Rc
    - Ra <= Rb << Rc
  * - A
    - SHRV
    - 22
    - Shift right
    - SHRV Ra, Rb, Rc
    - Ra <= Rb >> Rc
  * - L
    - BEQ
    - 30
    - Jump if equal
    - BEQ Ra, Rb, Cx
    - if (Ra==Rb), PC <= PC + Cx
  * - L
    - BNE
    - 31
    - Jump if not equal
    - BNE Ra, Rb, Cx
    - if (Ra!=Rb), PC <= PC + Cx
  * - J
    - JMP
    - 36
    - Jump (unconditional)
    - JMP Cx
    - PC <= PC + Cx
  * - J
    - SWI
    - 3A
    - Software interrupt
    - SWI Cx
    - LR <= PC; PC <= Cx
  * - J
    - JSUB
    - 3B
    - Jump to subroutine
    - JSUB Cx
    - LR <= PC; PC <= PC + Cx
  * - J
    - RET
    - 3C
    - Return from subroutine
    - RET LR
    - PC <= LR
  * - J
    - IRET
    - 3D
    - Return from interrupt handler
    - IRET
    - PC <= LR; INT 0
  * - J
    - JALR
    - 3E
    - Jump to subroutine
    - JR Rb
    - LR <= PC; PC <= Rb
  * - L
    - SLTi
    - 26
    - Set less Then
    - SLTi Ra, Rb, Cx
    - Ra <= (Rb < Cx)
  * - L
    - SLTiu
    - 27
    - SLTi unsigned 
    - SLTiu Ra, Rb, Cx
    - Ra <= (Rb < Cx)
  * - A
    - SLT
    - 28
    - Set less Then
    - SLT Ra, Rb, Rc
    - Ra <= (Rb < Rc)
  * - A
    - SLTu
    - 29
    - SLT unsigned
    - SLTu Ra, Rb, Rc
    - Ra <= (Rb < Rc)
  * - L
    - MULT
    - 41
    - Multiply for 64 bits result
    - MULT Ra, Rb
    - (HI,LO) <= MULT(Ra,Rb)
  * - L
    - MULTU
    - 42
    - MULT for unsigned 64 bits
    - MULTU Ra, Rb
    - (HI,LO) <= MULTU(Ra,Rb)
  * - L
    - DIV
    - 43
    - Divide
    - DIV Ra, Rb
    - HI<=Ra%Rb, LO<=Ra/Rb
  * - L
    - DIVU
    - 44
    - Divide
    - DIV Ra, Rb
    - HI<=Ra%Rb, LO<=Ra/Rb
  * - L
    - MFHI
    - 46
    - Move HI to GPR
    - MFHI Ra
    - Ra <= HI
  * - L
    - MFLO
    - 47
    - Move LO to GPR
    - MFLO Ra
    - Ra <= LO
  * - L
    - MTHI
    - 48
    - Move GPR to HI
    - MTHI Ra
    - HI <= Ra
  * - L
    - MTLO
    - 49
    - Move GPR to LO
    - MTLO Ra
    - LO <= Ra


As above, the OPu, such as ADDu is for unsigned integer or No Trigger 
Exception. The LUi for example, "LUi $2, 0x7000", load 0x700 to high 16 bits 
of $2 and fill the low 16 bits of $2 to 0x0000. 


Cpu0 code changes
~~~~~~~~~~~~~~~~~

Chapter12_2/ include the changes for new instruction sets as follows,

.. rubric:: LLVMBackendTutorialExampleCode/Chapter12_2/AsmParser/Cpu0AsmParser.cpp
.. code-block:: c++

  // Cpu0AsmParser.cpp
  ...
  int Cpu0AsmParser::matchRegisterName(StringRef Name) {
    ...
        .Case("t0",  Cpu0::T0)
    ...
  }
  
.. rubric:: LLVMBackendTutorialExampleCode/Chapter12_2/Disassembler/Cpu0Disassembler.cpp
.. code-block:: c++

  // Decoder tables for Cpu0 register
  static const unsigned CPURegsTable[] = {
  // Change SW to T0 which is a caller saved
    Cpu0::T0, ...
  };
  
  // DecodeCMPInstruction() function is removed since No CMP instruction.
  ...
  
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
  
.. rubric:: LLVMBackendTutorialExampleCode/Chapter12_2/MCTargetDesc/Cpu0AsmBackend.cpp
.. code-block:: c++

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
  
.. rubric:: LLVMBackendTutorialExampleCode/Chapter12_2/MCTargetDesc/Cpu0BaseInfo.cpp
.. code-block:: c++

  inline static unsigned getCpu0RegisterNumbering(unsigned RegEnum)
  {
    switch (RegEnum) {
    ...
    case Cpu0::T0:
    ...
    }
  }
  
.. rubric:: LLVMBackendTutorialExampleCode/Chapter12_2/MCTargetDesc/Cpu0FixupKinds.cpp
.. code-block:: c++

    enum Fixups {
      ...
      // PC relative branch fixup resulting in - R_CPU0_PC16.
      // cpu0 PC16, e.g. beq
      fixup_Cpu0_PC16,
      ...
    };
  
.. rubric:: LLVMBackendTutorialExampleCode/Chapter12_2/MCTargetDesc/Cpu0MCCodeEmitter.cpp
.. code-block:: c++

  unsigned Cpu0MCCodeEmitter::
  getBranchTargetOpValue(const MCInst &MI, unsigned OpNo,
                         SmallVectorImpl<MCFixup> &Fixups) const {
    ...
    Fixups.push_back(MCFixup::Create(0, Expr,
                                     MCFixupKind(Cpu0::fixup_Cpu0_PC16)));
    return 0;
  }
  ...
  unsigned Cpu0MCCodeEmitter::
  getJumpTargetOpValue(const MCInst &MI, unsigned OpNo,
                       SmallVectorImpl<MCFixup> &Fixups) const {
    ...
    if (Opcode == Cpu0::JSUB || Opcode == Cpu0::JMP)
    ...
  }

.. rubric:: LLVMBackendTutorialExampleCode/Chapter12_2/Cpu0InstrInfo.cpp
.. code-block:: c++

  // Cpu0InstrInfo::copyPhysReg()
  void Cpu0InstrInfo::
  copyPhysReg(MachineBasicBlock &MBB,
              MachineBasicBlock::iterator I, DebugLoc DL,
              unsigned DestReg, unsigned SrcReg,
              bool KillSrc) const {
    unsigned Opc = 0, ZeroReg = 0;
  
    if (Cpu0::CPURegsRegClass.contains(DestReg)) { // Copy to CPU Reg.
      if (Cpu0::CPURegsRegClass.contains(SrcReg))
        Opc = Cpu0::ADD, ZeroReg = Cpu0::ZERO;
      else if (SrcReg == Cpu0::HI)
        Opc = Cpu0::MFHI, SrcReg = 0;
      else if (SrcReg == Cpu0::LO)
        Opc = Cpu0::MFLO, SrcReg = 0;
    }
    else if (Cpu0::CPURegsRegClass.contains(SrcReg)) { // Copy from CPU Reg.
      if (DestReg == Cpu0::HI)
        Opc = Cpu0::MTHI, DestReg = 0;
      else if (DestReg == Cpu0::LO)
        Opc = Cpu0::MTLO, DestReg = 0;
    }
  
    assert(Opc && "Cannot copy registers");
  
    MachineInstrBuilder MIB = BuildMI(MBB, I, DL, get(Opc));
  
    if (DestReg)
      MIB.addReg(DestReg, RegState::Define);
  
    if (ZeroReg)
      MIB.addReg(ZeroReg);
  
    if (SrcReg)
      MIB.addReg(SrcReg, getKillRegState(KillSrc));
  }

.. rubric:: LLVMBackendTutorialExampleCode/Chapter12_2/Cpu0InstrInfo.td
.. code-block:: c++

  def jmptarget    : Operand<OtherVT> {
    let EncoderMethod = "getJumpTargetOpValue";
    let OperandType = "OPERAND_PCREL";
    let DecoderMethod = "DecodeJumpRelativeTarget";
  }
  ...
  // Immediate can be loaded with LUi (32-bit int with lower 16-bit cleared).
  def immLow16Zero : PatLeaf<(imm), [{
    int64_t Val = N->getSExtValue();
    return isInt<32>(Val) && !(Val & 0xffff);
  }]>;
  ...
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
  ...
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
  // Unconditional branch, such as JMP
  class UncondBranch<bits<8> op, string instr_asm>:
    FJ<op, (outs), (ins jmptarget:$addr),
               !strconcat(instr_asm, "\t$addr"), [(br bb:$addr)], IIBranch> {
    let isBranch = 1;
    let isTerminator = 1;
    let isBarrier = 1;
    let hasDelaySlot = 0;
  }
  ...
  def SLTi    : SetCC_I<0x26, "slti", setlt, simm16, immSExt16, CPURegs>;
  def SLTiu   : SetCC_I<0x27, "sltiu", setult, simm16, immSExt16, CPURegs>;
  def SLT     : SetCC_R<0x28, "slt", setlt, CPURegs>;
  def SLTu    : SetCC_R<0x29, "sltu", setult, CPURegs>;
  
  /// Jump and Branch Instructions
  def BEQ     : CBranch<0x30, "beq", seteq, CPURegs>;
  def BNE     : CBranch<0x31, "bne", setne, CPURegs>;
  
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
  // a == b
    def : Pat<(seteq RC:$lhs, RC:$rhs),
                  (SLTiuOp (XOROp RC:$lhs, RC:$rhs), 1)>;
  // a != b
    def : Pat<(setne RC:$lhs, RC:$rhs),
                  (SLTuOp ZEROReg, (XOROp RC:$lhs, RC:$rhs))>;
  }
  
  // a <= b
  multiclass SetlePats<RegisterClass RC, Instruction SLTOp, Instruction SLTuOp> {
    def : Pat<(setle RC:$lhs, RC:$rhs),
  // a <= b is equal to (XORi (b < a), 1)
                  (XORi (SLTOp RC:$rhs, RC:$lhs), 1)>;
    def : Pat<(setule RC:$lhs, RC:$rhs),
                  (XORi (SLTuOp RC:$rhs, RC:$lhs), 1)>;
  }
  
  // a > b
  multiclass SetgtPats<RegisterClass RC, Instruction SLTOp, Instruction SLTuOp> {
    def : Pat<(setgt RC:$lhs, RC:$rhs),
  // a > b is equal to b < a is equal to setlt(b, a)
                  (SLTOp RC:$rhs, RC:$lhs)>;
    def : Pat<(setugt RC:$lhs, RC:$rhs),
                  (SLTuOp RC:$rhs, RC:$lhs)>;
  }
  
  // a >= b
  multiclass SetgePats<RegisterClass RC, Instruction SLTOp, Instruction SLTuOp> {
    def : Pat<(setge RC:$lhs, RC:$rhs),
  // a >= b is equal to b <= a
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


.. rubric:: LLVMBackendTutorialExampleCode/Chapter12_2/Cpu0ISelDAGToDAG.cpp
.. code-block:: c++

  /// Select instructions not customized! Used for
  /// expanded, promoted and normal instructions
  SDNode* Cpu0DAGToDAGISel::Select(SDNode *Node) {
    ...
    case ISD::SUBE:
    case ISD::ADDE: {
      ...
      SDNode *StatusWord = CurDAG->getMachineNode(Cpu0::CMP, dl, VT, Ops);
      SDValue Constant1 = CurDAG->getTargetConstant(1, VT);
      SDNode *Carry = CurDAG->getMachineNode(Cpu0::ANDi, dl, VT, 
                                             SDValue(StatusWord,0), Constant1);
      ...
    ...
  }

.. rubric:: LLVMBackendTutorialExampleCode/Chapter12_2/Cpu0RegisterInfo.td
.. code-block:: c++

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
related code from Chapter12_1/, and change from JEQ 24bits offset to BEQ 16 bits 
offset.


Cpu0 Verilog language changes
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. rubric:: LLVMBackendTutorialExampleCode/cpu0_verilog/cpu0IIs.v
.. literalinclude:: ../LLVMBackendTutorialExampleCode/cpu0_verilog/cpu0IIs.v

In addition to cpu0IIs.v, the "`ifdef CPU0II" in cpu0.v is added for extended
slt, beq, bne instructions.


Run the Cpu0II
~~~~~~~~~~~~~~~~

Run Chapter12_2/ with ch_run_backend.cpp to get result as below. 
It match the expect value as comment in ch_run_backend.cpp.

.. rubric:: LLVMBackendTutorialExampleCode/InputFiles/ch_run_backend.cpp
.. literalinclude:: ../LLVMBackendTutorialExampleCode/InputFiles/ch_run_backend.cpp
    :start-after: /// start

.. code-block:: bash

  118-165-77-203:InputFiles Jonathan$ /Users/Jonathan/llvm/test/cmake_debug_build/
  bin/Debug/llc -march=cpu0 -relocation-model=static -filetype=obj -stats 
  ch_run_backend.bc -o ch_run_backend.cpu0.o
  ===-------------------------------------------------------------------------===
                            ... Statistics Collected ...
  ===-------------------------------------------------------------------------===
    ...
     5 del-jmp     - Number of useless jmp deleted
    ...
  
  118-165-77-203:InputFiles Jonathan$ /Users/Jonathan/llvm/test/cmake_debug_build/
  bin/Debug/llvm-objdump -d ch_run_backend.cpu0.o | tail -n +6| awk '{print "/* " $1 
  " */\t" $2 " " $3 " " $4 " " $5 "\t/* " $6"\t" $7" " $8" " $9" " $10 "\t*/"}' > 
  ../cpu0_verilog/redesign/cpu0.hex
  
  JonathantekiiMac:InputFiles Jonathan$ cd ../cpu0_verilog/
  JonathantekiiMac:redesign Jonathan$ iverilog -o cpu0IIs cpu0IIs.v 
  JonathantekiiMac:redesign Jonathan$ ./cpu0IIs
  taskInterrupt(001)
  74
  253
  3
  1
  14
  3
  393307
  16777222
  51
  2147483647
  -2147483648
  15
  RET to PC < 0, finished!

Run with ch8_1_1.cpp, it reduce some branch from pair instructions "CMP, JXX" 
to 1 single instruction ether is BEQ or BNE, as follows,

.. code-block:: bash

  118-165-77-203:InputFiles Jonathan$ /Users/Jonathan/llvm/test/cmake_debug_build/
  bin/Debug/llc -march=cpu0 -relocation-model=static -filetype=asm ch8_1_1.bc -o 
  ch8_1_1.cpu0.s
  118-165-77-203:InputFiles Jonathan$ cat ch8_1_1.cpu0.s 
	  .section .mdebug.abi32
	  .previous
	  .file	"ch8_1_1.bc"
	  .text
	  .globl	_Z13test_control1v
	  .align	2
	  .type	_Z13test_control1v,@function
	  .ent	_Z13test_control1v      # @_Z13test_control1v
  _Z13test_control1v:
	  .cfi_startproc
	  .frame	$fp,48,$lr
	  .mask 	0x00000800,-4
	  .set	noreorder
	  .set	nomacro
  # BB#0:                                 # %entry
	  addiu	$sp, $sp, -48
  $tmp3:
	  .cfi_def_cfa_offset 48
	  st	$fp, 44($sp)            # 4-byte Folded Spill
  $tmp4:
	  .cfi_offset 11, -4
	  addu	$fp, $sp, $zero
  $tmp5:
	  .cfi_def_cfa_register 11
	  addiu	$3, $zero, 0
	  st	$3, 40($fp)
	  addiu	$2, $zero, 1
	  st	$2, 36($fp)
	  addiu	$4, $zero, 2
	  st	$4, 32($fp)
	  addiu	$4, $zero, 3
	  st	$4, 28($fp)
	  addiu	$4, $zero, 4
	  st	$4, 24($fp)
	  addiu	$4, $zero, 5
	  st	$4, 20($fp)
	  addiu	$4, $zero, 6
	  st	$4, 16($fp)
	  addiu	$4, $zero, 7
	  st	$4, 12($fp)
	  addiu	$4, $zero, 8
	  st	$4, 8($fp)
	  addiu	$4, $zero, 9
	  st	$4, 4($fp)
	  ld	$4, 40($fp)
	  bne	$4, $zero, $BB0_2
  # BB#1:                                 # %if.then
	  ld	$4, 40($fp)
	  addiu	$4, $4, 1
	  st	$4, 40($fp)
  $BB0_2:                                 # %if.end
	  ld	$4, 36($fp)
	  beq	$4, $zero, $BB0_4
  # BB#3:                                 # %if.then2
	  ld	$4, 36($fp)
	  addiu	$4, $4, 1
	  st	$4, 36($fp)
  $BB0_4:                                 # %if.end4
	  ld	$4, 32($fp)
	  slti	$4, $4, 1
	  bne	$4, $zero, $BB0_6
  # BB#5:                                 # %if.then6
	  ld	$4, 32($fp)
	  addiu	$4, $4, 1
	  st	$4, 32($fp)
  $BB0_6:                                 # %if.end8
	  ld	$4, 28($fp)
	  slti	$4, $4, 0
	  bne	$4, $zero, $BB0_8
  # BB#7:                                 # %if.then10
	  ld	$4, 28($fp)
	  addiu	$4, $4, 1
	  st	$4, 28($fp)
  $BB0_8:                                 # %if.end12
	  ld	$4, 24($fp)
	  addiu	$5, $zero, -1
	  slt	$4, $5, $4
	  bne	$4, $zero, $BB0_10
  # BB#9:                                 # %if.then14
	  ld	$4, 24($fp)
	  addiu	$4, $4, 1
	  st	$4, 24($fp)
  $BB0_10:                                # %if.end16
	  ld	$4, 20($fp)
	  slt	$3, $3, $4
	  bne	$3, $zero, $BB0_12
  # BB#11:                                # %if.then18
	  ld	$3, 20($fp)
	  addiu	$3, $3, 1
	  st	$3, 20($fp)
  $BB0_12:                                # %if.end20
	  ld	$3, 16($fp)
	  slt	$2, $2, $3
	  bne	$2, $zero, $BB0_14
  # BB#13:                                # %if.then22
	  ld	$2, 16($fp)
	  addiu	$2, $2, 1
	  st	$2, 16($fp)
  $BB0_14:                                # %if.end24
	  ld	$2, 12($fp)
	  slti	$2, $2, 1
	  bne	$2, $zero, $BB0_16
  # BB#15:                                # %if.then26
	  ld	$2, 12($fp)
	  addiu	$2, $2, 1
	  st	$2, 12($fp)
  $BB0_16:                                # %if.end28
	  ld	$2, 12($fp)
	  ld	$3, 8($fp)
	  slt	$2, $3, $2
	  beq	$2, $zero, $BB0_18
  # BB#17:                                # %if.then30
	  ld	$2, 8($fp)
	  addiu	$2, $2, 1
	  st	$2, 8($fp)
  $BB0_18:                                # %if.end32
	  ld	$2, 36($fp)
	  ld	$3, 40($fp)
	  beq	$3, $2, $BB0_20
  # BB#19:                                # %if.then34
	  ld	$2, 4($fp)
	  addiu	$2, $2, 1
	  st	$2, 4($fp)
  $BB0_20:                                # %if.end36
	  ld	$2, 36($fp)
	  ld	$3, 40($fp)
	  addu	$2, $3, $2
	  ld	$3, 32($fp)
	  addu	$2, $2, $3
	  ld	$3, 28($fp)
	  addu	$2, $2, $3
	  ld	$3, 24($fp)
	  addu	$2, $2, $3
	  ld	$3, 20($fp)
	  addu	$2, $2, $3
	  ld	$3, 16($fp)
	  addu	$2, $2, $3
	  ld	$3, 12($fp)
	  addu	$2, $2, $3
	  ld	$3, 8($fp)
	  addu	$2, $2, $3
	  ld	$3, 4($fp)
	  addu	$2, $2, $3
	  addu	$sp, $fp, $zero
	  ld	$fp, 44($sp)            # 4-byte Folded Reload
	  addiu	$sp, $sp, 48
	  ret	$lr
	  .set	macro
	  .set	reorder
	  .end	_Z13test_control1v
  $tmp6:
	  .size	_Z13test_control1v, ($tmp6)-_Z13test_control1v
	  .cfi_endproc


The ch12_3.cpp is written in assembly for AsmParser test. You can check if it 
will generate the obj.

.. [#sra-note] Rb '>> Cx, Rb '>> Rc: Shift with signed bit remain. It's equal to ((Rb&'h80000000)|Rb>>Cx) or ((Rb&'h80000000)|Rb>>Rc).

