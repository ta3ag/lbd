.. _sec-optimize:

Backend Optimization
====================

This chapter introduce how to do backend optimization in LLVM first. 
Next we do optimization via extend instruction sets in hardware level to 
do optimization in a way of creating an efficient RISC CPU which aims to C/C++ 
high level language.

Cpu0 backend Optimization: Remove useless JMP
---------------------------------------------

LLVM uses functional pass both in code generation and optimization. 
Following the 3 tiers of compiler architecture, LLVM did much optimization in 
middle tier of LLVM IR, SSA form. 
Beyond middle tier optimization, there are opportunities in 
optimization which depend on backend features. 
Mips fill delay slot is an example of backend optimization used in pipeline 
RISC machine.
You can migrate from Mips if your backend is a pipeline RISC with 
delay slot. 
In this section, we apply the "delete useless jmp" in Cpu0 
backend optimization. 
This algorithm is simple and effective as a perfect tutorial in optimization. 
Through this example, you can understand how to add an optimization pass and 
coding your complicate optimization algorithm on your backend in real project.

Chapter12_1/ supports "delete useless jmp" optimization algorithm which add 
codes as follows,

.. rubric:: lbdex/Chapter12_1/CMakeLists.txt
.. code-block:: c++

  add_llvm_target(Cpu0CodeGen
    ...
    Cpu0DelUselessJMP.cpp
    ...
    )
  
.. rubric:: lbdex/Chapter12_1/Cpu0.h
.. code-block:: c++

  ...
    FunctionPass *createCpu0DelJmpPass(Cpu0TargetMachine &TM);
  
.. rubric:: lbdex/Chapter12_1/Cpu0TargetMachine.cpp
.. code-block:: c++

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

.. rubric:: lbdex/Chapter12_1/Cpu0DelUselessJMP.cpp
.. literalinclude:: ../../../lib/Target/Cpu0/Cpu0DelUselessJMP.cpp


As above code, except Cpu0DelUselessJMP.cpp, other files changed for register 
class DelJmp as a functional pass. 
As the comment of above code, MBB is the current 
block and MBBN is the next block. For each last instruction of every MBB, we 
check if it is the JMP instruction as well as 
its Operand is the next basic block. 
By getMBB() in MachineOperand, you can get the MBB address. 
For the member functions of MachineOperand, please check 
include/llvm/CodeGen/MachineOperand.h
Now, let's run Chapter12_1/ with ch12_1.cpp for explanation.

.. rubric:: lbdex/InputFiles/ch12_1.cpp
.. literalinclude:: ../lbdex/InputFiles/ch12_1.cpp
    :start-after: /// start

.. code-block:: bash

  118-165-78-10:InputFiles Jonathan$ clang -target mips-unknown-linux-gnu 
  -c ch12_1.cpp -emit-llvm -o ch12_1.bc
  118-165-78-10:InputFiles Jonathan$ /Users/Jonathan/llvm/test/cmake_debug_build/
  bin/Debug/llc -march=cpu0 -relocation-model=static -filetype=asm -stats 
  ch12_1.bc -o -
	  .section .mdebug.abi32
	  .previous
	  .file	"ch12_1.bc"
	  .text
	  .globl	_Z18test_DelUselessJMPiii
	  .align	2
	  .type	_Z18test_DelUselessJMPiii,@function
	  .ent	_Z18test_DelUselessJMPiii # @_Z18test_DelUselessJMPiii
  _Z18test_DelUselessJMPiii:
	  .frame	$fp,16,$lr
	  .mask 	0x00001000,-4
	  .set	noreorder
	  .set	nomacro
  # BB#0:
	  addiu	$sp, $sp, -16
	  st	$fp, 12($sp)            # 4-byte Folded Spill
	  addu	$fp, $sp, $zero
	  ld	$2, 16($fp)
	  st	$2, 8($fp)
	  ld	$2, 20($fp)
	  st	$2, 4($fp)
	  ld	$2, 24($fp)
	  st	$2, 0($fp)
	  addiu	$2, $zero, 0
	  ld	$3, 8($fp)
	  cmp	$sw, $3, $2
	  jne	$sw, $BB0_2
  # BB#1:
	  ld	$3, 8($fp)
	  addiu	$3, $3, 1
	  st	$3, 8($fp)
  $BB0_2:
	  ld	$3, 4($fp)
	  cmp	$sw, $3, $2
	  jne	$sw, $BB0_4
	  jmp	$BB0_3
  $BB0_4:
	  addiu	$2, $zero, -1
	  ld	$3, 4($fp)
	  cmp	$sw, $3, $2
	  jgt	$sw, $BB0_6
	  jmp	$BB0_5
  $BB0_3:
	  ld	$2, 8($fp)
	  addiu	$2, $2, 3
	  st	$2, 8($fp)
	  ld	$2, 4($fp)
	  addiu	$2, $2, 1
	  st	$2, 4($fp)
	  jmp	$BB0_6
  $BB0_5:
	  ld	$2, 4($fp)
	  ld	$3, 8($fp)
	  addu	$2, $3, $2
	  st	$2, 8($fp)
	  ld	$2, 4($fp)
	  addiu	$2, $2, -1
	  st	$2, 4($fp)
  $BB0_6:
	  addiu	$2, $zero, 1
	  ld	$3, 0($fp)
	  cmp	$sw, $3, $2
	  jlt	$sw, $BB0_8
  # BB#7:
	  ld	$2, 0($fp)
	  ld	$3, 8($fp)
	  addu	$2, $3, $2
	  st	$2, 8($fp)
	  ld	$2, 0($fp)
	  addiu	$2, $2, 1
	  st	$2, 0($fp)
  $BB0_8:
	  ld	$2, 8($fp)
	  addu	$sp, $fp, $zero
	  ld	$fp, 12($sp)            # 4-byte Folded Reload
	  addiu	$sp, $sp, 16
	  ret	$lr
	  .set	macro
	  .set	reorder
	  .end	_Z18test_DelUselessJMPiii
  $tmp3:
	  .size	_Z18test_DelUselessJMPiii, ($tmp3)-_Z18test_DelUselessJMPiii
  ...
  ===-------------------------------------------------------------------------===
                            ... Statistics Collected ...
  ===-------------------------------------------------------------------------===
   ...
   2 del-jmp        - Number of useless jmp deleted
   ...

