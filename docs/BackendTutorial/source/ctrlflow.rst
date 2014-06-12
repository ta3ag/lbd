.. _sec-controlflow:

Control flow statements
=======================

This chapter illustrates the corresponding IR for control flow statements, like 
**“if else”**, **“while”** and **“for”** loop statements in C, and how to 
translate these control flow statements of llvm IR into Cpu0 instructions in 
section I. In section II, an optimization pass of control flow for backend is 
introduced. It's a simple tutorial program to let readers know how to add a 
backend optimization pass and program it. Section III, include the conditional 
instructions handle since the clang will generate specific IR select and 
select_cc to support the backend optimiation in control flow statement.


Control flow statement
-----------------------

Run ch8_1_1.cpp with clang will get result as follows,

.. rubric:: lbdex/InputFiles/ch8_1_1.cpp
.. literalinclude:: ../lbdex/InputFiles/ch8_1_1.cpp
    :start-after: /// start

.. code-block:: bash

  ; Function Attrs: nounwind uwtable
  define i32 @_Z13test_control1v() #0 {
  entry:
    %a = alloca i32, align 4
    %b = alloca i32, align 4
    %c = alloca i32, align 4
    %d = alloca i32, align 4
    %e = alloca i32, align 4
    %f = alloca i32, align 4
    %g = alloca i32, align 4
    %h = alloca i32, align 4
    %i = alloca i32, align 4
    %j = alloca i32, align 4
    store i32 0, i32* %a, align 4
    store i32 1, i32* %b, align 4
    store i32 2, i32* %c, align 4
    store i32 3, i32* %d, align 4
    store i32 4, i32* %e, align 4
    store i32 5, i32* %f, align 4
    store i32 6, i32* %g, align 4
    store i32 7, i32* %h, align 4
    store i32 8, i32* %i, align 4
    store i32 9, i32* %j, align 4
    %0 = load i32* %a, align 4
    %cmp = icmp eq i32 %0, 0
    br i1 %cmp, label %if.then, label %if.end

  if.then:                                          ; preds = %entry
    %1 = load i32* %a, align 4
    %inc = add i32 %1, 1
    store i32 %inc, i32* %a, align 4
    br label %if.end

  if.end:                                           ; preds = %if.then, %entry
    %2 = load i32* %b, align 4
    %cmp1 = icmp ne i32 %2, 0
    br i1 %cmp1, label %if.then2, label %if.end4

  if.then2:                                         ; preds = %if.end
    %3 = load i32* %b, align 4
    %inc3 = add nsw i32 %3, 1
    store i32 %inc3, i32* %b, align 4
    br label %if.end4

  if.end4:                                          ; preds = %if.then2, %if.end
    %4 = load i32* %c, align 4
    %cmp5 = icmp sgt i32 %4, 0
    br i1 %cmp5, label %if.then6, label %if.end8

  if.then6:                                         ; preds = %if.end4
    %5 = load i32* %c, align 4
    %inc7 = add nsw i32 %5, 1
    store i32 %inc7, i32* %c, align 4
    br label %if.end8

  if.end8:                                          ; preds = %if.then6, %if.end4
    %6 = load i32* %d, align 4
    %cmp9 = icmp sge i32 %6, 0
    br i1 %cmp9, label %if.then10, label %if.end12

  if.then10:                                        ; preds = %if.end8
    %7 = load i32* %d, align 4
    %inc11 = add nsw i32 %7, 1
    store i32 %inc11, i32* %d, align 4
    br label %if.end12

  if.end12:                                         ; preds = %if.then10, %if.end8
    %8 = load i32* %e, align 4
    %cmp13 = icmp slt i32 %8, 0
    br i1 %cmp13, label %if.then14, label %if.end16

  if.then14:                                        ; preds = %if.end12
    %9 = load i32* %e, align 4
    %inc15 = add nsw i32 %9, 1
    store i32 %inc15, i32* %e, align 4
    br label %if.end16

  if.end16:                                         ; preds = %if.then14, %if.end12
    %10 = load i32* %f, align 4
    %cmp17 = icmp sle i32 %10, 0
    br i1 %cmp17, label %if.then18, label %if.end20

  if.then18:                                        ; preds = %if.end16
    %11 = load i32* %f, align 4
    %inc19 = add nsw i32 %11, 1
    store i32 %inc19, i32* %f, align 4
    br label %if.end20

  if.end20:                                         ; preds = %if.then18, %if.end16
    %12 = load i32* %g, align 4
    %cmp21 = icmp sle i32 %12, 1
    br i1 %cmp21, label %if.then22, label %if.end24

  if.then22:                                        ; preds = %if.end20
    %13 = load i32* %g, align 4
    %inc23 = add nsw i32 %13, 1
    store i32 %inc23, i32* %g, align 4
    br label %if.end24

  if.end24:                                         ; preds = %if.then22, %if.end20
    %14 = load i32* %h, align 4
    %cmp25 = icmp sge i32 %14, 1
    br i1 %cmp25, label %if.then26, label %if.end28

  if.then26:                                        ; preds = %if.end24
    %15 = load i32* %h, align 4
    %inc27 = add nsw i32 %15, 1
    store i32 %inc27, i32* %h, align 4
    br label %if.end28

  if.end28:                                         ; preds = %if.then26, %if.end24
    %16 = load i32* %i, align 4
    %17 = load i32* %h, align 4
    %cmp29 = icmp slt i32 %16, %17
    br i1 %cmp29, label %if.then30, label %if.end32

  if.then30:                                        ; preds = %if.end28
    %18 = load i32* %i, align 4
    %inc31 = add nsw i32 %18, 1
    store i32 %inc31, i32* %i, align 4
    br label %if.end32

  if.end32:                                         ; preds = %if.then30, %if.end28
    %19 = load i32* %a, align 4
    %20 = load i32* %b, align 4
    %cmp33 = icmp ne i32 %19, %20
    br i1 %cmp33, label %if.then34, label %if.end36

  if.then34:                                        ; preds = %if.end32
    %21 = load i32* %j, align 4
    %inc35 = add nsw i32 %21, 1
    store i32 %inc35, i32* %j, align 4
    br label %if.end36

  if.end36:                                         ; preds = %if.then34, %if.end32
    %22 = load i32* %a, align 4
    %23 = load i32* %b, align 4
    %add = add i32 %22, %23
    %24 = load i32* %c, align 4
    %add37 = add i32 %add, %24
    %25 = load i32* %d, align 4
    %add38 = add i32 %add37, %25
    %26 = load i32* %e, align 4
    %add39 = add i32 %add38, %26
    %27 = load i32* %f, align 4
    %add40 = add i32 %add39, %27
    %28 = load i32* %g, align 4
    %add41 = add i32 %add40, %28
    %29 = load i32* %h, align 4
    %add42 = add i32 %add41, %29
    %30 = load i32* %i, align 4
    %add43 = add i32 %add42, %30
    %31 = load i32* %j, align 4
    %add44 = add i32 %add43, %31
    ret i32 %add44
  }


