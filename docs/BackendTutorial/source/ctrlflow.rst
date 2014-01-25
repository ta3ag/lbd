.. _sec-controlflow:

Control flow statements
=======================

This chapter illustrates the corresponding IR for control flow statements, like 
**“if else”**, **“while”** and **“for”** loop statements in C, and how to 
translate these control flow statements of llvm IR into Cpu0 instructions. 

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
Less Than, **“sle”** stands for Set Less Equal. 
Run version Chapter8_1/ with ``llc  -view-isel-dags`` or ``-debug`` option, you 
can see it has translated **if** statement into 
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

For the first addiu instruction as above which move Constant<c> into register, 
we have defined it before by the following code,

.. rubric:: lbdex/Chapter3_5/Cpu0InstrInfo.td
.. code-block:: c++

    // Small immediates
    def : Pat<(i32 immSExt16:$in),
              (ADDiu ZERO, imm:$in)>;
    
    // Arbitrary immediates
    def : Pat<(i32 imm:$imm),
          (ORi (LUi (HI16 imm:$imm)), (LO16 imm:$imm))>;

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
The other two Cpu0 instructions translation is more complicate than simple 
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

Above definition support (setne RC:$lhs, RC:$rhs) register to register compare. 
There are other compare pattern like, seteq, setlt, ... . In addition to seteq, 
setne, ..., we define setueq, setune, ...,  by reference Mips code even though we 
didn't find how setune came from. 
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
    addiu $r1, $r2, 3   // $r1 register never be allocated to $SW
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
    cmake_debug_build/bin/Debug/llc -march=cpu0 -relocation-model=pic -filetype=obj 
    ch8_1_1.bc -o ch8_1_1.cpu0.o

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

Cpu0 is for teaching purpose and didn't consider the performance with design. 
In reality, the conditional branch is important in performance of CPU design. 
According bench mark information, every 7 instructions will meet 1 branch 
instruction in average. 
Cpu0 take 2 instructions for conditional branch, (jne(cmp...)), while Mips use 
one instruction (bne).

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
    /// Conditional Branch
	class CBranch24<bits<8> op, string instr_asm, RegisterClass RC,
					   list<Register> UseRegs>:
	  FJ<op, (outs), (ins RC:$ra, brtarget:$addr),
				 !strconcat(instr_asm, "\t$addr"),
				 [], IIBranch> {
	  let isBranch = 1;
	  let isTerminator = 1;
	  let hasDelaySlot = 0;
	  let neverHasSideEffects = 1;
	}
	
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
    def JMP     : UncondBranch<0x36, "jmp">;
    ...
    // brcond patterns
    multiclass BrcondPats<RegisterClass RC, Instruction JEQOp, 
      Instruction JNEOp, Instruction JLTOp, Instruction JGTOp, 
      Instruction JLEOp, Instruction JGEOp, Instruction CMPOp, 
      Register ZEROReg> {          
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
              (JLEOp (CMPOp RC:$rhs, RC:$lhs), bb:$dst)>;
    def : Pat<(brcond (i32 (setule RC:$lhs, RC:$rhs)), bb:$dst),
              (JLEOp (CMPOp RC:$rhs, RC:$lhs), bb:$dst)>;
    def : Pat<(brcond (i32 (setge RC:$lhs, RC:$rhs)), bb:$dst),
              (JGEOp (CMPOp RC:$lhs, RC:$rhs), bb:$dst)>;
    def : Pat<(brcond (i32 (setuge RC:$lhs, RC:$rhs)), bb:$dst),
              (JGEOp (CMPOp RC:$lhs, RC:$rhs), bb:$dst)>;
    
    def : Pat<(brcond RC:$cond, bb:$dst),
              (JNEOp (CMPOp RC:$cond, ZEROReg), bb:$dst)>;
    }
    
    defm : BrcondPats<CPURegs, JEQ, JNE, JLT, JGT, JLE, JGE, CMP, ZERO>;

The ch8_1_2.cpp is for **“nest if”** test. The ch8_1_3.cpp is the 
**“for loop”** as well as **“while loop”**, **“continue”**, **“break”**, 
**“goto”** test. The ch8_1_4.cpp is for **“goto”** test.
You can run with them if you like to test more.


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