The terminal display "Number of useless jmp deleted" by ``llc -stats`` option 
because we set the "STATISTIC(NumDelJmp, "Number of useless jmp deleted")" in 
code. It deletes 2 jmp instructions from block "# BB#0" and "$BB0_6".
You can check it by ``llc -enable-cpu0-del-useless-jmp=false`` option to see 
the difference to non-optimization version.
If you run with ch8_1_1.cpp, will find 10 jmp instructions are deleted in 100 
lines of assembly code, which meaning 10\% improvement in speed and code size 
[#]_.


Cpu0 Optimization: Extends instruction sets
---------------------------------------------

If you compare the cpu0 and Mips instruction sets, you will find that Mips use 
SLT, BEQ and set the status in explicit/general register while Cpu0 use CMP, 
JEQ and set status in implicit/specific register.

Follow RISC principle, this section will replace CMP, JEQ with Mips style 
instructions.
Mips style BEQ instructions will reduce the number of branch instructions too. 
Which means optimization in speed and code size.

Cpu0 new instruction sets table
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Add Cpu0 instructions as follows,

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
    - BEQ
    - 37
    - Jump if equal
    - BEQ Ra, Rb, Cx
    - if (Ra==Rb), PC <= PC + Cx
  * - L
    - BNE
    - 38
    - Jump if not equal
    - BNE Ra, Rb, Cx
    - if (Ra!=Rb), PC <= PC + Cx



Cpu0 code changes
~~~~~~~~~~~~~~~~~

Chapter12_2/ include the changes for new instruction sets as follows,

.. rubric:: lbdex/Chapter12_2/Disassembler/Cpu0Disassembler.cpp
.. code-block:: c++

  static DecodeStatus DecodeBranch16Target(MCInst &Inst,
                                         unsigned Insn,
                                         uint64_t Address,
                                         const void *Decoder);
  ...            
  static DecodeStatus DecodeBranch16Target(MCInst &Inst,
                                         unsigned Insn,
                                         uint64_t Address,
                                         const void *Decoder) {
    int BranchOffset = fieldFromInstruction(Insn, 0, 16);
    if (BranchOffset > 0x8fff)
      BranchOffset = -1*(0x10000 - BranchOffset);
    Inst.addOperand(MCOperand::CreateImm(BranchOffset));
    return MCDisassembler::Success;
  }
  
.. rubric:: lbdex/Chapter12_2/MCTargetDesc/Cpu0AsmBackend.cpp
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
  
.. rubric:: lbdex/Chapter12_2/MCTargetDesc/Cpu0ELFObjectWriter.cpp
.. code-block:: c++

  unsigned Cpu0ELFObjectWriter::GetRelocType(const MCValue &Target,
                                             const MCFixup &Fixup,
                                             bool IsPCRel,
                                             bool IsRelocWithSymbol,
                                             int64_t Addend) const {
    ...
    switch (Kind) {
      ...
      case Cpu0::fixup_Cpu0_PC16:
      Type = ELF::R_CPU0_PC16;
      break;
      ...
    }
    ...
  }

.. rubric:: lbdex/Chapter12_2/MCTargetDesc/Cpu0FixupKinds.cpp
.. code-block:: c++

    enum Fixups {
      ...
      // PC relative branch fixup resulting in - R_CPU0_PC16.
      // cpu0 PC16, e.g. beq
      fixup_Cpu0_PC16,
      ...
    };
  
.. rubric:: lbdex/Chapter12_2/MCTargetDesc/Cpu0MCCodeEmitter.cpp
.. code-block:: c++

    // getBranch16TargetOpValue - Return binary encoding of the branch
    // target operand, such as BEQ, BNE. If the machine operand
    // requires relocation, record the relocation and return zero.
    unsigned getBranch16TargetOpValue(const MCInst &MI, unsigned OpNo,
                                    SmallVectorImpl<MCFixup> &Fixups) const;
  
  ...
  /// getBranch16TargetOpValue - Return binary encoding of the branch
  /// target operand. If the machine operand requires relocation,
  /// record the relocation and return zero.
  unsigned Cpu0MCCodeEmitter::
  getBranch16TargetOpValue(const MCInst &MI, unsigned OpNo,
                         SmallVectorImpl<MCFixup> &Fixups) const {
  
    const MCOperand &MO = MI.getOperand(OpNo);
  
    // If the destination is an immediate, we have nothing to do.
    if (MO.isImm()) return MO.getImm();
    assert(MO.isExpr() && "getBranch16TargetOpValue expects only expressions");
  
    const MCExpr *Expr = MO.getExpr();
    Fixups.push_back(MCFixup::Create(0, Expr,
                                     MCFixupKind(Cpu0::fixup_Cpu0_PC16)));
    return 0;
  } // lbd document - mark - getBranch16TargetOpValue


.. rubric:: lbdex/Chapter12_2/MCTargetDesc/Cpu0TargetDesc.cpp
.. code-block:: c++

  static std::string ParseCpu0Triple(StringRef TT, StringRef CPU) {
    ...
    if (TheTriple == "cpu0" || TheTriple == "cpu0el") {
      ...
      } else if (CPU == "cpu032II") {
        Cpu0ArchFeature = "+cpu032II";
      }
    }
    return Cpu0ArchFeature;
  }
  