The **“icmp ne”** stand for integer compare NotEqual, **“slt”** stands for Set 
Less Than, **“sle”** stands for Set Less or Equal. 
Run version Chapter8_1/ with ``llc  -view-isel-dags`` or ``-debug`` option, you 
can see the **if** statement is translated into 
(br (brcond (%1, setcc(%2, Constant<c>, setne)), BasicBlock_02), BasicBlock_01).
Ignore %1, we get the form (br (brcond (setcc(%2, Constant<c>, setne)), 
BasicBlock_02), BasicBlock_01). 
For explanation, We list the IR DAG as follows,

.. code-block:: bash

    %cond=setcc(%2, Constant<c>, setne)
    brcond %cond, BasicBlock_02
    br BasicBlock_01
    
We want to translate them into Cpu0 instructions DAG as follows,

.. code-block:: bash

    addiu %3, ZERO, Constant<c>
    cmp %2, %3
    jne BasicBlock_02
    jmp BasicBlock_01

For the last IR br, we translate unconditional branch (br BasicBlock_01) into 
jmp BasicBlock_01 by the following pattern definition,

.. rubric:: lbdex/Chapter8_1/Cpu0InstrInfo.td
.. code-block:: c++

    // Unconditional branch
    class UncondBranch<bits<8> op, string instr_asm>:
      BranchBase<op, (outs), (ins brtarget:$imm24),
                 !strconcat(instr_asm, "\t$imm24"), [(br bb:$imm24)], IIBranch> {
      let isBranch = 1;
      let isTerminator = 1;
      let isBarrier = 1;
      let hasDelaySlot = 0;
    }
    ...
    def JMP     : UncondBranch<0x26, "jmp">;

The pattern [(br bb:$imm24)] in class UncondBranch is translated into jmp 
machine instruction.
The **cmp** and **jne** two Cpu0 instructions translation is more complicate 
than simple 
one-to-one IR to machine instruction translation we have experienced until now. 
To solve this chained IR to machine instructions translation, we define the 
following pattern,

