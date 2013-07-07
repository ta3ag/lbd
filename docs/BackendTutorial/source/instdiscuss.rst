.. _sec-appendix-inst-discuss:

Appendix C: instructions discuss
=================================

This chapter discuss other backend instructions.

Implicit operand
-----------------

LLVM IR is a 3 address form (4 tuple <opcode, %1, %2, %3) which match the 
current RISC cpu0 (like Mips). 
So, it seems no "move" IR DAG. 
Because "move a, b" can be replaced by "lw a, b_offset($sp)" for local 
variable, or can be replaced by "addu $a, $0,$ b". 
The cpu0 is same as Mips. 
Base on this reason, the move instruction is useless even though it supplied by 
the cpu0 author.

For the old CPU or Micro Processor (MCU), like PIC, 8051 and old intel processor. 
These CPU/MCU need memory saving and not aim to high level of program (such as 
C) only (they aim to assembly code program too). 
These CPU/MCU need implicit operand, maybe use ACC (accumulate register). 

It will translate,

.. code-block:: c++

  c = a + b + d; 
  
into,

.. code-block:: c++

	mtacc   Addr(12) // Move b To Acc
	add     Addr(16) // Add a To Acc
	add     Addr(4)  // Add d To Acc
	mfacc   Addr(8)  // Move Acc To c

Above code also can be coded by programmer who use assembly language directly 
in MCU or BIOS programm since maybe the code size is just 4KB or less.

Since cpu0 is a 32 bits (code size can be 4GB), it use Store and Load 
instructions for memory address access only. 
Other instructions (include add), use register to register style operation.
We change the implicit operand support in this section. 
It's just a demonstration with this design, not fully support. 
The purpose is telling reader how to implement this style of CPU/MCU backend. 
Run Chapter8_4_2/ with ch_move.cpp will get the following result,

.. rubric:: LLVMBackendTutorialExampleCode/InputFiles/ch_move.cpp
.. literalinclude:: ../LLVMBackendTutorialExampleCode/InputFiles/ch_move.cpp
    :lines: 5-
    :linenos:

.. code-block:: bash

  ld  $3, 12($sp) // $3 is a
  ld  $4, 16($sp) // $4 is b
  mtacc $4        // Move b To Acc
  add $3          // Add a To Acc
  ld  $4, 4($sp)  // $4 is d
  add $4          // Add d To Acc
  mfacc $3        // Move Acc to $3
  addiu $3, $3, 5 // Add e(=5) to $3
  st  $3, 8($sp)


To support this implicit operand, ACC. 
The following code is added to Chapter8_4_2.cpp.

.. rubric:: LLVMBackendTutorialExampleCode/Chapter8_4_2/Cpu0RegisterInfo.td 
.. code-block:: c++

  let Namespace = "Cpu0" in {
    // General Purpose Registers
    def ZERO : Cpu0GPRReg< 0, "ZERO">, DwarfRegNum<[0]>;
    ...
    def ACC : Register<"acc">, DwarfRegNum<[20]>;
  }
  ...
  def RACC : RegisterClass<"Cpu0", [i32], 32, (add ACC)>;
  
  