.. rubric:: lbdex/Chapter12_2/Cpu0.td
.. code-block:: c++

  //===----------------------------------------------------------------------===//
  // Cpu0 Subtarget features                                                    //
  //===----------------------------------------------------------------------===//
  ...
  def FeatureCpu032II    : SubtargetFeature<"cpu032II", "Cpu0ArchVersion",                      
           "Cpu032II", "Cpu032II ISA Support (use instruction slt)">;
  
  def FeatureCpu032III   : SubtargetFeature<"cpu032III", "Cpu0ArchVersion",                      
           "Cpu032III", "Cpu032III ISA Support (use instruction slt)">;
  
  //===----------------------------------------------------------------------===//
  // Cpu0 processors supported.
  //===----------------------------------------------------------------------===//
  ...
  def : Proc<"cpu032II", [FeatureCpu032II]>;
  def : Proc<"cpu032III", [FeatureCpu032III]>;


.. rubric:: lbdex/Chapter12_2/Cpu0InstrInfo.td
.. code-block:: c++

  def NotCpu032II :     Predicate<"!Subtarget.hasCpu032II()">,
                        AssemblerPredicate<"FeatureCpu032I">;
  def HasCpu032II :     Predicate<"Subtarget.hasCpu032II()">,
                        AssemblerPredicate<"!FeatureCpu032III">;
  // !FeatureCpu032III is for disassembler in "llvm-objdump -d"
  
  /* In Cpu0GenSubtargetInfo.inc,
  namespace llvm {
  namespace Cpu0 {
  enum {
    FeatureCpu032I =  1ULL << 0,
    FeatureCpu032II =  1ULL << 1,
    FeatureCpu032III =  1ULL << 2
  };
  }
  } // End llvm namespace 
  
  static bool checkDecoderPredicate(unsigned Idx, uint64_t Bits) {
    switch (Idx) {
    default: llvm_unreachable("Invalid index!");
    case 0:
      return ((Bits & Cpu0::FeatureCpu032I)); // came from "FeatureCpu032I"
    case 1:
      return (!(Bits & Cpu0::FeatureCpu032III)); // came from !FeatureCpu032III"
    }
  }
  
  To let disassembler work, the function 
  checkDecoderPredicate(unsigned Idx, uint64_t Bits) must return true(=1).
  As above code, the argument Bits always is 1. Set !FeatureCpu032III" to do 
  disassembler for expectation. */
  ...
  // BEQ, BNE
  def brtarget16    : Operand<OtherVT> {
    let EncoderMethod = "getBranch16TargetOpValue";
    let OperandType = "OPERAND_PCREL";
    let DecoderMethod = "DecodeBranch16Target";
  }
  ...
  class ArithOverflowR<bits<8> op, string instr_asm,
                      InstrItinClass itin, RegisterClass RC, bit isComm = 0>:
    FA<op, (outs GPROut:$ra), (ins RC:$rb, RC:$rc),
       !strconcat(instr_asm, "\t$ra, $rb, $rc"), [], itin> {
    let shamt = 0;
    let isCommutable = isComm;
  }
  class CmpInstr<bits<8> op, string instr_asm, 
                 InstrItinClass itin, RegisterClass RC, RegisterClass RD, 
                 bit isComm = 0>:
    ...
    let Predicates = [NotCpu032II];
  }
  // Conditional Branch, e.g. JEQ brtarget24
  class CBranch24<bits<8> op, string instr_asm, RegisterClass RC,
                     list<Register> UseRegs>:
    FJ<op, (outs), (ins RC:$ra, brtarget24:$addr),
               !strconcat(instr_asm, "\t$ra, $addr"),
               [], IIBranch>, Requires<[NotCpu032II]> {
    ...
  //  let Predicates = [HasCpu032II]; // same effect as Requires
  }
  
  // Conditional Branch, e.g. BEQ $r1, $r2, brtarget16
  class CBranch16<bits<8> op, string instr_asm, PatFrag cond_op, RegisterClass RC>:
    FL<op, (outs), (ins RC:$ra, RC:$rb, brtarget16:$imm16),
               !strconcat(instr_asm, "\t$ra, $rb, $imm16"),
               [(brcond (i32 (cond_op RC:$ra, RC:$rb)), bb:$imm16)], IIBranch>, 
               Requires<[HasCpu032II]> {
    let isBranch = 1;
    let isTerminator = 1;
    let hasDelaySlot = 1;
    let Defs = [AT];
  }
  
  // SetCC
  class SetCC_R<bits<8> op, string instr_asm, PatFrag cond_op,
                RegisterClass RC>:
    FA<op, (outs GPROut:$ra), (ins RC:$rb, RC:$rc),
       !strconcat(instr_asm, "\t$ra, $rb, $rc"),
       [(set GPROut:$ra, (cond_op RC:$rb, RC:$rc))],
       IIAlu>, Requires<[HasCpu032II]> {
    let shamt = 0;
  }
  
  class SetCC_I<bits<8> op, string instr_asm, PatFrag cond_op, Operand Od,
                PatLeaf imm_type, RegisterClass RC>:
    FL<op, (outs GPROut:$ra), (ins RC:$rb, Od:$imm16),
       !strconcat(instr_asm, "\t$ra, $rb, $imm16"),
       [(set GPROut:$ra, (cond_op RC:$rb, imm_type:$imm16))],
       IIAlu>, Requires<[HasCpu032II]> {
  }
  ...
  def SLTi    : SetCC_I<0x26, "slti", setlt, simm16, immSExt16, CPURegs>;
  def SLTiu   : SetCC_I<0x27, "sltiu", setult, simm16, immSExt16, CPURegs>;
  def SLT     : SetCC_R<0x28, "slt", setlt, CPURegs>;
  def SLTu    : SetCC_R<0x29, "sltu", setult, CPURegs>;
  ...
  /// Jump and Branch Instructions
  def BEQ     : CBranch<0x37, "beq", seteq, CPURegs>;
  def BNE     : CBranch<0x38, "bne", setne, CPURegs>;
  ...
  // brcond for slt instruction
  multiclass BrcondPatsSlt<RegisterClass RC, Instruction BEQOp, Instruction BNEOp,
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
  
  let Predicates = [NotCpu032II] in {
  defm : BrcondPatsCmp<CPURegs, JEQ, JNE, JLT, JGT, JLE, JGE, CMP, ZERO>;
  }
  
  let Predicates = [HasCpu032II] in {
  defm : BrcondPatsSlt<CPURegs, BEQ, BNE, SLT, SLTu, SLTi, SLTiu, ZERO>;
  }
  // setcc for slt instruction
  multiclass SeteqPatsSlt<RegisterClass RC, Instruction SLTiuOp, Instruction XOROp,
                       Instruction SLTuOp, Register ZEROReg> {
  // a == b
    def : Pat<(seteq RC:$lhs, RC:$rhs),
                  (SLTiuOp (XOROp RC:$lhs, RC:$rhs), 1)>;
  // a != b
    def : Pat<(setne RC:$lhs, RC:$rhs),
                  (SLTuOp ZEROReg, (XOROp RC:$lhs, RC:$rhs))>;
  }
  
  // a <= b
  multiclass SetlePatsSlt<RegisterClass RC, Instruction SLTOp, Instruction SLTuOp> {
    def : Pat<(setle RC:$lhs, RC:$rhs),
  // a <= b is equal to (XORi (b < a), 1)
                  (XORi (SLTOp RC:$rhs, RC:$lhs), 1)>;
    def : Pat<(setule RC:$lhs, RC:$rhs),
                  (XORi (SLTuOp RC:$rhs, RC:$lhs), 1)>;
  }
  
  // a > b
  multiclass SetgtPatsSlt<RegisterClass RC, Instruction SLTOp, Instruction SLTuOp> {
    def : Pat<(setgt RC:$lhs, RC:$rhs),
  // a > b is equal to b < a is equal to setlt(b, a)
                  (SLTOp RC:$rhs, RC:$lhs)>;
    def : Pat<(setugt RC:$lhs, RC:$rhs),
                  (SLTuOp RC:$rhs, RC:$lhs)>;
  }
  
  // a >= b
  multiclass SetgePatsSlt<RegisterClass RC, Instruction SLTOp, Instruction SLTuOp> {
    def : Pat<(setge RC:$lhs, RC:$rhs),
  // a >= b is equal to b <= a
                  (XORi (SLTOp RC:$lhs, RC:$rhs), 1)>;
    def : Pat<(setuge RC:$lhs, RC:$rhs),
                  (XORi (SLTuOp RC:$lhs, RC:$rhs), 1)>;
  }
  
  multiclass SetgeImmPatsSlt<RegisterClass RC, Instruction SLTiOp,
                          Instruction SLTiuOp> {
    def : Pat<(setge RC:$lhs, immSExt16:$rhs),
                  (XORi (SLTiOp RC:$lhs, immSExt16:$rhs), 1)>;
    def : Pat<(setuge RC:$lhs, immSExt16:$rhs),
                  (XORi (SLTiuOp RC:$lhs, immSExt16:$rhs), 1)>;
  }
  
  let Predicates = [NotCpu032II] in {
  defm : SeteqPatsCmp<CPURegs>;
  defm : SetltPatsCmp<CPURegs>;
  defm : SetlePatsCmp<CPURegs>;
  defm : SetgtPatsCmp<CPURegs>;
  defm : SetgePatsCmp<CPURegs>;
  }
  
  let Predicates = [HasCpu032II] in {
  defm : SeteqPatsSlt<CPURegs, SLTiu, XOR, SLTu, ZERO>;
  defm : SetlePatsSlt<CPURegs, SLT, SLTu>;
  defm : SetgtPatsSlt<CPURegs, SLT, SLTu>;
  defm : SetgePatsSlt<CPURegs, SLT, SLTu>;
  defm : SetgeImmPatsSlt<CPURegs, SLTi, SLTiu>;
  }