.. rubric:: lbdex/Chapter8_1/Cpu0InstrInfo.td
.. code-block:: c++

    // brcond patterns
    multiclass BrcondPats<RegisterClass RC, Instruction JEQOp, Instruction JNEOp, 
      Instruction JLTOp, Instruction JGTOp, Instruction JLEOp, Instruction JGEOp, 
      Instruction CMPOp> {
    ...
    def : Pat<(brcond (i32 (setne RC:$lhs, RC:$rhs)), bb:$dst),
              (JNEOp (CMPOp RC:$lhs, RC:$rhs), bb:$dst)>;
    ...
    def : Pat<(brcond RC:$cond, bb:$dst),
              (JNEOp (CMPOp RC:$cond, ZEROReg), bb:$dst)>;

Since the BrcondPats pattern as above use RC (Register Class) as operand, the 
following ADDiu pattern defined in Chapter3_5 will generate instruction 
**addiu** before the instruction **cmp** for the first IR, 
**setcc(%2, Constant<c>, setne)** as above.

.. rubric:: lbdex/Chapter3_5/Cpu0InstrInfo.td
.. code-block:: c++

    // Small immediates
    def : Pat<(i32 immSExt16:$in),
              (ADDiu ZERO, imm:$in)>;
    
    // Arbitrary immediates
    def : Pat<(i32 imm:$imm),
          (ORi (LUi (HI16 imm:$imm)), (LO16 imm:$imm))>;

The definition BrcondPats support setne, seteq, setlt, ..., register operand 
compare and setult, setugt, ..., for unsigned int type. In addition to seteq 
and setne, we define setueq and setune, by reference Mips code even though 
we didn't find how to generate setune IR from C language. 
We have tried to define unsigned int type, but clang still generate setne 
instead of setune. 
Pattern search order is according their appear order in context. 
The last pattern (brcond RC:$cond, bb:$dst) is meaning branch to $dst 
if $cond != 0, it is equal to (JNEOp (CMPOp RC:$cond, ZEROReg), bb:$dst) in 
cpu0 translation.

The CMP instruction will set the result to register SW, and then JNE check the 
condition based on SW status as :num:`Figure #ctrlflow-f1`. 
Since SW belongs to a different register class, it 
is correct even an instruction is inserted between CMP and JNE as follows,

.. _ctrlflow-f1:
.. figure:: ../Fig/ctrlflow/1.png
  :height: 465 px
  :width: 446 px
  :align: center

  JNE (CMP $r2, $r3),

.. code-block:: c++

    cmp %2, %3
    addiu $r1, $r2, 3   // $r1 register never be allocated to $SW because in 
                        //  class ArithLogicI, GPROut is the output register 
                        //  class and the GPROut is defined without $SW in 
                        //  Cpu0RegisterInforGPROutForOther.td
    jne BasicBlock_02


The reserved registers setting by the following 
function code we defined before,

.. rubric:: lbdex/Chapter8_1/Cpu0RegisterInfo.cpp
.. code-block:: c++

  // pure virtual method
  BitVector Cpu0RegisterInfo::
  getReservedRegs(const MachineFunction &MF) const {
    static const uint16_t ReservedCPURegs[] = {
      Cpu0::ZERO, Cpu0::AT, Cpu0::SP, Cpu0::LR, Cpu0::PC
    };
    BitVector Reserved(getNumRegs());
    typedef TargetRegisterClass::iterator RegIter;

    for (unsigned I = 0; I < array_lengthof(ReservedCPURegs); ++I)
      Reserved.set(ReservedCPURegs[I]);

    const Cpu0FunctionInfo *Cpu0FI = MF.getInfo<Cpu0FunctionInfo>();
    // Reserve GP if globalBaseRegFixed()
    if (Cpu0FI->globalBaseRegFixed())
      Reserved.set(Cpu0::GP);

    return Reserved;
  }

Although the following definition in Cpu0RegisterInfo.td has no real effect in 
Reserved Registers, you should comment the Reserved Registers in it for 
readability. Setting SW both in register class CPURegs and SR to allow the SW 
to be accessed by RISC instructions like ``andi`` and allow programmer use 
traditional assembly instruction ``cmp``. 
The copyPhysReg() is called when DestReg and SrcReg belong to different Register 
Class. 

.. rubric:: lbdex/Chapter2/Cpu0RegisterInfo.td
.. code-block:: c++

  //===----------------------------------------------------------------------===//
  // Register Classes
  //===----------------------------------------------------------------------===//
  
  def CPURegs : RegisterClass<"Cpu0", [i32], 32, (add
    // Reserved
    ZERO, AT, 
    // Return Values and Arguments
    V0, V1, A0, A1, 
    // Not preserved across procedure calls
    T9, T0,
    // Callee save
    S0, S1, SW, 
    // Reserved
    GP, FP, 
    SP, LR, PC)>;
  ...
  // Status Registers
  def SR   : RegisterClass<"Cpu0", [i32], 32, (add SW)>;
  

.. rubric:: lbdex/Chapter2/Cpu0RegisterInfoGPROutForOther.td
.. literalinclude:: ../../../lib/Target/Cpu0/Cpu0RegisterInfoGPROutForOther.td


Chapter8_1/ include support for control flow statement. 
Run with it as well as the following ``llc`` option, you can get the obj file 
and dump it's content by gobjdump or hexdump as follows,

.. code-block:: bash

    118-165-79-206:InputFiles Jonathan$ cat ch8_1_1.cpu0.s 
    ...
    ld  $4, 36($fp)
    cmp $sw, $4, $3
    jne $BB0_2
    jmp $BB0_1
  $BB0_1:                                 # %if.then
    ld  $4, 36($fp)
    addiu $4, $4, 1
    st  $4, 36($fp)
  $BB0_2:                                 # %if.end
    ld  $4, 32($fp)
    ...

.. code-block:: bash
    
    118-165-79-206:InputFiles Jonathan$ /Users/Jonathan/llvm/test/
    cmake_debug_build/bin/Debug/llc -march=cpu0 -mcpu=cpu032I -relocation-model=pic 
    -filetype=obj ch8_1_1.bc -o ch8_1_1.cpu0.o

    118-165-79-206:InputFiles Jonathan$ hexdump ch8_1_1.cpu0.o 
        // jmp offset is 0x10=16 bytes which is correct
    0000080 ...................................... 10 43 00 00
    0000090 31 00 00 10 36 00 00 00 ..........................

The immediate value of jne (op 0x31) is 16; The offset between jne and $BB0_2 
is 20 (5 words = 5*4 bytes). Suppose the jne address is X, then the label 
$BB0_2 is X+20. 
Cpu0 is a RISC cpu0 with 3 stages of pipeline which are fetch, decode and 
execution according to cpu0 web site information. 
The cpu0 do branch instruction execution at decode stage which like mips. 
After the jne instruction fetched, the PC (Program Counter) is X+4 since cpu0 
update PC at fetch stage. 
The $BB0_2 address is equal to PC+16 for the jne branch instruction execute at 
decode stage. 
List and explain this again as follows,

.. code-block:: bash

                // Fetch instruction stage for jne instruction. The fetch stage 
                // can be divided into 2 cycles. First cycle fetch the 
                // instruction. Second cycle adjust PC = PC+4. 
    jne $BB0_2  // Do jne compare in decode stage. PC = X+4 at this stage. 
                // When jne immediate value is 16, PC = PC+16. It will fetch 
                //  X+20 which equal to label $BB0_2 instruction, ld $2, 28($sp). 
    jmp $BB0_1
  $BB0_1:                                 # %if.then
    ld  $4, 36($fp)
    addiu $4, $4, 1
    st  $4, 36($fp)
  $BB0_2:                                 # %if.end
    ld  $4, 32($fp)

If cpu0 do **"jne"** compare in execution stage, then we should set PC=PC+12, 
offset of ($BB0_2, jn e $BB02) – 8, in this example.

In reality, the conditional branch is important in performance of CPU design. 
According bench mark information, every 7 instructions will meet 1 branch 
instruction in average. 
The cpu032I take 2 instructions for conditional branch, (jne(cmp...)), while 
cpu032II use one instruction (bne) as follws,

.. code-block:: bash

  JonathantekiiMac:InputFiles Jonathan$ /Users/Jonathan/llvm/test/
  cmake_debug_build/bin/Debug/llc -march=cpu0 -mcpu=cpu032I -relocation-model=pic 
  -filetype=asm ch8_1_1.bc -o -
    ...
  	cmp	$sw, $4, $3
  	jne	$sw, $BB0_2
  	jmp	$BB0_1
  $BB0_1:
  
  JonathantekiiMac:InputFiles Jonathan$ /Users/Jonathan/llvm/test/
  cmake_debug_build/bin/Debug/llc -march=cpu0 -mcpu=cpu032II -relocation-model=pic 
  -filetype=asm ch8_1_1.bc -o -
    ...
  	bne	$4, $zero, $BB0_2
  	jmp	$BB0_1
  $BB0_1:


Finally we list the code added for full support of control flow statement,

.. rubric:: lbdex/Chapter8_1/MCTargetDesc/Cpu0MCCodeEmitter.cpp
.. literalinclude:: ../../../lib/Target/Cpu0/MCTargetDesc/Cpu0MCCodeEmitter.cpp
    :start-after: // lbd document - mark - declare getBranch16TargetOpValue
    :end-before: // getMachineOpValue - Return binary encoding of operand. 
.. literalinclude:: ../../../lib/Target/Cpu0/MCTargetDesc/Cpu0MCCodeEmitter.cpp
    :start-after: // lbd document - mark - getBranch16TargetOpValue
    :end-before: /// getMachineOpValue - Return binary encoding of operand

.. rubric:: lbdex/Chapter8_1/Cpu0ISelLowering.cpp
.. code-block:: c++

    Cpu0TargetLowering::
    Cpu0TargetLowering(Cpu0TargetMachine &TM)
      : TargetLowering(TM, new Cpu0TargetObjectFile()),
        Subtarget(&TM.getSubtarget<Cpu0Subtarget>()) {
      ...
      // Used by legalize types to correctly generate the setcc result.
      // Without this, every float setcc comes with a AND/OR with the result,
      // we don't want this, since the fpcmp result goes to a flag register,
      // which is used implicitly by brcond and select operations.
      AddPromotedToType(ISD::SETCC, MVT::i1, MVT::i32);
      ...
      setOperationAction(ISD::BRCOND,             MVT::Other, Custom);
      ...
      // Operations not directly supported by Cpu0.
      setOperationAction(ISD::BR_CC,             MVT::i32, Expand);
      ...
    }
    ...
    SDValue Cpu0TargetLowering::
    LowerOperation(SDValue Op, SelectionDAG &DAG) const
    {
      switch (Op.getOpcode())
      {
        case ISD::BRCOND:             return LowerBRCOND(Op, DAG);
        ...
      }
      ...
    }
    ...
    SDValue Cpu0TargetLowering::
    LowerBRCOND(SDValue Op, SelectionDAG &DAG) const
    {
      return Op;
    }

.. rubric:: lbdex/Chapter8_1/Cpu0ISelLowering.h
.. code-block:: c++

      SDValue LowerBRCOND(SDValue Op, SelectionDAG &DAG) const;

.. rubric:: lbdex/Chapter8_1/Cpu0MCInstLower.cpp
.. code-block:: c++

    MCOperand Cpu0MCInstLower::LowerSymbolOperand(const MachineOperand &MO,
                                                  MachineOperandType MOTy,
                                                  unsigned Offset) const {
      ...
      switch (MOTy) {
      ...
      case MachineOperand::MO_MachineBasicBlock:
        Symbol = MO.getMBB()->getSymbol();
        break;
      ...
      case MachineOperand::MO_BlockAddress:
      Symbol = AsmPrinter.GetBlockAddressSymbol(MO.getBlockAddress());
      Offset += MO.getOffset();
      break;
      ...
    }
    
    MCOperand Cpu0MCInstLower::LowerOperand(const MachineOperand& MO,
                                            unsigned offset) const {
      MachineOperandType MOTy = MO.getType();
    
      switch (MOTy) {
      ...
      case MachineOperand::MO_MachineBasicBlock:
      ...
      case MachineOperand::MO_BlockAddress:
      ...
      }
      ...
    }

.. rubric:: lbdex/Chapter8_1/Cpu0InstrFormats.td
.. code-block:: c++

	//===----------------------------------------------------------------------===//
	// Format J instruction class in Cpu0 : <|opcode|address|>
	//===----------------------------------------------------------------------===//
	
	class FJ<bits<8> op, dag outs, dag ins, string asmstr, list<dag> pattern,
			 InstrItinClass itin>: Cpu0Inst<outs, ins, asmstr, pattern, itin, FrmJ>
	{
	  bits<24> addr;
	
	  let Opcode = op;
	
	  let Inst{23-0} = addr;
	}

.. rubric:: lbdex/Chapter8_1/Cpu0InstrInfo.td
.. code-block:: c++

  // Cpu0InstrInfo.td
  // BEQ, BNE
  def brtarget16    : Operand<OtherVT> {
    let EncoderMethod = "getBranch16TargetOpValue";
    let OperandType = "OPERAND_PCREL";
    let DecoderMethod = "DecodeBranch16Target";
  }
  
  // Instruction operand types
  def brtarget24    : Operand<OtherVT> {
    let EncoderMethod = "getBranchTargetOpValue";
    let OperandType = "OPERAND_PCREL";
    let DecoderMethod = "DecodeBranchTarget";
  }
  // JMP
  def jmptarget    : Operand<OtherVT> {
    let EncoderMethod = "getJumpTargetOpValue";
    let OperandType = "OPERAND_PCREL";
    let DecoderMethod = "DecodeJumpRelativeTarget";
  }
  ...
  // Conditional Branch, e.g. JEQ brtarget24
  class CBranch24<bits<8> op, string instr_asm, RegisterClass RC,
                     list<Register> UseRegs>:
    FJ<op, (outs), (ins RC:$ra, brtarget24:$addr),
               !strconcat(instr_asm, "\t$ra, $addr"),
               [], IIBranch>, Requires<[HasCmp]> {
    let isBranch = 1;
    let isTerminator = 1;
    let hasDelaySlot = 0;
    let neverHasSideEffects = 1;
  //  let Predicates = [HasCmp]; // same effect as Requires
  }
  
  // Conditional Branch, e.g. BEQ $r1, $r2, brtarget16
  class CBranch16<bits<8> op, string instr_asm, PatFrag cond_op, RegisterClass RC>:
    FL<op, (outs), (ins RC:$ra, RC:$rb, brtarget16:$imm16),
               !strconcat(instr_asm, "\t$ra, $rb, $imm16"),
               [(brcond (i32 (cond_op RC:$ra, RC:$rb)), bb:$imm16)], IIBranch>, 
               Requires<[HasSlt]> {
    let isBranch = 1;
    let isTerminator = 1;
    let hasDelaySlot = 1;
    let Defs = [AT];
  }
  ...
  // Unconditional branch, such as JMP
  class UncondBranch<bits<8> op, string instr_asm>:
    FJ<op, (outs), (ins brtarget:$addr),
         !strconcat(instr_asm, "\t$addr"), [(br bb:$addr)], IIBranch> {
    let isBranch = 1;
    let isTerminator = 1;
    let isBarrier = 1;
    let hasDelaySlot = 0;
  }
  ...
  /// Jump and Branch Instructions
  def JEQ     : CBranch<0x30, "jeq", CPURegs>;
  def JNE     : CBranch<0x31, "jne", CPURegs>;
  def JLT     : CBranch<0x32, "jlt", CPURegs>;
  def JGT     : CBranch<0x33, "jgt", CPURegs>;
  def JLE     : CBranch<0x34, "jle", CPURegs>;
  def JGE     : CBranch<0x35, "jge", CPURegs>;
  
  def BEQ     : CBranch16<0x37, "beq", seteq, GPROut>;
  def BNE     : CBranch16<0x38, "bne", setne, GPROut>;
  
  def JMP     : UncondBranch<0x36, "jmp">;
  ...
  // brcond patterns
  // brcond for cmp instruction
  multiclass BrcondPatsCmp<RegisterClass RC, Instruction JEQOp, Instruction JNEOp, 
  Instruction JLTOp, Instruction JGTOp, Instruction JLEOp, Instruction JGEOp, 
  Instruction CMPOp, Register ZEROReg> {
  def : Pat<(brcond (i32 (seteq RC:$lhs, RC:$rhs)), bb:$dst),
            (JEQOp (CMPOp RC:$lhs, RC:$rhs), bb:$dst)>;
  def : Pat<(brcond (i32 (setueq RC:$lhs, RC:$rhs)), bb:$dst),
            (JEQOp (CMPOp RC:$lhs, RC:$rhs), bb:$dst)>;
  def : Pat<(brcond (i32 (setne RC:$lhs, RC:$rhs)), bb:$dst),
            (JNEOp (CMPOp RC:$lhs, RC:$rhs), bb:$dst)>;
  def : Pat<(brcond (i32 (setune RC:$lhs, RC:$rhs)), bb:$dst),
            (JNEOp (CMPOp RC:$lhs, RC:$rhs), bb:$dst)>;
  def : Pat<(brcond (i32 (setlt RC:$lhs, RC:$rhs)), bb:$dst),
            (JLTOp (CMPOp RC:$lhs, RC:$rhs), bb:$dst)>;
  def : Pat<(brcond (i32 (setult RC:$lhs, RC:$rhs)), bb:$dst),
            (JLTOp (CMPOp RC:$lhs, RC:$rhs), bb:$dst)>;
  def : Pat<(brcond (i32 (setgt RC:$lhs, RC:$rhs)), bb:$dst),
            (JGTOp (CMPOp RC:$lhs, RC:$rhs), bb:$dst)>;
  def : Pat<(brcond (i32 (setugt RC:$lhs, RC:$rhs)), bb:$dst),
            (JGTOp (CMPOp RC:$lhs, RC:$rhs), bb:$dst)>;
  def : Pat<(brcond (i32 (setle RC:$lhs, RC:$rhs)), bb:$dst),
            (JLEOp (CMPOp RC:$lhs, RC:$rhs), bb:$dst)>;
  def : Pat<(brcond (i32 (setule RC:$lhs, RC:$rhs)), bb:$dst),
            (JLEOp (CMPOp RC:$lhs, RC:$rhs), bb:$dst)>;
  def : Pat<(brcond (i32 (setge RC:$lhs, RC:$rhs)), bb:$dst),
            (JGEOp (CMPOp RC:$lhs, RC:$rhs), bb:$dst)>;
  def : Pat<(brcond (i32 (setuge RC:$lhs, RC:$rhs)), bb:$dst),
            (JGEOp (CMPOp RC:$lhs, RC:$rhs), bb:$dst)>;
  
  def : Pat<(brcond RC:$cond, bb:$dst),
            (JNEOp (CMPOp RC:$cond, ZEROReg), bb:$dst)>;
  }
  
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
  
  let Predicates = [HasSlt] in {
  defm : BrcondPatsSlt<CPURegs, BEQ, BNE, SLT, SLTu, SLTi, SLTiu, ZERO>;
  }
  
  let Predicates = [HasCmp] in {
  defm : BrcondPatsCmp<CPURegs, JEQ, JNE, JLT, JGT, JLE, JGE, CMP, ZERO>;
  }


The ch8_1_2.cpp is for **“nest if”** test. The ch8_1_3.cpp is the 
**“for loop”** as well as **“while loop”**, **“continue”**, **“break”**, 
**“goto”** test. The ch8_1_4.cpp is for **“goto”** test.
You can run with them if you like to test more.


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
This algorithm is simple and effective to be a perfect tutorial in optimization. 
Through this example, you can understand how to add an optimization pass and 
coding your complicate optimization algorithm on your backend in real project.

Chapter8_2/ supports "delete useless jmp" optimization algorithm which add 
codes as follows,

.. rubric:: lbdex/Chapter8_2/CMakeLists.txt
.. code-block:: c++

  add_llvm_target(Cpu0CodeGen
    ...
    Cpu0DelUselessJMP.cpp
    ...
    )
  
.. rubric:: lbdex/Chapter8_2/Cpu0.h
.. code-block:: c++

  ...
    FunctionPass *createCpu0DelJmpPass(Cpu0TargetMachine &TM);
  
.. rubric:: lbdex/Chapter8_2/Cpu0TargetMachine.cpp
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

.. rubric:: lbdex/Chapter8_2/Cpu0DelUselessJMP.cpp
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
Now, let's run Chapter8_2/ with ch8_2.cpp for explanation.

.. rubric:: lbdex/InputFiles/ch8_2.cpp
.. literalinclude:: ../lbdex/InputFiles/ch8_2.cpp
    :start-after: /// start

.. code-block:: bash

  118-165-78-10:InputFiles Jonathan$ clang -target mips-unknown-linux-gnu 
  -c ch8_2.cpp -emit-llvm -o ch8_2.bc
  118-165-78-10:InputFiles Jonathan$ /Users/Jonathan/llvm/test/cmake_debug_build/
  bin/Debug/llc -march=cpu0 -relocation-model=static -filetype=asm -stats 
  ch8_2.bc -o -
    .section .mdebug.abi32
    .previous
    .file "ch8_2.bc"
    .text
    .globl  _Z18test_DelUselessJMPv
    .align  2
    .type _Z18test_DelUselessJMPv,@function
    .ent  _Z18test_DelUselessJMPv # @_Z18test_DelUselessJMPv
  _Z18test_DelUselessJMPv:
    .frame  $sp,16,$lr
    .mask   0x00000000,0
    .set  noreorder
    .set  nomacro
  # BB#0:
    addiu $sp, $sp, -16
    addiu $2, $zero, 1
    st  $2, 12($sp)
    addiu $3, $zero, 2
    st  $3, 8($sp)
    addiu $3, $zero, 3
    st  $3, 4($sp)
    addiu $3, $zero, 0
    ld  $4, 12($sp)
    cmp $sw, $4, $3
    jne $sw, $BB0_2
  # BB#1:
    ld  $4, 12($sp)
    addiu $4, $4, 1
    st  $4, 12($sp)
  $BB0_2:
    ld  $4, 8($sp)
    cmp $sw, $4, $3
    jne $sw, $BB0_4
    jmp $BB0_3
  $BB0_4:
    addiu $3, $zero, -1
    ld  $4, 8($sp)
    cmp $sw, $4, $3
    jgt $sw, $BB0_6
    jmp $BB0_5
  $BB0_3:
    ld  $3, 12($sp)
    addiu $3, $3, 3
    st  $3, 12($sp)
    ld  $3, 8($sp)
    addiu $3, $3, 1
    st  $3, 8($sp)
    jmp $BB0_6
  $BB0_5:
    ld  $3, 8($sp)
    ld  $4, 12($sp)
    addu  $3, $4, $3
    st  $3, 12($sp)
    ld  $3, 8($sp)
    addiu $3, $3, -1
    st  $3, 8($sp)
  $BB0_6:
    ld  $3, 4($sp)
    cmp $sw, $3, $2
    jlt $sw, $BB0_8
  # BB#7:
    ld  $2, 4($sp)
    ld  $3, 12($sp)
    addu  $2, $3, $2
    st  $2, 12($sp)
    ld  $2, 4($sp)
    addiu $2, $2, 1
    st  $2, 4($sp)
  $BB0_8:
    addiu $sp, $sp, 16
    ret $lr
    .set  macro
    .set  reorder
    .end  _Z18test_DelUselessJMPv
  $tmp1:
    .size _Z18test_DelUselessJMPv, ($tmp1)-_Z18test_DelUselessJMPv
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


Conditional instruction
------------------------

Since the clang optimization level O1 or above level will generate **select** 
and **select_cc** to support conditional instruction, we add this feature in 
Cpu0 backend too. 

.. rubric:: lbdex/InputFiles/ch8_3.cpp
.. literalinclude:: ../lbdex/InputFiles/ch8_3.cpp
    :start-after: /// start

If you run Chapter8_1 with ch8_3.cpp will get the following result.

.. code-block:: bash

  114-37-150-209:InputFiles Jonathan$ clang -O1 -target mips-unknown-linux-gnu 
  -c ch8_3.cpp -emit-llvm -o ch8_3.bc
  114-37-150-209:InputFiles Jonathan$ ~/llvm/test/cmake_debug_build/bin/Debug/
  llvm-dis ch8_3.bc -o -
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
  ...

  114-37-150-209:InputFiles Jonathan$ ~/llvm/test/cmake_debug_build/bin/Debug/
  llc -march=cpu0 -mcpu=cpu032I -relocation-model=static -filetype=asm ch8_3.bc 
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


As llvm IR of ch8_3.bc as above, clang generate **select** IR for small 
basic control block (if statement only include one assign statement). 
This **select** IR is optimization result for CPU which has conditional 
instructions support. 
And from above llc command debug trace message, IR **select** is changed to 
**select_cc** during DAG optimization stages.


Chapter8_2 support **select** with the following code added and changed.

.. rubric:: lbdex/Chapter8_2/Cpu0InstInfo.td
.. code-block:: c++

  include "Cpu0CondMov.td"

.. rubric:: lbdex/Chapter8_2/Cpu0CondMov.td
.. literalinclude:: ../../../lib/Target/Cpu0/Cpu0CondMov.td


.. rubric:: lbdex/Chapter8_2/Cpu0ISelLowering.h
.. code-block:: c++

    SDValue lowerSELECT(SDValue Op, SelectionDAG &DAG) const;


.. rubric:: lbdex/Chapter8_2/Cpu0ISelLowering.cpp
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
Let's run Chapter8_2 with ch8_3.cpp to get the following result. 
Again, the cpu032II use **slt** instead of **cmp** has a little improved in 
instructions number.

.. code-block:: bash

  114-37-150-209:InputFiles Jonathan$ ~/llvm/test/cmake_debug_build/bin/Debug/llc 
  -march=cpu0 -mcpu=cpu032I -relocation-model=static -filetype=asm ch8_3.bc -debug 
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
  -march=cpu0 -mcpu=cpu032I -relocation-model=static -filetype=asm ch8_3.bc -o -
    .section .mdebug.abi32
    .previous
    .file "ch8_3.bc"
    .text
    .globl  _Z8select_1v
    .align  2
    .type _Z8select_1v,@function
    .ent  _Z8select_1v            # @_Z8select_1v
  _Z8select_1v:
    .frame  $sp,0,$lr
    .mask   0x00000000,0
    .set  noreorder
    .set  nomacro
  # BB#0:
    lui $2, %hi(a)
    addiu $2, $2, %lo(a)
    ld  $2, 0($2)
    lui $3, %hi(b)
    addiu $3, $3, %lo(b)
    ld  $3, 0($3)
    cmp $sw, $2, $3
    andi  $3, $sw, 1
    addiu $2, $zero, 2
    addiu $4, $zero, 1
    movn  $2, $4, $3
    ret $lr
    .set  macro
    .set  reorder
    .end  _Z8select_1v
  $tmp0:
    .size _Z8select_1v, ($tmp0)-_Z8select_1v
  
    .globl  _Z8select_2v
    .align  2
    .type _Z8select_2v,@function
    .ent  _Z8select_2v            # @_Z8select_2v
  _Z8select_2v:
    .frame  $sp,0,$lr
    .mask   0x00000000,0
    .set  noreorder
    .set  nomacro
  # BB#0:
    lui $2, %hi(a)
    addiu $2, $2, %lo(a)
    ld  $3, 0($2)
    addiu $2, $zero, 1
    addiu $4, $zero, 3
    movz  $2, $4, $3
    ret $lr
    .set  macro
    .set  reorder
    .end  _Z8select_2v
  $tmp1:
    .size _Z8select_2v, ($tmp1)-_Z8select_2v
  
    .type a,@object               # @a
    .data
    .globl  a
    .align  2
  a:
    .4byte  1                       # 0x1
    .size a, 4
  
    .type b,@object               # @b
    .globl  b
    .align  2
  b:
    .4byte  2                       # 0x2
    .size b, 4


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


Compare to the non-optimize version (clang -O0) which don't use conditional move 
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
  -c ch8_3.cpp -emit-llvm -o ch8_3.bc
  
  114-37-150-209:InputFiles Jonathan$ ~/llvm/test/cmake_debug_build/bin/Debug/
  llvm-dis ch8_3.bc -o -
  ...
  define i32 @_Z8select_1v() #0 {
    %1 = alloca i32, align 4
    %c = alloca i32, align 4
    store i32 0, i32* %c, align 4
    %2 = load volatile i32* @a, align 4
    %3 = load volatile i32* @b, align 4
    %4 = icmp slt i32 %2, %3
    br i1 %4, label %5, label %6
  
  ; <label>:5                                       ; preds = %0
    store i32 1, i32* %1
    br label %7
  
  ; <label>:6                                       ; preds = %0
    store i32 2, i32* %1
    br label %7
  
  ; <label>:7                                       ; preds = %6, %5
    %8 = load i32* %1
    ret i32 %8
  }
  ...
  
  114-37-150-209:InputFiles Jonathan$ ~/llvm/test/cmake_debug_build/bin/Debug/llc 
  -march=cpu0 -mcpu=cpu032II -relocation-model=static -filetype=asm ch8_3.bc -o -
    .section .mdebug.abi32
    .previous
    .file "ch8_3.bc"
    .text
    .globl  _Z8select_1v
    .align  2
    .type _Z8select_1v,@function
    .ent  _Z8select_1v            # @_Z8select_1v
  _Z8select_1v:
    .frame  $sp,8,$lr
    .mask   0x00000000,0
    .set  noreorder
    .set  nomacro
  # BB#0:
    addiu $sp, $sp, -8
    addiu $2, $zero, 0
    st  $2, 0($sp)
    lui $2, %hi(a)
    addiu $2, $2, %lo(a)
    ld  $2, 0($2)
    lui $3, %hi(b)
    addiu $3, $3, %lo(b)
    ld  $3, 0($3)
    cmp $sw, $2, $3
    jge $sw, $BB0_2
  # BB#1:
    addiu $2, $zero, 1
    st  $2, 4($sp)
    jmp $BB0_3
  $BB0_2:
    addiu $2, $zero, 2
    st  $2, 4($sp)
  $BB0_3:
    ld  $2, 4($sp)
    addiu $sp, $sp, 8
    ret $lr
    ...


RISC CPU knowledge
-------------------

As mentioned in the previous section, cpu0 is a RISC (Reduced Instruction Set 
Computer) CPU with 3 stages of pipeline. 
RISC CPU is full in world. 
Even the X86 of CISC (Complex Instruction Set Computer) is RISC inside. 
(It translate CISC instruction into micro-instruction which do pipeline as 
RISC). Knowledge with RISC will make you satisfied in compiler design. 
List these two excellent books we have read which include the real RISC CPU 
knowledge needed for reference. 
Sure, there are many books in Computer Architecture, and some of them contain 
real RISC CPU knowledge needed, but these two are what we have read.

Computer Organization and Design: The Hardware/Software Interface (The Morgan 
Kaufmann Series in Computer Architecture and Design)

Computer Architecture: A Quantitative Approach (The Morgan Kaufmann Series in 
Computer Architecture and Design) 

The book of “Computer Organization and Design: The Hardware/Software Interface” 
(there are 4 editions until the book is written) is for the introduction 
(simple). 
“Computer Architecture: A Quantitative Approach” (there are 5 editions until 
the book is written) is more complicate and deep in CPU architecture. 

Above two books use Mips CPU as example since Mips is more RISC-like than other 
market CPUs. ARM serials of CPU dominate the embedded market especially in 
mobile phone and other portable devices. The following book is good which I am 
reading now.

ARM System Developer's Guide: Designing and Optimizing System Software 
(The Morgan Kaufmann Series in Computer Architecture and Design).



.. [#] On a platform with cache and DRAM, the cache miss cost serveral tens 
       time of instruction cycle. The compiler engineers work in the vendor of 
       platform solution spend much effort try to reduce the cache miss for 
       speed. Reduce code size will cut down the cache miss frequency too.

.. [#] http://llvm.org/docs/WritingAnLLVMBackend.html#expand

.. [#Quantitative] See book Computer Architecture: A Quantitative Approach (The Morgan 
       Kaufmann Series in Computer Architecture and Design) 