.. rubric:: LLVMBackendTutorialExampleCode/Chapter8_4_2/Cpu0InstrInfo.td 
.. code-block:: c++

  class MoveFromACC<bits<8> op, string instr_asm, RegisterClass RC,
             list<Register> UseRegs>:
    FL<op, (outs RC:$ra), (ins),
     !strconcat(instr_asm, "\t$ra"), [], IIAlu> {
    let rb = 0;
    let imm16 = 0;
    let Uses = UseRegs;
    let neverHasSideEffects = 1;
  }
  
  class MoveToACC<bits<8> op, string instr_asm, RegisterClass RC,
           list<Register> DefRegs>:
    FL<op, (outs), (ins RC:$ra),
     !strconcat(instr_asm, "\t$ra"), [], IIAlu> {
    let rb = 0;
    let imm16 = 0;
    let Defs = DefRegs;
    let neverHasSideEffects = 1;
  }
  
  class ArithLogicUniR2<bits<8> op, string instr_asm, RegisterClass RC1,
           RegisterClass RC2, list<Register> DefRegs>:
    FL<op, (outs), (ins RC1:$accum, RC2:$ra),
     !strconcat(instr_asm, "\t$ra"), [], IIAlu> {
    let rb = 0;
    let imm16 = 0;
    let Defs = DefRegs;
    let neverHasSideEffects = 1;
  }
  ...
  //def ADD     : ArithLogicR<0x13, "add", add, IIAlu, CPURegs, 1>;
  ...
  def MFACC : MoveFromACC<0x44, "mfacc", CPURegs, [ACC]>;
  def MTACC : MoveToACC<0x45, "mtacc", CPURegs, [ACC]>;
  def ADD   : ArithLogicUniR2<0x46, "add", RACC, CPURegs, [ACC]>;
  ...
  def : Pat<(add RACC:$lhs, CPURegs:$rhs),
        (ADD RACC:$lhs, CPURegs:$rhs)>;
  
  def : Pat<(add CPURegs:$lhs, CPURegs:$rhs),
        (ADD (MTACC CPURegs:$lhs), CPURegs:$rhs)>;
  
  
.. rubric:: LLVMBackendTutorialExampleCode/Chapter8_4_2/Cpu0InstrInfo.cpp
.. code-block:: c++

  //- Called when DestReg and SrcReg belong to different Register Class.
  void Cpu0InstrInfo::
  copyPhysReg(MachineBasicBlock &MBB,
        MachineBasicBlock::iterator I, DebugLoc DL,
        unsigned DestReg, unsigned SrcReg,
        bool KillSrc) const {
    unsigned Opc = 0, ZeroReg = 0;
  
    if (Cpu0::CPURegsRegClass.contains(DestReg)) { // Copy to CPU Reg.
    ...
    else if (SrcReg == Cpu0::ACC)
      Opc = Cpu0::MFACC, SrcReg = 0;
    }
    else if (Cpu0::CPURegsRegClass.contains(SrcReg)) { // Copy from CPU Reg.
    ...
    else if (DestReg == Cpu0::ACC)
      Opc = Cpu0::MTACC, DestReg = 0;
    }
    ...
  }

  
Explain the code as follows,

.. code-block:: bash

  ld  $3, 12($sp) // $3 is a
  ld  $4, 16($sp) // $4 is b
  
  mtacc $4      // Move b To Acc
  // After meet first a+b IR, it call this pattern,
  //  def : Pat<(add CPURegs:$lhs, CPURegs:$rhs),
  //        (ADD (MTACC CPURegs:$lhs), CPURegs:$rhs)>;
  // After this pattern translation, the DestReg class change from CPU0Regs to 
  //  RACC according the following code of copyPhysReg(). copyPhysReg() is called 
  //  when DestReg and SrcReg belong to different Register Class.
  //
  //  if (DestReg)
  //    MIB.addReg(DestReg, RegState::Define);
  //
  //  if (ZeroReg)
  //    MIB.addReg(ZeroReg);
  //
  //  if (SrcReg)
  //    MIB.addReg(SrcReg, getKillRegState(KillSrc));

  add $3      // Add a To Acc
  // Apply this pattern since the DestReg class is RACC
  //  def : Pat<(add RACC:$lhs, CPURegs:$rhs),
  //        (ADD RACC:$lhs, CPURegs:$rhs)>;

  ld  $4, 4($sp)  // $4 is d
  add $4      // Add d To Acc
  // Apply the pattern as above since the DestReg class is RACC

  mfacc $3    // Move Acc to $3
  // Compiler/backend can use ADDiu since e is 5. But it add MFACC before ADDiu 
  //  since the DestReg class is RACC. Translate to CPU0Regs class by MFACC and 
  //  apply ADDiu since ADDiu use CPU0Regs as operands.
  addiu $3, $3, 5 // Add e(=5) to $3
  st  $3, 8($sp)




.. _section CPU0 processor architecture:
    http://jonathan2251.github.com/lbd/llvmstructure.html#cpu0-processor-
    architecture