.. rubric:: lbdex/Chapter12_2/Cpu0ISelDAGToDAG.cpp
.. code-block:: c++

  /// Select instructions not customized! Used for
  /// expanded, promoted and normal instructions
  SDNode* Cpu0DAGToDAGISel::Select(SDNode *Node) {
    ...
    case ISD::SUBE:
    case ISD::ADDE: {
      ...
      const Cpu0TargetMachine &TM = getTargetMachine();
      const Cpu0Subtarget &Subtarget = TM.getSubtarget<Cpu0Subtarget>();
      SDNode *Carry;
      if (Subtarget.hasCpu032II())
        Carry = CurDAG->getMachineNode(Cpu0::SLTu, DL, VT, Ops);
      else {
        SDNode *StatusWord = CurDAG->getMachineNode(Cpu0::CMP, DL, VT, Ops);
        SDValue Constant1 = CurDAG->getTargetConstant(1, VT);
        Carry = CurDAG->getMachineNode(Cpu0::ANDi, DL, VT, 
                                               SDValue(StatusWord,0), Constant1);
      }
      ...
    ...
  }

.. rubric:: lbdex/Chapter12_2/Cpu0Subtarget.h
.. code-block:: c++

  class Cpu0Subtarget : public Cpu0GenSubtargetInfo {
    ...
    enum Cpu0ArchEnum {
      Cpu032I
      , Cpu032II,
      Cpu032III
    };
    ...
    bool hasCpu032I() const { return Cpu0ArchVersion >= Cpu032I; }
    bool hasCpu032II() const { return Cpu0ArchVersion == Cpu032II; }
    ...
  }


As modified as listed above, the Chapter12_1 instructions are work for cpu032I 
and the added instructions in Chapter12_2 is for cpu032II. 
The llc will generate cpu032I cmp, jeq, 
..., instructions when `llc -mcpu=cpu032I` while `llc -mcpu=cpu032II` will
generate slt, beq when meet "if else", "while" and "for" flow control 
statements. Please notify the Cpu0ArchVersion must be initialized as the 
following code, otherwise the Cpu0ArchVersion can be any value and the functions 
hasCpu032I() and hasCpu032II() which support `llc -mcpu=cpu032I` and 
`llc -mcpu=cpu032II` will have trouble.

.. rubric:: lbdex/Chapter3_1/Cpu0Subtarget.cpp
.. code-block:: c++
  
  Cpu0Subtarget::Cpu0Subtarget(const std::string &TT, const std::string &CPU,
                               const std::string &FS, bool little, 
                               Reloc::Model _RM) :
    Cpu0GenSubtargetInfo(TT, CPU, FS),
    Cpu0ArchVersion(Cpu032I), ...

.. rubric:: lbdex/InputFiles/ch12_2.cpp
.. literalinclude:: ../lbdex/InputFiles/ch12_2.cpp
    :start-after: /// start

.. code-block:: bash

  118-165-78-10:InputFiles Jonathan$ clang -target mips-unknown-linux-gnu -O2 
  -c ch12_2.cpp -emit-llvm -o ch12_2.bc
  118-165-78-10:InputFiles Jonathan$ /Users/Jonathan/llvm/test/cmake_debug_build/
  bin/Debug/llc -march=cpu0 -mcpu=cpu032I -relocation-model=static -filetype=asm 
  ch12_2.bc -o -
    ...
    addiu $sp, $sp, -8
    addiu $2, $zero, 2
    ld  $3, 12($sp)
    cmp $sw, $3, $2
    st  $sw, 4($sp)             # 4-byte Folded Spill
    addiu $2, $zero, 1
    ld  $3, 8($sp)
    cmp $sw, $3, $2
    andi  $2, $sw, 1
    ld  $sw, 4($sp)             # 4-byte Folded Reload
    andi  $3, $sw, 1
    addu  $2, $3, $2
    addiu $sp, $sp, 8
    ret $lr
    ...
  118-165-78-10:InputFiles Jonathan$ /Users/Jonathan/llvm/test/cmake_debug_build/
  bin/Debug/llc -march=cpu0 -mcpu=cpu032I -relocation-model=static -filetype=asm 
  ch12_2.bc -o -
    ...
    ld  $2, 0($sp)
    slti  $2, $2, 1
    ld  $3, 4($sp)
    slti  $3, $3, 2
    addu  $2, $3, $2
    ret $lr
    ...

Run these two `llc -mcpu` option for Chapter12_2 with ch12_2.cpp get the 
above result. Ignore the move between \$sw and general purpose register in 
`llc -mcpu=cpu032I`, the two cmp instructions in it will has hazard in 
instruction reorder since both of them use \$sw register while  
`llc -mcpu=cpu032II` has not [#Quantitative]_. The slti version can reorder as follows,

.. code-block:: bash

    ...
    ld  $3, 4($sp)
    slti  $3, $3, 2
    ld  $2, 0($sp)
    slti  $2, $2, 1
    addu  $2, $3, $2
    ret $lr
    ...


Cpu0 Verilog language changes
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. rubric:: lbdex/cpu0_verilog/cpu0IIs.v
.. literalinclude:: ../lbdex/cpu0_verilog/cpu0IIs.v

In addition to cpu0IIs.v, the "\`ifdef CPU0II" in cpu0.v is added for extended
instructions, slt, beq and bne.


Run the Cpu0II
~~~~~~~~~~~~~~~~

Run Chapter12_2/ with ch_run_backend.cpp to get result as below. 
It match the output result as comments in ch_run_backend.cpp.

.. rubric:: lbdex/InputFiles/ch_run_backend.cpp
.. literalinclude:: ../lbdex/InputFiles/ch_run_backend.cpp
    :start-after: /// start

.. code-block:: bash

  118-165-77-203:InputFiles Jonathan$ /Users/Jonathan/llvm/test/cmake_debug_build/
  bin/Debug/llc -march=cpu0 -mcpu=cpu032II -relocation-model=static -filetype=obj 
  -stats ch_run_backend.bc -o ch_run_backend.cpu0.o
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
  JonathantekiiMac:cpu0_verilog Jonathan$ iverilog -o cpu0IIs cpu0IIs.v 
  JonathantekiiMac:cpu0_verilog Jonathan$ ./cpu0IIs
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
  -2147483648
  15
  2
  1
  RET to PC < 0, finished!

Run with ch8_1_1.cpp, it reduces some branches from pair instructions "CMP, JXX" 
to 1 single instruction ether is BEQ or BNE, as follows,

.. code-block:: bash

  118-165-77-203:InputFiles Jonathan$ /Users/Jonathan/llvm/test/cmake_debug_build/
  bin/Debug/llc -march=cpu0 -mcpu=cpu032II -relocation-model=static -filetype=asm 
  ch8_1_1.bc -o ch8_1_1.cpu0.s
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


Conditional instruction
------------------------

Since the clang optimization level O1 or above level will generate **select** 
and **select_cc** to support conditional instruction, we add this feature in 
Cpu0 backend too. 

.. rubric:: lbdex/InputFiles/ch12_4.cpp
.. literalinclude:: ../lbdex/InputFiles/ch12_4.cpp
    :start-after: /// start

If you run Chapter12_2 with ch12_4.cpp will get the following result.

.. code-block:: bash

  114-37-150-209:InputFiles Jonathan$ clang -O1 -target mips-unknown-linux-gnu 
  -c ch12_4.cpp -emit-llvm -o ch12_4.bc
  114-37-150-209:InputFiles Jonathan$ ~/llvm/test/cmake_debug_build/bin/Debug/
  llvm-dis ch12_4.bc -o -
  ...
  define i32 @_Z8select_1ii(i32 %a, i32 %b) #0 {
    %1 = icmp slt i32 %a, %b
    %. = select i1 %1, i32 1, i32 2
    ret i32 %.
  }
  
  ; Function Attrs: nounwind readnone
  define i32 @_Z8select_2i(i32 %a) #0 {
    %1 = icmp eq i32 %a, 0
    %. = select i1 %1, i32 3, i32 1
    ret i32 %.
  }
  
  ; Function Attrs: nounwind readnone
  define i32 @_Z11test_selectii(i32 %a, i32 %b) #0 {
    %1 = tail call i32 @_Z8select_1ii(i32 %a, i32 %b)
    %2 = tail call i32 @_Z8select_2i(i32 %a)
    %3 = add nsw i32 %2, %1
    ret i32 %3
  }
  ...

  114-37-150-209:InputFiles Jonathan$ ~/llvm/test/cmake_debug_build/bin/Debug/
  llc -march=cpu0 -mcpu=cpu032I -relocation-model=static -filetype=asm ch12_4.bc 
  -debug -o -
  ...
  === _Z8select_1ii
  Initial selection DAG: BB#0 '_Z8select_1ii:'
  SelectionDAG has 14 nodes:
    ...
          0x39f3fc0: ch = setlt

        0x39f40c0: i1 = setcc 0x39f3cc0, 0x39f3ec0, 0x39f3fc0 [ORD=1]

        0x39f41c0: i32 = Constant<1>

        0x39f42c0: i32 = Constant<2>

      0x39f43c0: i32 = select 0x39f40c0, 0x39f41c0, 0x39f42c0 [ORD=2]
    ...

  Replacing.3 0x39f43c0: i32 = select 0x39f40c0, 0x39f41c0, 0x39f42c0 [ORD=2]

  With: 0x39f47c0: i32 = select_cc 0x39f3cc0, 0x39f3ec0, 0x39f41c0, 0x39f42c0, 
  0x39f3fc0 [ORD=2]
  ...
  Optimized lowered selection DAG: BB#0 '_Z8select_1ii:'
  SelectionDAG has 13 nodes:
    ...
        0x39f41c0: i32 = Constant<1>

        0x39f42c0: i32 = Constant<2>

        0x39f3fc0: ch = setlt

      0x39f47c0: i32 = select_cc 0x39f3cc0, 0x39f3ec0, 0x39f41c0, 0x39f42c0, 
      0x39f3fc0 [ORD=2]
    ...
  LLVM ERROR: Cannot select: 0x39f47c0: i32 = select_cc 0x39f3cc0, 0x39f3ec0, 
  0x39f41c0, 0x39f42c0, 0x39f3fc0 [ORD=2] [ID=10]


As llvm IR of ch12_4.bc as above, clang generate **select** IR for small 
basic control block (if statement only include one assign statement). 
This **select** IR is optimization result for CPU which has conditional 
instructions support. 
And from above llc command debug trace message, IR **select** is changed to 
**select_cc** during DAG optimization stages.


Chapter12_3 support **select** with the following code added and changed.


.. rubric:: lbdex/Chapter12_3/AsmParser/Cpu0AsmParser.cpp
.. literalinclude:: ../../../lib/Target/Cpu0/AsmParser/Cpu0AsmParser.cpp
    :start-after: #define GET_REGISTER_MATCHER
    :end-before: unsigned Cpu0AsmParser::

.. rubric:: lbdex/Chapter12_3/Cpu0InstInfo.td
.. code-block:: c++

  include "Cpu0CondMov.td"

.. rubric:: lbdex/Chapter12_3/Cpu0CondMov.td
.. literalinclude:: ../../../lib/Target/Cpu0/Cpu0CondMov.td


.. rubric:: lbdex/Chapter12_3/Cpu0ISelLowering.h
.. code-block:: c++

    SDValue lowerSELECT(SDValue Op, SelectionDAG &DAG) const;


.. rubric:: lbdex/Chapter12_3/Cpu0ISelLowering.h
.. code-block:: c++

  Cpu0TargetLowering::
  Cpu0TargetLowering(Cpu0TargetMachine &TM)
    : TargetLowering(TM, new Cpu0TargetObjectFile()),
      Subtarget(&TM.getSubtarget<Cpu0Subtarget>()) {
    ...
    setOperationAction(ISD::SELECT,             MVT::i32,   Custom);
    ...  
    setOperationAction(ISD::SELECT_CC,         MVT::Other, Expand);
    ...
  }
  ...
  SDValue Cpu0TargetLowering::
  LowerOperation(SDValue Op, SelectionDAG &DAG) const
  {
    switch (Op.getOpcode())
    {
      ...
      case ISD::SELECT:             return lowerSELECT(Op, DAG);
      ...
    }
    return SDValue();
  }
  ...
  SDValue Cpu0TargetLowering::
  lowerSELECT(SDValue Op, SelectionDAG &DAG) const
  {
    return Op;
  }

Set ISD::SELECT_CC to Expand will stop llvm optimization to merge setcc and 
select into one IR select_cc [#]_. Next the LowerSELECT() return ISD::SELECT as 
Op code directly. Finally the pattern define in Cpu0CondMov.td will 
translate the **select** IR into **movz** or **movn** conditional instruction. 
Let's run Chapter12_3 with ch12_4.cpp and ch_optimize.cpp to get the following 
result. 
Again, the cpu032II use **slt** instead of **cmp** has a little improved in 
instructions number.

.. code-block:: bash

  114-37-150-209:InputFiles Jonathan$ ~/llvm/test/cmake_debug_build/bin/Debug/llc 
  -march=cpu0 -mcpu=cpu032I -relocation-model=static -filetype=asm ch12_4.bc -debug 
  -o -
  ...
  === _Z8select_1ii
  Initial selection DAG: BB#0 '_Z8select_1ii:'
  SelectionDAG has 14 nodes:
    ...
          0x3d87fc0: ch = setlt

        0x3d880c0: i1 = setcc 0x3d87cc0, 0x3d87ec0, 0x3d87fc0 [ORD=1]

        0x3d881c0: i32 = Constant<1>

        0x3d882c0: i32 = Constant<2>

      0x3d883c0: i32 = select 0x3d880c0, 0x3d881c0, 0x3d882c0 [ORD=2]
    ...
  Optimized lowered selection DAG: BB#0 '_Z8select_1ii:'
  SelectionDAG has 14 nodes:
    ...
          0x3d87fc0: ch = setlt

        0x3d880c0: i1 = setcc 0x3d87cc0, 0x3d87ec0, 0x3d87fc0 [ORD=1]

        0x3d881c0: i32 = Constant<1>

        0x3d882c0: i32 = Constant<2>

      0x3d883c0: i32 = select 0x3d880c0, 0x3d881c0, 0x3d882c0 [ORD=2]
    ...

  114-37-150-209:InputFiles Jonathan$ ~/llvm/test/cmake_debug_build/bin/Debug/llc 
  -march=cpu0 -mcpu=cpu032I -relocation-model=static -filetype=asm ch12_4.bc -o -
  	.section .mdebug.abi32
  	.previous
  	.file	"ch12_4.bc"
  	.text
  	.globl	_Z8select_1ii
  	.align	2
  	.type	_Z8select_1ii,@function
  	.ent	_Z8select_1ii           # @_Z8select_1ii
  _Z8select_1ii:
  	.frame	$sp,0,$lr
  	.mask 	0x00000000,0
  	.set	noreorder
  	.set	nomacro
  # BB#0:
  	ld	$2, 4($sp)
  	ld	$3, 0($sp)
  	cmp	$sw, $3, $2
  	andi	$3, $sw, 1
  	addiu	$2, $zero, 2
  	addiu	$4, $zero, 1
  	movn	$2, $4, $3
  	ret	$lr
  	.set	macro
  	.set	reorder
  	.end	_Z8select_1ii
  $tmp0:
  	.size	_Z8select_1ii, ($tmp0)-_Z8select_1ii
  
  	.globl	_Z8select_2i
  	.align	2
  	.type	_Z8select_2i,@function
  	.ent	_Z8select_2i            # @_Z8select_2i
  _Z8select_2i:
  	.frame	$sp,0,$lr
  	.mask 	0x00000000,0
  	.set	noreorder
  	.set	nomacro
  # BB#0:
  	addiu	$2, $zero, 1
  	ld	$3, 0($sp)
  	addiu	$4, $zero, 3
  	movz	$2, $4, $3
  	ret	$lr
  	.set	macro
  	.set	reorder
  	.end	_Z8select_2i
  $tmp1:
  	.size	_Z8select_2i, ($tmp1)-_Z8select_2i
  
  	.globl	_Z11test_selectii
  	.align	2
  	.type	_Z11test_selectii,@function
  	.ent	_Z11test_selectii       # @_Z11test_selectii
  _Z11test_selectii:
  	.frame	$sp,32,$lr
  	.mask 	0x00004300,-4
  	.set	noreorder
  	.set	nomacro
  # BB#0:
  	addiu	$sp, $sp, -32
  	st	$lr, 28($sp)            # 4-byte Folded Spill
  	st	$9, 24($sp)             # 4-byte Folded Spill
  	st	$8, 20($sp)             # 4-byte Folded Spill
  	ld	$2, 36($sp)
  	st	$2, 4($sp)
  	ld	$9, 32($sp)
  	st	$9, 0($sp)
  	jsub	_Z8select_1ii
  	add	$8, $zero, $2
  	st	$9, 0($sp)
  	jsub	_Z8select_2i
  	addu	$2, $2, $8
  	ld	$8, 20($sp)             # 4-byte Folded Reload
  	ld	$9, 24($sp)             # 4-byte Folded Reload
  	ld	$lr, 28($sp)            # 4-byte Folded Reload
  	addiu	$sp, $sp, 32
  	ret	$lr
  	.set	macro
  	.set	reorder
  	.end	_Z11test_selectii
  $tmp4:
  	.size	_Z11test_selectii, ($tmp4)-_Z11test_selectii
  	
  114-37-150-209:InputFiles Jonathan$ ~/llvm/test/cmake_debug_build/bin/Debug/llc 
  -march=cpu0 -mcpu=cpu032II -relocation-model=static -filetype=asm ch12_4.bc -o -
  	.section .mdebug.abi32
  	.previous
  	.file	"ch12_4.bc"
  	.text
  	.globl	_Z8select_1ii
  	.align	2
  	.type	_Z8select_1ii,@function
  	.ent	_Z8select_1ii           # @_Z8select_1ii
  _Z8select_1ii:
  	.frame	$sp,0,$lr
  	.mask 	0x00000000,0
  	.set	noreorder
  	.set	nomacro
  # BB#0:
  	ld	$2, 4($sp)
  	ld	$3, 0($sp)
  	slt	$3, $3, $2
  	addiu	$2, $zero, 2
  	addiu	$4, $zero, 1
  	movn	$2, $4, $3
  	ret	$lr
  	...
  
.. rubric:: lbdex/InputFiles/ch_optimize.cpp
.. literalinclude:: ../lbdex/InputFiles/ch_optimize.cpp
    :start-after: /// start

.. rubric:: lbdex/InputFiles/build-optimize.sh
.. literalinclude:: ../lbdex/InputFiles/build-optimize.sh

.. code-block:: bash

  114-37-150-209:InputFiles Jonathan$ bash build-optimize.sh 
  OS = Darwin
  CPU = cpu032I
  ...
  114-37-150-209:InputFiles Jonathan$ cd ../cpu0_verilog/
  114-37-150-209:cpu0_verilog Jonathan$ iverilog -o cpu0Is cpu0Is.v
  114-37-150-209:cpu0_verilog Jonathan$ ./cpu0Is
  WARNING: ./cpu0.v:373: $readmemh(cpu0.hex): Not enough words in the file for 
  the requested range [0:524287].
  taskInterrupt(001)
  2
  RET to PC < 0, finished!
  
  114-37-150-209:cpu0_verilog Jonathan$ cd ../InputFiles/
  114-37-150-209:InputFiles Jonathan$ bash build-optimize.sh cpu032II
  OS = Darwin
  CPU = cpu032II
  ...
  114-37-150-209:InputFiles Jonathan$ cd ../cpu0_verilog/
  114-37-150-209:cpu0_verilog Jonathan$ iverilog -o cpu0IIs cpu0IIs.v
  114-37-150-209:cpu0_verilog Jonathan$ ./cpu0IIs
  WARNING: ./cpu0.v:373: $readmemh(cpu0.hex): Not enough words in the file for 
  the requested range [0:524287].
  taskInterrupt(001)
  2
  RET to PC < 0, finished!


Compare to the non-optimize version which don't use conditional move 
instructions as the following. The clang use **select** IR in small basic block 
to reduce the branch cost in pipeline machine since the branch will make the 
pipeline stall. 
But it needs the conditional instruction support [#Quantitative]_. 
If your backend has no conditional instruction and want the clang compiler with 
optimization option **O1** level above, you can change clang to force it 
generate traditional branch basic block instead of IR **select**.
RISC CPU came from pipeline advantage and add more and more instruction as time 
passed. Compare Mips and ARM, the Mips has only **movz** and **movn** two 
instructions while ARM has many. We create Cpu0 instructions as a RISC pipeline 
machine as well as simple instructions for compiler toolchain tutorial. 
Anyway the **cmp** instruction hired because many programmer used 
it in pass and now (ARM use it). It match the thinking in assembly programming. 
But the **slt** instruction is more efficient in RISC pipleline.
If you designed a backend aimed for C/C++ highlevel language, you should 
consider **slt** instead **cmp**. Assembly is rare case in programming and 
the assembly programmer can accept **slt** too since usually they are 
professional.

 
.. code-block:: bash

  114-37-150-209:InputFiles Jonathan$ clang -O0 -target mips-unknown-linux-gnu 
  -c ch12_4.cpp -emit-llvm -o ch12_4.bc
  
  114-37-150-209:InputFiles Jonathan$ ~/llvm/test/cmake_debug_build/bin/Debug/
  llvm-dis ch12_4.bc -o -
  ...
  define i32 @_Z8select_1ii(i32 %a, i32 %b) #0 {
    %1 = alloca i32, align 4
    %2 = alloca i32, align 4
    %3 = alloca i32, align 4
    %c = alloca i32, align 4
    store i32 %a, i32* %2, align 4
    store i32 %b, i32* %3, align 4
    store i32 0, i32* %c, align 4
    %4 = load i32* %2, align 4
    %5 = load i32* %3, align 4
    %6 = icmp slt i32 %4, %5
    br i1 %6, label %7, label %8
  
  ; <label>:7                                       ; preds = %0
    store i32 1, i32* %1
    br label %9
  
  ; <label>:8                                       ; preds = %0
    store i32 2, i32* %1
    br label %9
  
  ; <label>:9                                       ; preds = %8, %7
    %10 = load i32* %1
    ret i32 %10
  }
  ...
  
  114-37-150-209:InputFiles Jonathan$ ~/llvm/test/cmake_debug_build/bin/Debug/llc 
  -march=cpu0 -mcpu=cpu032II -relocation-model=static -filetype=asm ch12_4.bc -o -
    .section .mdebug.abi32
    .previous
    .file "ch12_4.bc"
    .text
    .globl  _Z8select_1ii
    .align  2
    .type _Z8select_1ii,@function
    .ent  _Z8select_1ii           # @_Z8select_1ii
  _Z8select_1ii:
    .frame  $sp,16,$lr
    .mask   0x00000000,0
    .set  noreorder
    .set  nomacro
  # BB#0:
    addiu $sp, $sp, -16
    ld  $2, 16($sp)
    st  $2, 8($sp)
    ld  $2, 20($sp)
    st  $2, 4($sp)
    addiu $2, $zero, 0
    st  $2, 0($sp)
    ld  $2, 4($sp)
    ld  $3, 8($sp)
    slt $2, $3, $2
    beq $2, $zero, $BB0_2
  # BB#1:
    addiu $2, $zero, 1
    st  $2, 12($sp)
    jmp $BB0_3
  $BB0_2:
    addiu $2, $zero, 2
    st  $2, 12($sp)
  $BB0_3:
    ld  $2, 12($sp)
    addiu $sp, $sp, 16
    ret $lr


.. [#] On a platform with cache and DRAM, the cache miss cost serveral tens 
       time of instruction cycle. The compiler engineers work in the vendor of 
       platform solution spend much effort try to reduce the cache miss for 
       speed. Reduce code size will cut down the cache miss frequency too.

.. [#] http://llvm.org/docs/WritingAnLLVMBackend.html#expand

.. [#Quantitative] See book Computer Architecture: A Quantitative Approach (The Morgan 
       Kaufmann Series in Computer Architecture and Design) 


