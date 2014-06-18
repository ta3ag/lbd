.. _sec-funccall:

Function call
==============

The subroutine/function call of backend code translation is supported in this 
chapter. 
A lots of code needed in function call. We break it down according llvm 
supplied interface for easy to explanation. 
This chapter start from introducing the Mips stack frame structure since we 
borrow many part of ABI from it. 
Although each CPU has it's own ABI, most of RISC CPUs ABI are similar. 
In addition to support fixed number of arguments function call, cpu0 also 
upport variable number of arguments since C/C++ support this feature. 
Supply Mips ABI and assemble language manual on internet link in this chapter 
for your reference. 
The section “4.5 DAG Lowering” of tricore_llvm.pdf contains some knowledge 
about Lowering process. Section “4.5.1 Calling Conventions” of tricore_llvm.pdf 
is the related materials you can reference.

This chapter is more complicate than any of the previous chapter. 
It include stack frame and the related ABI support. 
If you have problem in reading the stack frame illustrated in the first three 
sections of this chapter, you can read the appendix B of “Procedure Call 
Convention” of book “Computer Organization and Design” which listed in 
section “RISC CPU knowledge” of chapter “Control flow statement” [#]_, 
“Run Time Memory” of compiler book, or “Function Call Sequence”  and 
“Stack Frame” of Mips ABI.

Mips stack frame
-----------------

The first thing for design the cpu0 function call is deciding how to pass 
arguments in function call. There are two options. 
The first is pass arguments all in stack. 
Second is pass arguments in the registers which are reserved for function 
arguments, and put the other arguments in stack if it over the number of 
registers reserved for function call. For example, Mips pass the first 4 
arguments in register $a0, $a1, $a2, $a3, and the other arguments in stack 
if it over 4 arguments. :num:`Figure #funccall-f1` is the Mips stack frame.

.. _funccall-f1:
.. figure:: ../Fig/funccall/1.png
    :height: 531 px
    :width: 688 px
    :scale: 100 %
    :align: center

    Mips stack frame
    
Run ``llc -march=mips`` for ch9_1.bc, you will get the following result. 
See comment **"//"**.

.. rubric:: lbdex/InputFiles/ch9_1.cpp
.. literalinclude:: ../lbdex/InputFiles/ch9_1.cpp
    :start-after: /// start

.. code-block:: bash

  118-165-78-230:InputFiles Jonathan$ clang -target mips-unknown-linux-gnu -c 
  ch9_1.cpp -emit-llvm -o ch9_1.bc
  118-165-78-230:InputFiles Jonathan$ /Users/Jonathan/llvm/test/cmake_debug_build/
  bin/Debug/llc -march=mips -relocation-model=pic -filetype=asm ch9_1.bc -o 
  ch9_1.mips.s
  118-165-78-230:InputFiles Jonathan$ cat ch9_1.mips.s 
    .section .mdebug.abi32
    .previous
    .file "ch9_1.bc"
    .text
    .globl  _Z5sum_iiiiiii
    .align  2
    .type _Z5sum_iiiiiii,@function
    .set  nomips16                # @_Z5sum_iiiiiii
    .ent  _Z5sum_iiiiiii
  _Z5sum_iiiiiii:
    .cfi_startproc
    .frame  $sp,32,$ra
    .mask   0x00000000,0
    .fmask  0x00000000,0
    .set  noreorder
    .set  nomacro
    .set  noat
  # BB#0:
    addiu $sp, $sp, -32
  $tmp1:
    .cfi_def_cfa_offset 32
    sw  $4, 28($sp)
    sw  $5, 24($sp)
    sw  $t9, 20($sp)
    sw  $7, 16($sp)
    lw  $1, 48($sp) // load argument 5
    sw  $1, 12($sp)
    lw  $1, 52($sp) // load argument 6
    sw  $1, 8($sp)
    lw  $2, 24($sp)
    lw  $3, 28($sp)
    addu  $2, $3, $2
    lw  $3, 20($sp)
    addu  $2, $2, $3
    lw  $3, 16($sp)
    addu  $2, $2, $3
    lw  $3, 12($sp)
    addu  $2, $2, $3
    addu  $2, $2, $1
    sw  $2, 4($sp)
    jr  $ra
    addiu $sp, $sp, 32
    .set  at
    .set  macro
    .set  reorder
    .end  _Z5sum_iiiiiii
  $tmp2:
    .size _Z5sum_iiiiiii, ($tmp2)-_Z5sum_iiiiiii
    .cfi_endproc
  
    .globl  main
    .align  2
    .type main,@function
    .set  nomips16                # @main
    .ent  main
  main:
    .cfi_startproc
    .frame  $sp,40,$ra
    .mask   0x80000000,-4
    .fmask  0x00000000,0
    .set  noreorder
    .set  nomacro
    .set  noat
  # BB#0:
    lui $2, %hi(_gp_disp)
    addiu $2, $2, %lo(_gp_disp)
    addiu $sp, $sp, -40
  $tmp5:
    .cfi_def_cfa_offset 40
    sw  $ra, 36($sp)            # 4-byte Folded Spill
  $tmp6:
    .cfi_offset 31, -4
    addu  $gp, $2, $25
    sw  $zero, 32($sp)
    addiu $1, $zero, 6
    sw  $1, 20($sp) // Save argument 6 to 20($sp)
    addiu $1, $zero, 5
    sw  $1, 16($sp) // Save argument 5 to 16($sp)
    lw  $25, %call16(_Z5sum_iiiiiii)($gp)
    addiu $4, $zero, 1    // Pass argument 1 to $4 (=$a0)
    addiu $5, $zero, 2    // Pass argument 2 to $5 (=$a1)
    addiu $t9, $zero, 3
    jalr  $25
    addiu $7, $zero, 4
    sw  $2, 28($sp)
    lw  $ra, 36($sp)            # 4-byte Folded Reload
    jr  $ra
    addiu $sp, $sp, 40
    .set  at
    .set  macro
    .set  reorder
    .end  main
  $tmp7:
    .size main, ($tmp7)-main
    .cfi_endproc


From the mips assembly code generated as above, we know it save the first 4 
arguments to $a0..$a3 and last 2 arguments to 16($sp) and 20($sp). 
:num:`Figure #funccall-f2` is the arguments location for example code 
ch9_1.cpp. 
It load argument 5 from 48($sp) in sum_i() since the argument 5 is saved to 
16($sp) in main(). 
The stack size of sum_i() is 32, so 16+32($sp) is the location of incoming 
argument 5.

.. _funccall-f2:
.. figure:: ../Fig/funccall/2.png
    :height: 577 px
    :width: 740 px
    :scale: 100 %
    :align: center

    Mips arguments location in stack frame


The 007-2418-003.pdf in [#]_ is the Mips assembly language manual. 
[#]_ is Mips Application Binary Interface which include the 
:num:`Figure #funccall-f1`.

Load incoming arguments from stack frame
-----------------------------------------

From last section, to support function call, we need implementing the arguments 
pass mechanism with stack frame. Before do that, let's run the old version of 
code Chapter8_2/ with ch9_1.cpp and see what happens.

.. code-block:: bash

  118-165-79-31:InputFiles Jonathan$ /Users/Jonathan/llvm/test/
  cmake_debug_build/bin/Debug/llc -march=cpu0 -relocation-model=pic -filetype=asm 
  ch9_1.bc -o ch9_1.cpu0.s
  Assertion failed: (InVals.size() == Ins.size() && "LowerFormalArguments didn't 
  emit the correct number of values!"), function LowerArguments, file /Users/
  Jonathan/llvm/test/src/lib/CodeGen/SelectionDAG/
  SelectionDAGBuilder.cpp, ...
  ...
  0.  Program arguments: /Users/Jonathan/llvm/test/cmake_debug_build/
  bin/Debug/llc -march=cpu0 -relocation-model=pic -filetype=asm ch9_1.bc -o 
  ch9_1.cpu0.s 
  1.  Running pass 'Function Pass Manager' on module 'ch9_1.bc'.
  2.  Running pass 'CPU0 DAG->DAG Pattern Instruction Selection' on function 
  '@_Z5sum_iiiiiii'
  Illegal instruction: 4

Since Chapter8_2/ define the LowerFormalArguments() with empty, we get the error 
message as above. 
Before define LowerFormalArguments(), we have to choose how to pass arguments 
in function call. We choose pass arguments all in stack frame. 
We don't reserve any dedicated register for arguments passing since cpu0 has 
only 16 registers while Mips has 32 registers. Cpu0CallingConv.td is defined 
for cpu0 passing rule as follows,

.. rubric:: lbdex/Chapter9_1/Cpu0CallingConv.td
.. code-block:: c++
  
  //===----------------------------------------------------------------------===//
  // Cpu0 EABI Calling Convention
  //===----------------------------------------------------------------------===//
  
  def CC_Cpu0EABI : CallingConv<[
    // Promote i8/i16 arguments to i32.
    CCIfType<[i8, i16], CCPromoteToType<i32>>,
    // Integer values get stored in stack slots that are 4 bytes in
    // size and 4-byte aligned.
    CCIfType<[i32], CCAssignToStack<4, 4>>
  ]>;
  
  
  //===----------------------------------------------------------------------===//
  // Cpu0 Calling Convention Dispatch
  //===----------------------------------------------------------------------===//
  
  def CC_Cpu0 : CallingConv<[
    CCDelegateTo<CC_Cpu0EABI>
  ]>;


As above, CC_Cpu0 is the cpu0 Calling Convention which delegate to CC_Cpu0EABI. 
The reason we don't define the Calling Convention directly in CC_Cpu0 is that 
a real general CPU like Mips can has several Calling Convention. 
Combine with the mechanism of "section Target Registration" [#]_ which llvm 
supplied, we can use different Calling Convention in different target. 
Although cpu0 only has a Calling Convention right now, define with a dedicate 
Call Convention name (CC_Cpu0EABI in this example) is a better solution for 
system expand. CC_Cpu0EABI as above, say it pass arguments in stack frame, 
and allow Cpu0 have another ABI for passing first 4 registers in future.

Function LowerFormalArguments() charge function incoming arguments creation. 
We define it as follows,

.. rubric:: lbdex/Chapter9_1/Cpu0ISelLowering.cpp
.. code-block:: c++

  SDValue
  Cpu0TargetLowering::LowerCall(TargetLowering::CallLoweringInfo &CLI,
                                SmallVectorImpl<SDValue> &InVals) const {
    return CLI.Chain;
  }
  ...

.. literalinclude:: ../../../lib/Target/Cpu0/Cpu0ISelLowering.cpp
    :start-after: } // lbd document - mark - ReadByValArg
    :end-before: unsigned NumWords = (Flags.getByValSize() + 3) / 4;
.. literalinclude:: ../../../lib/Target/Cpu0/Cpu0ISelLowering.cpp
    :start-after: &*FuncArg);
    :end-before: #if 1 // Incomming. Without this, it will use $3 instead of $2 as return
.. literalinclude:: ../../../lib/Target/Cpu0/Cpu0ISelLowering.cpp
    :start-after: } // lbd document - mark - if (isVarArg)
    :end-before: //               Return Value Calling Convention Implementation

.. rubric:: lbdex/Chapter9_1/Cpu0ISelLowering.h
.. code-block:: c++

      virtual SDValue
        LowerCall(TargetLowering::CallLoweringInfo &CLI,
                  SmallVectorImpl<SDValue> &InVals) const;
    // LowerCall: outgoing arguments


Refresh "section Global variable" [#]_, we handled global 
variable translation by create the IR DAG in LowerGlobalAddress() first, and 
then finish the Instruction Selection according their corresponding machine 
instruction DAG in Cpu0InstrInfo.td. 
LowerGlobalAddress() is called when ``llc`` meet the global variable access. 
LowerFormalArguments() work in the same way. 
It is called when function is entered. 
It get incoming arguments information by CCInfo(CallConv,..., ArgLocs, ...) 
before enter **“for loop”**. In ch9_1.cpp, there are 6 arguments in sum_i(...) 
function call and we use the stack frame only for arguments passing without 
any arguments pass in registers. 
So ArgLocs.size() is 6, each argument information is in ArgLocs[i] and 
ArgLocs[i].isMemLoc() is true. 
In **“for loop”**, it creates each frame index object by LastFI = 
MFI->CreateFixedObject(ValVT.getSizeInBits()/8,VA.getLocMemOffset(), true) and 
FIN = DAG.getFrameIndex(LastFI, getPointerTy()). 
And then creates IR DAG load node and puts the load node into vector InVals by 
InVals.push_back(DAG.getLoad(ValVT, DL, Chain, FIN, 
MachinePointerInfo::getFixedStack(LastFI), false, false, false, 0)). 
Cpu0FI->setVarArgsFrameIndex(0) and Cpu0FI->setLastInArgFI(LastFI) are called 
when before and after above work. 
For example code ch9_1.cpp, LowerFormalArguments() 
will be called twice. First time is for sum_i() which will create 6 load DAG 
for 6 incoming arguments passing into this function. 
Second time is for main() which won't create any load DAG for no incoming 
argument passing into main(). 
In addition to LowerFormalArguments() which creates the load DAG, we need to 
define the loadRegFromStackSlot() to issue the machine instruction 
**“ld $r, offset($sp)”** to load incoming arguments from stack frame offset.
GetMemOperand(..., FI, ...) return the Memory location of the frame index 
variable, which is the offset.

.. rubric:: lbdex/Chapter9_1/Cpu0InstrInfo.cpp
.. literalinclude:: ../../../lib/Target/Cpu0/Cpu0InstrInfo.cpp
    :start-after: #include "Cpu0TargetMachine.h"
    :end-before: #include "llvm/CodeGen/MachineInstrBuilder.h"
.. literalinclude:: ../../../lib/Target/Cpu0/Cpu0InstrInfo.cpp
    :start-after: // lbd document - mark - copyPhysReg
    :end-before: //- st SrcReg, MMO(FI)
.. literalinclude:: ../../../lib/Target/Cpu0/Cpu0InstrInfo.cpp
    :start-after: // lbd document - mark - storeRegToStackSlot
    :end-before: MachineInstr*

.. rubric:: lbdex/Chapter9_1/Cpu0InstrInfo.h
.. code-block:: c++

    virtual void loadRegFromStackSlot(MachineBasicBlock &MBB,
                                      MachineBasicBlock::iterator MBBI,
                                      unsigned DestReg, int FrameIndex,
                                      const TargetRegisterClass *RC,
                                      const TargetRegisterInfo *TRI) const;

In addition to Calling Convention and LowerFormalArguments(), Chapter9_1/ add the 
following code for the instruction selection and printing of Cpu0 instructions 
**swi** (Software Interrupt), **jsub** and **jalr** (function call).
    
.. rubric:: lbdex/Chapter9_1/Cpu0InstrInfo.td
.. code-block:: c++

  def SDT_Cpu0JmpLink      : SDTypeProfile<0, 1, [SDTCisVT<0, iPTR>]>;
  ...
  // Call
  def Cpu0JmpLink : SDNode<"Cpu0ISD::JmpLink",SDT_Cpu0JmpLink,
                           [SDNPHasChain, SDNPOutGlue, SDNPOptInGlue,
                            SDNPVariadic]>;
  ...
  def calltarget  : Operand<iPTR> {
    let EncoderMethod = "getJumpTargetOpValue";
  }
  ...
  // Jump and Link (Call)
  let isCall=1, hasDelaySlot=0 in {
    class JumpLink<bits<8> op, string instr_asm>:
      FJ<op, (outs), (ins calltarget:$target, variable_ops),
         !strconcat(instr_asm, "\t$target"), [(Cpu0JmpLink imm:$target)],
         IIBranch> {
         }
    
    class JumpLinkReg<bits<8> op, string instr_asm,
                      RegisterClass RC>:
      FA<op, (outs), (ins RC:$rb, variable_ops),
         !strconcat(instr_asm, "\t$rb"), [(Cpu0JmpLink RC:$rb)], IIBranch> {
      let rc = 0;
      let ra = 14;
      let shamt = 0;
    }
  }
  ...
  /// Jump and Branch Instructions
  def SWI  : JumpLink<0x2a, "swi">;
  def JSUB : JumpLink<0x2b, "jsub">;
  ...
  def JALR    : JumpLinkReg<0x2e, "jalr", GPROut>;
  ...
  def : Pat<(Cpu0JmpLink (i32 tglobaladdr:$dst)),
            (JSUB tglobaladdr:$dst)>;
  def : Pat<(Cpu0JmpLink (i32 texternalsym:$dst)),
            (JSUB texternalsym:$dst)>;
  ...
    
.. rubric:: lbdex/Chapter9_1/InstPrinter/Cpu0InstPrinter.cpp
.. code-block:: c++

  static void printExpr(const MCExpr *Expr, raw_ostream &OS) {
    switch (Kind) {
    ...
    case MCSymbolRefExpr::VK_Cpu0_GOT_CALL:  OS << "%call16("; break;
    ...
    }
  ...
  }
    
.. rubric:: lbdex/Chapter9_1/Cpu0MCInstLower.cpp
.. code-block:: c++

  MCOperand Cpu0MCInstLower::LowerSymbolOperand(const MachineOperand &MO,
                                                MachineOperandType MOTy,
                                                unsigned Offset) const {
    ...
    case Cpu0II::MO_GOT_CALL:  Kind = MCSymbolRefExpr::VK_Cpu0_GOT_CALL; break;
    ...
    switch (MOTy) {
    ...
    case MachineOperand::MO_ExternalSymbol:
      Symbol = AsmPrinter.GetExternalSymbolSymbol(MO.getSymbolName());
      Offset += MO.getOffset();
      break;
      ...
    }
    ...
  }
  
  MCOperand Cpu0MCInstLower::LowerOperand(const MachineOperand& MO,
                                          unsigned offset) const {
    ...
    switch (MOTy) {
    ...
    case MachineOperand::MO_ExternalSymbol:
    case MachineOperand::MO_BlockAddress:
      return LowerSymbolOperand(MO, MOTy, offset);
    ...
   }
    ...
  }

.. rubric:: lbdex/Chapter9_1/MCTargetDesc/Cpu0AsmBackend.cpp
.. code-block:: c++

    case Cpu0::fixup_Cpu0_CALL16:

.. rubric:: lbdex/Chapter9_1/MCTargetDesc/Cpu0ELFObjectWriter.cpp
.. code-block:: c++

    case Cpu0::fixup_Cpu0_CALL16:
      Type = ELF::R_CPU0_CALL16;
      break;

.. rubric:: lbdex/Chapter9_1/MCTargetDesc/Cpu0FixupKinds.h
.. code-block:: c++

      // resulting in - R_CPU0_CALL16.
      fixup_Cpu0_CALL16,


.. rubric:: lbdex/Chapter9_1/MCTargetDesc/Cpu0MCCodeEmitter.cpp
.. code-block:: c++

  unsigned Cpu0MCCodeEmitter::
  getJumpTargetOpValue(const MCInst &MI, unsigned OpNo,
                       SmallVectorImpl<MCFixup> &Fixups) const {
    ...
    if (Opcode == Cpu0::JSUB || Opcode == Cpu0::JMP)
      Fixups.push_back(MCFixup::Create(0, Expr,
                                       MCFixupKind(Cpu0::fixup_Cpu0_PC24)));
    else if (Opcode == Cpu0::SWI)
      Fixups.push_back(MCFixup::Create(0, Expr,
                                       MCFixupKind(Cpu0::fixup_Cpu0_24)));
    ...
  } // lbd document - mark - getJumpTargetOpValue
  ...
  unsigned Cpu0MCCodeEmitter::
  getMachineOpValue(const MCInst &MI, const MCOperand &MO,
                    SmallVectorImpl<MCFixup> &Fixups) const {
  ...
    switch(cast<MCSymbolRefExpr>(Expr)->getKind()) {
    ...
    case MCSymbolRefExpr::VK_Cpu0_GOT_CALL:
      FixupKind = Cpu0::fixup_Cpu0_CALL16;
      break;
    ...
    }
  ...
  }

  unsigned Cpu0MCCodeEmitter::
  getMachineOpValue(const MCInst &MI, const MCOperand &MO,
                    SmallVectorImpl<MCFixup> &Fixups) const {
    ...
    case MCSymbolRefExpr::VK_Cpu0_GOT_CALL:
      FixupKind = Cpu0::fixup_Cpu0_CALL16;
      break;
    ...
  }
 
.. rubric:: lbdex/Chapter9_1/Cpu0MachineFucntion.h
.. code-block:: c++

  class Cpu0FunctionInfo : public MachineFunctionInfo {
    ...
      /// VarArgsFrameIndex - FrameIndex for start of varargs area.
    int VarArgsFrameIndex;
  
    // Range of frame object indices.
    // InArgFIRange: Range of indices of all frame objects created during call to
    //               LowerFormalArguments.
    // OutArgFIRange: Range of indices of all frame objects created during call to
    //                LowerCall except for the frame object for restoring $gp.
    std::pair<int, int> InArgFIRange, OutArgFIRange;
    ...
    mutable int DynAllocFI; // Frame index of dynamically allocated stack area.
    ...
  
  public:
    Cpu0FunctionInfo(MachineFunction& MF)
    : ...
      VarArgsFrameIndex(0), InArgFIRange(std::make_pair(-1, 0)),
      OutArgFIRange(std::make_pair(-1, 0)), GPFI(0), DynAllocFI(0),
      ...
      {}
      
    bool isInArgFI(int FI) const {
      return FI <= InArgFIRange.first && FI >= InArgFIRange.second;
    }
    void setLastInArgFI(int FI) { InArgFIRange.second = FI; }
    
    void extendOutArgFIRange(int FirstFI, int LastFI) {
      if (!OutArgFIRange.second)
        // this must be the first time this function was called.
        OutArgFIRange.first = FirstFI;
      OutArgFIRange.second = LastFI;
    }
    
    int getGPFI() const { return GPFI; }
    void setGPFI(int FI) { GPFI = FI; }
    bool needGPSaveRestore() const { return getGPFI(); }
    bool isGPFI(int FI) const { return GPFI && GPFI == FI; }
    
    // The first call to this function creates a frame object for dynamically
    // allocated stack area.
    int getDynAllocFI() const {
      if (!DynAllocFI)
        DynAllocFI = MF.getFrameInfo()->CreateFixedObject(4, 0, true);
    
      return DynAllocFI;
    }
    bool isDynAllocFI(int FI) const { return DynAllocFI && DynAllocFI == FI; }
    ...
    int getVarArgsFrameIndex() const { return VarArgsFrameIndex; }
    void setVarArgsFrameIndex(int Index) { VarArgsFrameIndex = Index; }
    ...
  };


The SWI, JSUB and JALR defined in Cpu0InstrInfo.td as above all use Cpu0JmpLink
node. They are distinguishable since both SWI and JSUB use "imm" operand while
JALR use register operand. JSUB take the priority to match since we set the
following code in Cpu0InstrInfo.td.

.. rubric:: lbdex/Chapter9_1/Cpu0InstrInfo.td
.. code-block:: c++

  def : Pat<(Cpu0JmpLink (i32 tglobaladdr:$dst)),
            (JSUB tglobaladdr:$dst)>;
  def : Pat<(Cpu0JmpLink (i32 texternalsym:$dst)),
            (JSUB texternalsym:$dst)>;

The code tells TableGen generate pattern match pattern to match the "imm" for
"tglobaladdr" pattern first. If it fails then try to match "texternalsym" next.
The function you declared is "tglobaladdr", the function which implicit used by
llvm most are "texternalsym" such as "memcpy". The "memcpy" will be generated
when define a long string. The ch9_1_2.cpp is an example for generating "memcpy"
function call. It will be shown in next section with Chapter9_2 example code.
Even though SWI have no chance to be matched in C/C++ language. We define it 
for implementing assembly parser easier which introduced in Chapter 11. This
SWI definition will save us to implement the assembly parser for this
instruction. TableGen will generate information for SWI instruction in assembly
and ELF obj encode automatically. The Cpu0GenDAGISel.inc contains the TablGen
generated information about JSUB and JALR pattern match information as follows,

.. code-block:: c++

            /*SwitchOpcode*/ 74,  TARGET_VAL(Cpu0ISD::JmpLink),// ->734
  /*660*/     OPC_RecordNode,   // #0 = 'Cpu0JmpLink' chained node
  /*661*/     OPC_CaptureGlueInput,
  /*662*/     OPC_RecordChild1, // #1 = $target
  /*663*/     OPC_Scope, 57, /*->722*/ // 2 children in Scope
  /*665*/       OPC_MoveChild, 1,
  /*667*/       OPC_SwitchOpcode /*3 cases */, 22,  TARGET_VAL(ISD::Constant),
  // ->693
  /*671*/         OPC_MoveParent,
  /*672*/         OPC_EmitMergeInputChains1_0,
  /*673*/         OPC_EmitConvertToTarget, 1,
  /*675*/         OPC_Scope, 7, /*->684*/ // 2 children in Scope
  /*677*/           OPC_MorphNodeTo, TARGET_VAL(Cpu0::SWI), 0|OPFL_Chain|
  OPFL_GlueInput|OPFL_GlueOutput|OPFL_Variadic1,
                        0/*#VTs*/, 1/*#Ops*/, 2, 
                    // Src: (Cpu0JmpLink (imm:iPTR):$target) - Complexity = 6
                    // Dst: (SWI (imm:iPTR):$target)
  /*684*/         /*Scope*/ 7, /*->692*/
  /*685*/           OPC_MorphNodeTo, TARGET_VAL(Cpu0::JSUB), 0|OPFL_Chain|
  OPFL_GlueInput|OPFL_GlueOutput|OPFL_Variadic1,
                        0/*#VTs*/, 1/*#Ops*/, 2, 
                    // Src: (Cpu0JmpLink (imm:iPTR):$target) - Complexity = 6
                    // Dst: (JSUB (imm:iPTR):$target)
  /*692*/         0, /*End of Scope*/
                /*SwitchOpcode*/ 11,  TARGET_VAL(ISD::TargetGlobalAddress),// ->707
  /*696*/         OPC_CheckType, MVT::i32,
  /*698*/         OPC_MoveParent,
  /*699*/         OPC_EmitMergeInputChains1_0,
  /*700*/         OPC_MorphNodeTo, TARGET_VAL(Cpu0::JSUB), 0|OPFL_Chain|
  OPFL_GlueInput|OPFL_GlueOutput|OPFL_Variadic1,
                      0/*#VTs*/, 1/*#Ops*/, 1, 
                  // Src: (Cpu0JmpLink (tglobaladdr:i32):$dst) - Complexity = 6
                  // Dst: (JSUB (tglobaladdr:i32):$dst)
                /*SwitchOpcode*/ 11,  TARGET_VAL(ISD::TargetExternalSymbol),// ->721
  /*710*/         OPC_CheckType, MVT::i32,
  /*712*/         OPC_MoveParent,
  /*713*/         OPC_EmitMergeInputChains1_0,
  /*714*/         OPC_MorphNodeTo, TARGET_VAL(Cpu0::JSUB), 0|OPFL_Chain|
  OPFL_GlueInput|OPFL_GlueOutput|OPFL_Variadic1,
                      0/*#VTs*/, 1/*#Ops*/, 1, 
                  // Src: (Cpu0JmpLink (texternalsym:i32):$dst) - Complexity = 6
                  // Dst: (JSUB (texternalsym:i32):$dst)
                0, // EndSwitchOpcode
  /*722*/     /*Scope*/ 10, /*->733*/
  /*723*/       OPC_CheckChild1Type, MVT::i32,
  /*725*/       OPC_EmitMergeInputChains1_0,
  /*726*/       OPC_MorphNodeTo, TARGET_VAL(Cpu0::JALR), 0|OPFL_Chain|
  OPFL_GlueInput|OPFL_GlueOutput|OPFL_Variadic1,
                    0/*#VTs*/, 1/*#Ops*/, 1, 
                // Src: (Cpu0JmpLink CPURegs:i32:$rb) - Complexity = 3
                // Dst: (JALR CPURegs:i32:$rb)
  /*733*/     0, /*End of Scope*/


After above changes, you can run Chapter9_1/ with ch9_1.cpp and see what happens 
in the following,

.. code-block:: bash

  118-165-79-83:InputFiles Jonathan$ /Users/Jonathan/llvm/test/
  cmake_debug_build/bin/Debug/llc -march=cpu0 -relocation-model=pic -filetype=asm 
  ch9_1.bc -o ch9_1.cpu0.s
  Assertion failed: ((CLI.IsTailCall || InVals.size() == CLI.Ins.size()) && 
  "LowerCall didn't emit the correct number of values!"), function LowerCallTo, 
  file /Users/Jonathan/llvm/test/src/lib/CodeGen/SelectionDAG/SelectionDAGBuilder.
  cpp, ...
  ...
  0.  Program arguments: /Users/Jonathan/llvm/test/cmake_debug_build/
  bin/Debug/llc -march=cpu0 -relocation-model=pic -filetype=asm ch9_1.bc -o 
  ch9_1.cpu0.s 
  1.  Running pass 'Function Pass Manager' on module 'ch9_1.bc'.
  2.  Running pass 'CPU0 DAG->DAG Pattern Instruction Selection' on function 
  '@main'
  Illegal instruction: 4
  
Now, the LowerFormalArguments() has the correct number, but LowerCall() has not  
the correct number of values!


Store outgoing arguments to stack frame
----------------------------------------

:num:`Figure #funccall-f2` depicts two steps to take care arguments passing. 
One is store outgoing arguments in caller function, and the other is load 
incoming arguments in callee function. 
We defined LowerFormalArguments() for **“load incoming arguments”** in callee 
function last section. 
Now, we will finish **“store outgoing arguments”** in caller function. 
LowerCall() is responsible to do this. The implementation as follows,

.. rubric:: lbdex/Chapter9_2/Cpu0ISelLowering.cpp
.. literalinclude:: ../../../lib/Target/Cpu0/Cpu0ISelLowering.cpp
    :start-after: // lbd document - mark - WriteByValArg
    :end-before: if (Cpu0FI->needGPSaveRestore())
.. literalinclude:: ../../../lib/Target/Cpu0/Cpu0ISelLowering.cpp
    :start-after: MFI->setObjectOffset(Cpu0FI->getGPFI(), NextStackOffset);
    :end-before: WriteByValArg(ByValChain, Chain, DL, RegsToPass, MemOpChains, LastFI,
.. literalinclude:: ../../../lib/Target/Cpu0/Cpu0ISelLowering.cpp
    :start-after: Subtarget->isLittle());
    :end-before: //===----------------------------------------------------------------------===//

.. rubric:: lbdex/Chapter9_2/Cpu0ISelLowering.h
.. literalinclude:: ../../../lib/Target/Cpu0/Cpu0ISelLowering.h
    :start-after: const Cpu0Subtarget *Subtarget;
    :end-before: // Lower Operand specifics

Just like load incoming arguments from stack frame, we call 
CCInfo(CallConv,..., ArgLocs, ...) to get outgoing arguments information before 
enter **“for loop”** and set stack alignment with 8 bytes. 
They're almost same in **“for loop”** with LowerFormalArguments(), except 
LowerCall() creates store DAG vector instead of load DAG vector. 
After the **“for loop”**, it create **“ld $t9, %call16(_Z5sum_iiiiiii)($gp)”** 
and jalr $t9 for calling subroutine (the $6 is $t9) in PIC mode.
DAG.getCALLSEQ_START() and DAG.getCALLSEQ_END() are set before the 
**“for loop”** and after call subroutine, respectively, they insert 
CALLSEQ_START, CALLSEQ_END, and translate them into pseudo machine instructions 
!ADJCALLSTACKDOWN, !ADJCALLSTACKUP later according Cpu0InstrInfo.td definition 
as follows.

.. rubric:: lbdex/Chapter9_2/Cpu0InstrInfo.td
.. code-block:: c++

  def SDT_Cpu0CallSeqStart : SDCallSeqStart<[SDTCisVT<0, i32>]>;
  def SDT_Cpu0CallSeqEnd   : SDCallSeqEnd<[SDTCisVT<0, i32>, SDTCisVT<1, i32>]>;
  ...
  // These are target-independent nodes, but have target-specific formats.
  def callseq_start : SDNode<"ISD::CALLSEQ_START", SDT_Cpu0CallSeqStart,
                             [SDNPHasChain, SDNPOutGlue]>;
  def callseq_end   : SDNode<"ISD::CALLSEQ_END", SDT_Cpu0CallSeqEnd,
                             [SDNPHasChain, SDNPOptInGlue, SDNPOutGlue]>;
  ...
  //===----------------------------------------------------------------------===//
  // Pseudo instructions
  //===----------------------------------------------------------------------===//
    
  // As stack alignment is always done with addiu, we need a 16-bit immediate
  let Defs = [SP], Uses = [SP] in {
  def ADJCALLSTACKDOWN : Cpu0Pseudo<(outs), (ins uimm16:$amt),
                                    "!ADJCALLSTACKDOWN $amt",
                                    [(callseq_start timm:$amt)]>;
  def ADJCALLSTACKUP   : Cpu0Pseudo<(outs), (ins uimm16:$amt1, uimm16:$amt2),
                                    "!ADJCALLSTACKUP $amt1",
                                    [(callseq_end timm:$amt1, timm:$amt2)]>;
  }
  

Like load incoming arguments, we need to implement storeRegToStackSlot() for 
store outgoing arguments to stack frame offset.
    
.. rubric:: lbdex/Chapter9_2/Cpu0InstrInfo.cpp
.. literalinclude:: ../../../lib/Target/Cpu0/Cpu0InstrInfo.cpp
    :start-after:  // lbd document - mark - GetMemOperand
    :end-before: // lbd document - mark - before loadRegFromStackSlot

.. rubric:: lbdex/Chapter9_2/Cpu0InstrInfo.h
.. literalinclude:: ../../../lib/Target/Cpu0/Cpu0InstrInfo.h
    :start-after:  unsigned GetInstSizeInBytes(const MachineInstr *MI) const;
    :end-before: loadRegFromStackSlot


.. code-block:: bash

  118-165-78-230:InputFiles Jonathan$ /Users/Jonathan/llvm/test/cmake_debug_build/
  bin/Debug/llc -march=cpu0 -relocation-model=pic -filetype=asm ch9_1.bc -o 
  ch9_1.cpu0.s
  118-165-78-230:InputFiles Jonathan$ cat ch9_1.cpu0.s 
  	.section .mdebug.abi32
  	.previous
  	.file	"ch9_1.bc"
  	.text
  	.globl	_Z5sum_iiiiiii
  	.align	2
  	.type	_Z5sum_iiiiiii,@function
  	.ent	_Z5sum_iiiiiii          # @_Z5sum_iiiiiii
  _Z5sum_iiiiiii:
  	.cfi_startproc
  	.frame	$sp,32,$lr
  	.mask 	0x00000000,0
  	.set	noreorder
  	.cpload	$t9
  	.set	nomacro
  # BB#0:
  	addiu	$sp, $sp, -32
  $tmp1:
  	.cfi_def_cfa_offset 32
  	ld	$2, 32($sp)
  	st	$2, 28($sp)
  	ld	$2, 36($sp)
  	st	$2, 24($sp)
  	ld	$2, 40($sp)
  	st	$2, 20($sp)
  	ld	$2, 44($sp)
  	st	$2, 16($sp)
  	ld	$2, 48($sp)
  	st	$2, 12($sp)
  	ld	$2, 52($sp)
  	st	$2, 8($sp)
  	addiu	$3, $zero, %got_hi(gI)
  	shl	$3, $3, 16
  	addu	$3, $3, $gp
  	ld	$3, %got_lo(gI)($3)
  	ld	$3, 0($3)
  	ld	$4, 28($sp)
  	addu	$3, $3, $4
  	ld	$4, 24($sp)
  	addu	$3, $3, $4
  	ld	$4, 20($sp)
  	addu	$3, $3, $4
  	ld	$4, 16($sp)
  	addu	$3, $3, $4
  	ld	$4, 12($sp)
  	addu	$3, $3, $4
  	addu	$2, $3, $2
  	st	$2, 4($sp)
  	addiu	$sp, $sp, 32
  	ret	$lr
  	.set	macro
  	.set	reorder
  	.end	_Z5sum_iiiiiii
  $tmp2:
  	.size	_Z5sum_iiiiiii, ($tmp2)-_Z5sum_iiiiiii
  	.cfi_endproc
  
  	.globl	main
  	.align	2
  	.type	main,@function
  	.ent	main                    # @main
  main:
  	.cfi_startproc
  	.frame	$sp,40,$lr
  	.mask 	0x00004000,-4
  	.set	noreorder
  	.cpload	$t9
  	.set	nomacro
  # BB#0:
  	addiu	$sp, $sp, -40
  $tmp5:
  	.cfi_def_cfa_offset 40
  	st	$lr, 36($sp)            # 4-byte Folded Spill
  $tmp6:
  	.cfi_offset 14, -4
  	addiu	$2, $zero, 0
  	st	$2, 32($sp)
  	!ADJCALLSTACKDOWN 24
  	addiu	$2, $zero, 6
  	st	$2, 60($sp)
  	addiu	$2, $zero, 5
  	st	$2, 56($sp)
  	addiu	$2, $zero, 4
  	st	$2, 52($sp)
  	addiu	$2, $zero, 3
  	st	$2, 48($sp)
  	addiu	$2, $zero, 2
  	st	$2, 44($sp)
  	addiu	$2, $zero, 1
  	st	$2, 40($sp)
  	ld	$t9, %call16(_Z5sum_iiiiiii)($gp)
  	jalr	$t9
  	!ADJCALLSTACKUP 24
  	st	$2, 28($sp)
  	ld	$lr, 36($sp)            # 4-byte Folded Reload
  	addiu	$sp, $sp, 40
  	ret	$lr
  	.set	macro
  	.set	reorder
  	.end	main
  $tmp7:
  	.size	main, ($tmp7)-main
  	.cfi_endproc
  
  	.type	gI,@object              # @gI
  	.data
  	.globl	gI
  	.align	2
  gI:
  	.4byte	100                     # 0x64
  	.size	gI, 4

The last section mentioned the "JSUB texternalsym" pattern. Run Chapter9_2 with
ch9_1_2.cpp to get the result as below. For long string, llvm call memcpy() to
initialize string (char str[81] = "Hello world" in this case). For short string,
the "call memcpy" is translated into "store with contant" in stages of
optimization.

.. rubric:: lbdex/InputFiles/ch9_1_2.cpp
.. literalinclude:: ../lbdex/InputFiles/ch9_1_2.cpp
    :start-after: /// start

.. code-block:: bash

  JonathantekiiMac:InputFiles Jonathan$ llvm-dis ch9_1_2.bc -o -
  ; ModuleID = 'ch9_1_2.bc'
  ...
  @_ZZ4mainE3str = private unnamed_addr constant [81 x i8] c"Hello world\00\00\00\
  00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00
  \00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\0
  0\00\00\00\00\00\00\00\00\00\00\00\00\00", align 1
  @_ZZ4mainE1s = private unnamed_addr constant [6 x i8] c"Hello\00", align 1
  
  ; Function Attrs: nounwind
  define i32 @main() #0 {
  entry:
    %retval = alloca i32, align 4
    %str = alloca [81 x i8], align 1
    store i32 0, i32* %retval
    %0 = bitcast [81 x i8]* %str to i8*
    call void @llvm.memcpy.p0i8.p0i8.i32(i8* %0, i8* getelementptr inbounds 
    ([81 x i8]* @_ZZ4mainE3str, i32 0, i32 0), i32 81, i32 1, i1 false)
    %1 = bitcast [6 x i8]* %s to i8*
    call void @llvm.memcpy.p0i8.p0i8.i32(i8* %1, i8* getelementptr inbounds 
    ([6 x i8]* @_ZZ4mainE1s, i32 0, i32 0), i32 6, i32 1, i1 false)
  
    ret i32 0
  }
  
  JonathantekiiMac:InputFiles Jonathan$ clang -target mips-unknown-linux-gnu -c 
  ch9_1_2.cpp -emit-llvm -o ch9_1_2.bc
  JonathantekiiMac:InputFiles Jonathan$ /Users/Jonathan/llvm/test/cmake_debug_build
  /bin/Debug/llc -march=cpu0 -relocation-model=static -filetype=asm ch9_1_2.bc -o -
    .section .mdebug.abi32
    ...
    addiu $2, $zero, 81
    st  $2, 128($fp)
    addiu $2, $zero, %hi($_ZZ4mainE3str)
    shl $2, $2, 16
    addiu $2, $2, %lo($_ZZ4mainE3str)
    st  $2, 124($fp)
    addiu $2, $fp, 28
    st  $2, 120($fp)
    jsub  memcpy
    !ADJCALLSTACKUP 16
    addiu $2, $zero, 28416  // 28416=0x6f00="o\0"
    sh  $2, 24($fp)
    addiu $2, $zero, 18533  // 18533=0x4865="He"
    shl $2, $2, 16
    ori $2, $2, 27756         // 27756=0x6c6c="ll"
    st  $2, 20($fp)
    ...
    .type $_ZZ4mainE3str,@object  # @_ZZ4mainE3str
    .section  .rodata,"a",@progbits
  $_ZZ4mainE3str:
    .asciz   "Hello world\000\000\000\000\000\000\000\000\000\000\000\000\000\000
    \000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\
    000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\0
    00\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
    .size $_ZZ4mainE3str, 81

The "call memcpy" for short string is optimized by llvm before "DAG->DAG Pattern 
Instruction Selection" stage and translate it into "store with contant" as
follows,

.. code-block:: bash

  JonathantekiiMac:InputFiles Jonathan$ /Users/Jonathan/llvm/test/cmake_debug_build
  /bin/Debug/llc -march=cpu0 -relocation-model=static -filetype=asm ch9_1_2.bc 
  -debug -o -
  
  Initial selection DAG: BB#0 'main:entry'
  SelectionDAG has 35 nodes:
    ...
          0x7fd909030810: <multiple use>
          0x7fd909030c10: i32 = Constant<1214606444>  // 1214606444=0x48656c6c="Hell"
  
          0x7fd909030910: <multiple use>
          0x7fd90902d810: <multiple use>
        0x7fd909030d10: ch = store 0x7fd909030810, 0x7fd909030c10, 0x7fd909030910, 
        0x7fd90902d810<ST4[%1]>
  
          0x7fd909030810: <multiple use>
          0x7fd909030e10: i16 = Constant<28416>      // 28416=0x6f00="o\0"
  
          ...
  
          0x7fd90902d810: <multiple use>
        0x7fd909031210: ch = store 0x7fd909030810, 0x7fd909030e10, 0x7fd909031010, 
        0x7fd90902d810<ST2[%1+4](align=4)>
    ...


The "isTailCall = false;" which set in LowerCall() of Cpu0ISelLowering.cpp 
meaning Cpu0 don't support tail call optimization at this moment. 
About tail call optimization please reference [#]_.


Fix issues
-----------

Run Chapter9_2/ with ch7_5.cpp to get the incorrect main return (return register 
$2 is not 0) as follows,

.. rubric:: lbdex/InputFiles/ch7_5.cpp
.. literalinclude:: ../lbdex/InputFiles/ch7_5.cpp
    :start-after: /// start

.. code-block:: bash

  118-165-78-31:InputFiles Jonathan$ clang -target mips-unknown-linux-gnu -c 
  ch7_5.cpp -emit-llvm -o ch7_5.bc
  118-165-78-31:InputFiles Jonathan$ /Users/Jonathan/llvm/test/cmake_debug_build/
  bin/Debug/llc -march=cpu0 -relocation-model=static -filetype=asm ch7_5.bc -o 
  ch7_5.cpu0.static.s
  118-165-78-31:InputFiles Jonathan$ cat ch7_5.cpu0.static.s 
    .section .mdebug.abi32
    .previous
    .file "ch7_5.bc"
    ...
    .cfi_startproc
    .frame  $sp,16,$lr
    .mask   0x00000000,0
    .set  noreorder
    .set  nomacro
  # BB#0:
    addiu $sp, $sp, -16
  $tmp1:
    .cfi_def_cfa_offset 16
    addiu $2, $zero, 0
    st  $2, 12($sp)
    addiu $2, $zero, %hi(date)
    shl $2, $2, 16
    addiu $2, $2, %lo(date)
    ld  $2, 8($2)
    st  $2, 8($sp)
    addiu $2, $zero, %hi(a)
    shl $2, $2, 16
    addiu $2, $2, %lo(a)
    ld  $2, 4($2)
    st  $2, 4($sp)
    addiu $sp, $sp, 16
    ret $lr
    .set  macro
  ...

Summary the issues for the code generated as above and in last section as follows:

1. It store the arguments to wrong offset. 
2. !ADJCALLSTACKUP and !ADJCALLSTACKDOWN.
3. The $gp is caller saved register. The caller main() hasn't save $gp before 
   calling sum_i() which will has
   bug if the callee sum_i() has changed $gp. Programmer can change $gp with 
   assembly code in sum_i().
4. Return value of main().

Solve these issues in each sub-section.


Fix the wrong offset in storing arguments to stack frame
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

To fix the wrong offset in storing arguments, we modify the following code 
in eliminateFrameIndex() as follows. 
The code as below is modified in Chapter9_3/ to set the caller outgoing 
arguments into spOffset($sp) (Chapter9_2/ set them to pOffset+stackSize($sp).

.. rubric:: lbdex/Chapter9_3/Cpu0RegisterInfo.cpp
.. code-block:: c++

  void Cpu0RegisterInfo::
  eliminateFrameIndex(MachineBasicBlock::iterator II, int SPAdj,
                      RegScavenger *RS) const {
    ...
    Cpu0FunctionInfo *Cpu0FI = MF.getInfo<Cpu0FunctionInfo>();
    ...
    if (Cpu0FI->isOutArgFI(FrameIndex) || Cpu0FI->isDynAllocFI(FrameIndex) ||
        (FrameIndex >= MinCSFI && FrameIndex <= MaxCSFI))
      FrameReg = Cpu0::SP;
    else
      FrameReg = getFrameRegister(MF);
    ...
    // Calculate final offset.
    // - There is no need to change the offset if the frame object is one of the
    //   following: an outgoing argument, pointer to a dynamically allocated
    //   stack space or a $gp restore location,
    // - If the frame object is any of the following, its offset must be adjusted
    //   by adding the size of the stack:
    //   incoming argument, callee-saved register location or local variable.
    if (Cpu0FI->isOutArgFI(FrameIndex) || Cpu0FI->isGPFI(FrameIndex) ||
        Cpu0FI->isDynAllocFI(FrameIndex))
      Offset = spOffset;
    else
      Offset = spOffset + (int64_t)stackSize;
    Offset    += MI.getOperand(i+1).getImm();
    ...
  }
    
.. rubric:: lbdex/Chapter9_3/Cpu0MachineFunction.h
.. code-block:: c++

  ...
  Cpu0FunctionInfo(MachineFunction& MF)
  : 
  ...
  bool isOutArgFI(int FI) const {
    return FI <= OutArgFIRange.first && FI >= OutArgFIRange.second;
  }
  ...


Run Chapter9_3/ with ch9_1.cpp will get the following result. 
It corrects arguements offset im main() from (0+40)$sp, (8+40)$sp, ..., to 
(0)$sp, (8)$sp, ..., where the stack size is 40 in main().

.. code-block:: bash

  118-165-78-230:InputFiles Jonathan$ /Users/Jonathan/llvm/test/cmake_debug_build/
  bin/Debug/llc -march=cpu0 -relocation-model=pic -filetype=asm ch9_1.bc -o 
  ch9_1.cpu0.s
  118-165-78-230:InputFiles Jonathan$ cat ch9_1.cpu0.s 
    ...
    addiu $2, $zero, 6
    st  $2, 20($sp)		// Correct offset
    addiu $2, $zero, 5
    st  $2, 16($sp)
    addiu $2, $zero, 4
    st  $2, 12($sp)
    addiu $2, $zero, 3
    st  $2, 8($sp)
    addiu $2, $zero, 2
    st  $2, 4($sp)
    addiu $2, $zero, 1
    st  $2, 0($sp)
    ld  $t9, %call16(_Z5sum_iiiiiii)($gp)
    jalr  $t9
    ...


The incoming arguments is the formal arguments defined in compiler and program 
language books. The outgoing arguments is the actual arguments.
Summary as Table: Callee incoming arguments and caller outgoing arguments.

.. table:: Callee incoming arguments and caller outgoing arguments

  ========================  ===========================================    ===============================
  Description               Callee                                         Caller   
  ========================  ===========================================    ===============================
  Charged Function          LowerFormalArguments()                         LowerCall()
  Charged Function Created  Create load vectors for incoming arguments     Create store vectors for outgoing arguments
  Arguments location        spOffset + stackSize                           spOffset
  ========================  ===========================================    ===============================



Pseudo hook instruction ADJCALLSTACKDOWN and ADJCALLSTACKUP
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

To fix the !ADJSTACKDOWN and !ADJSTACKUP, we call Cpu0GenInstrInfo(Cpu0::
ADJCALLSTACKDOWN, Cpu0::ADJCALLSTACKUP) in Cpu0InstrInfo() constructor 
function and define eliminateCallFramePseudoInstr() as follows, 

.. rubric:: lbdex/Chapter9_3/Cpu0InstrInfo.cpp
.. code-block:: c++

  Cpu0InstrInfo::Cpu0InstrInfo(Cpu0TargetMachine &tm)
    : 
      Cpu0GenInstrInfo(Cpu0::ADJCALLSTACKDOWN, Cpu0::ADJCALLSTACKUP),
  ...
  
.. rubric:: lbdex/Chapter9_3/Cpu0FrameLowering.h
.. code-block:: c++

  void eliminateCallFramePseudoInstr(MachineFunction &MF,
                                     MachineBasicBlock &MBB,
                                     MachineBasicBlock::iterator I) const;

.. rubric:: lbdex/Chapter9_3/Cpu0FrameLowering.cpp
.. code-block:: c++

  ...
  // Cpu0
  // This function eliminate ADJCALLSTACKDOWN,
  // ADJCALLSTACKUP pseudo instructions
  void Cpu0FrameLowering::
  eliminateCallFramePseudoInstr(MachineFunction &MF, MachineBasicBlock &MBB,
                  MachineBasicBlock::iterator I) const {
    // Simply discard ADJCALLSTACKDOWN, ADJCALLSTACKUP instructions.
    MBB.erase(I);
  }

With above definition, eliminateCallFramePseudoInstr() will be called when 
llvm meets pseudo instructions ADJCALLSTACKDOWN and ADJCALLSTACKUP. 
We just discard these 2 pseudo instructions. 
Run Chapter9_3/ with ch9_1.cpp will discard these two Pseudo hook instructions.

Handle $gp register in PIC addressing mode
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

In "section Global variable" [5]_, we mentioned two link 
type, the static link and dynamic link. 
The option -relocation-model=static is for static link function while option 
-relocation-model=pic is for dynamic link function.
One example of dynamic link function is used in share library. 
Share library include a lots of dynamic link functions usually can be loaded 
at run time. 
Since share library can be loaded in different memory address, the global 
variable address it access cannot be decided at link time. 
But, we still can caculate the distance between the global variable address and 
the start address of shared library function when it be loaded.

Let's run Chapter9_3/ with ch9_1.cpp to get the following correct result. 
We putting the comments in the result for explanation.

.. code-block:: bash

  118-165-78-230:InputFiles Jonathan$ cat ch9_1.cpu0.s
  _Z5sum_iiiiiii:
  ...
      .cpload $t9 // assign $gp = $t9 by loader when loader load re-entry 
                  //  function (shared library) of _Z5sum_iiiiiii
      .set    nomacro
  # BB#0:
      addiu   $sp, $sp, -32
  $tmp1:
      .cfi_def_cfa_offset 32
  ...
      ld  $3, %got(gI)($gp)   // %got(gI) is offset of (gI - _Z5sum_iiiiiii)
  ...
      ret $lr
      .set    macro
      .set    reorder
      .end    _Z5sum_iiiiiii
  ...
      .ent    main                    # @main
  main:
      .cfi_startproc
  ...
      .cpload $t9
      .set    nomacro
  ...
      .cprestore  24  	// save $gp to 24($sp)
      addiu   $2, $zero, 0
  ...
      ld  $t9, %call16(_Z5sum_iiiiiii)($gp)
      jalr    $t9      	// $t9 register is the alias of $6
      ld  $gp, 24($sp)	// restore $gp from 24($sp)
  ...
      .end    main
  $tmp7:
      .size   main, ($tmp7)-main
      .cfi_endproc
    
      .type   gI,@object              # @gI
      .data
      .globl  gI
      .align  2
  gI:
      .4byte  100                     # 0x64
      .size   gI, 4

As above code comment, **“.cprestore 24”** is a pseudo instruction for saving 
**$gp** to **24($sp)** while Instruction **“ld $gp, 24($sp)”** will restore 
the $gp. 
In other word, $gp is a caller saved register, so main() need to save/restore 
$gp before/after call the shared library _Z5sum_iiiiiii() function. 
In _Z5sum_iiiiiii() function, we translate global variable gI address by 
**“ld $3, %got(gI)($gp)”** where %got(gI) is the offset value of 
(gI - _Z5sum_iiiiiii) which can be caculated at link time.

According the original cpu0 web site information, it only support **“jsub”** 24 
bits address range access. 
We add **“jalr”** to cpu0 and expand it to 32 bit address. We did this change for 
two reasons. One is cpu0 can be expanded to 32 bit address space by only adding 
this instruction. 
The other is cpu0 as well as this book are designed for teaching purpose. 
We reserve **“jalr”** as PIC mode for dynamic linking function to demonstrates: 

1. How caller handle the caller saved register $gp in calling the function

2. How the code in the shared libray function use $gp to access global variable 
   address. 

3. The jalr for dynamic linking function is easier in implementation and faster. 
   As we have depicted in section "pic mode" of chapter "Global variables, structs 
   and arrays, other type". This solution is popular in reality and deserve change 
   cpu0 official design as a compiler book. 


Now, after the following code added in Chapter9_3/, we can issue 
**“.cprestore”** in emitPrologue() and emit "ld $gp, ($gp save slot on stack)" 
after jalr by create file Cpu0EmitGPRestore.cpp which run as a function pass.

.. rubric:: lbdex/Chapter9_3/CMakeLists.txt
.. code-block:: c++

  add_llvm_target(Cpu0CodeGen
    ...
    Cpu0EmitGPRestore.cpp
  ...
  
.. rubric:: lbdex/Chapter9_3/Cpu0TargetMachine.cpp
.. code-block:: c++

  Cpu0elTargetMachine::
  Cpu0elTargetMachine(const Target &T, StringRef TT,
                      StringRef CPU, StringRef FS, const TargetOptions &Options,
                      Reloc::Model RM, CodeModel::Model CM,
                      CodeGenOpt::Level OL)
    : Cpu0TargetMachine(T, TT, CPU, FS, Options, RM, CM, OL, true) {}
  namespace {
    ...
    virtual bool addPreRegAlloc();
    ...
  }

  bool Cpu0PassConfig::addPreRegAlloc() {
    // Do not restore $gp if target is Cpu064.
    // In N32/64, $gp is a callee-saved register.
  
    addPass(createCpu0EmitGPRestorePass(getCpu0TargetMachine()));
    return true;
  }
  
.. rubric:: lbdex/Chapter9_3/Cpu0.h
.. code-block:: c++

    FunctionPass *createCpu0EmitGPRestorePass(Cpu0TargetMachine &TM);
  
.. rubric:: lbdex/Chapter9_3/Cpu0FrameLowering.cpp
.. code-block:: c++

  void Cpu0FrameLowering::emitPrologue(MachineFunction &MF) const {
    ...
    unsigned RegSize = 4;
    unsigned LocalVarAreaOffset = 
      Cpu0FI->needGPSaveRestore() ?
      (MFI->getObjectOffset(Cpu0FI->getGPFI()) + RegSize) :
      Cpu0FI->getMaxCallFrameSize();
    ...
    // Restore GP from the saved stack location
    if (Cpu0FI->needGPSaveRestore()) {
      unsigned Offset = MFI->getObjectOffset(Cpu0FI->getGPFI());
      BuildMI(MBB, MBBI, dl, TII.get(Cpu0::CPRESTORE)).addImm(Offset)
        .addReg(Cpu0::GP);
    }
  }
  
.. rubric:: lbdex/Chapter9_3/Cpu0InstrInfo.td
.. code-block:: c++

  let neverHasSideEffects = 1 in
  def CPRESTORE : Cpu0Pseudo<(outs), (ins i32imm:$loc, CPURegs:$gp),
                 ".cprestore\t$loc", []>;
  
  
.. rubric:: lbdex/Chapter9_3/Cpu0ISelLowering.cpp
.. code-block:: c++

  SDValue
  Cpu0TargetLowering::LowerCall(TargetLowering::CallLoweringInfo &CLI,
                                SmallVectorImpl<SDValue> &InVals) const {
    ...
    // If this is the first call, create a stack frame object that points to
    // a location to which .cprestore saves $gp.
    if (IsPIC && Cpu0FI->globalBaseRegFixed() && !Cpu0FI->getGPFI())
      ...
      if (MaxCallFrameSize < NextStackOffset) {
        ...
        if (Cpu0FI->needGPSaveRestore())
          MFI->setObjectOffset(Cpu0FI->getGPFI(), NextStackOffset);
        }
        ...
      }
      ...
  }
  
.. rubric:: lbdex/Chapter9_3/Cpu0EmitGPRestore.cpp
.. literalinclude:: ../../../lib/Target/Cpu0/Cpu0EmitGPRestore.cpp

  
.. rubric:: lbdex/Chapter9_3/Cpu0AsmPrinter.cpp
.. literalinclude:: ../../../lib/Target/Cpu0/Cpu0AsmPrinter.cpp
    :start-after: using namespace llvm;
    :end-before: bool Cpu0AsmPrinter::runOnMachineFunction(MachineFunction &MF) {
.. code-block:: c++

  ...
  void Cpu0AsmPrinter::EmitInstruction(const MachineInstr *MI) {
    ...
    unsigned Opc = MI->getOpcode();
    ...

.. literalinclude:: ../../../lib/Target/Cpu0/Cpu0AsmPrinter.cpp
    :start-after: MCInst TmpInst0;
    :end-before: MCInstLowering.Lower(MI, TmpInst0);
.. code-block:: c++

    ...
  }
  
.. rubric:: lbdex/Chapter9_3/Cpu0MCInstLower.cpp
.. literalinclude:: ../../../lib/Target/Cpu0/Cpu0MCInstLower.cpp
    :start-after: // lbd document - mark - LowerCPLOAD
    :end-before: MCOperand Cpu0MCInstLower::LowerOperand

.. rubric:: lbdex/Chapter9_3/Cpu0MCInstLower.h
.. literalinclude:: ../../../lib/Target/Cpu0/Cpu0MCInstLower.h
    :start-after: void LowerCPLOAD(SmallVector<MCInst, 4>& MCInsts);
    :end-before: private:

The added code of Cpu0AsmPrinter.cpp as above will call the LowerCPRESTORE() when 
user run with ``llc -filetype=obj``. 
The added code of Cpu0MCInstLower.cpp as above takes care the .cprestore machine 
instructions.

.. code-block:: bash

  118-165-76-131:InputFiles Jonathan$ /Users/Jonathan/llvm/test/
  cmake_debug_build/bin/Debug/llc -march=cpu0 -relocation-model=pic -filetype=
  obj ch9_1.bc -o ch9_1.cpu0.o
  118-165-76-131:InputFiles Jonathan$ hexdump  ch9_1.cpu0.o
  ...
  // .cprestore machine instruction “ 01 ad 00 18”
  00000d0 01 ad 00 18 09 20 00 00 01 2d 00 40 09 20 00 06
  ...
  
  118-165-67-25:InputFiles Jonathan$ cat ch9_1.cpu0.s
  ...
    .ent  _Z5sum_iiiiiii          # @_Z5sum_iiiiiii
  _Z5sum_iiiiiii:
  ...
    .cpload $t9 // assign $gp = $t9 by loader when loader load re-entry function 
                // (shared library) of _Z5sum_iiiiiii
    .set  nomacro
  # BB#0:
  ...
    .ent  main                    # @main
  ...
    .cprestore  24  // save $gp to 24($sp)
  ...

Run ``llc -static`` will call jsub instruction instead of jalr as follows,

.. code-block:: bash

  118-165-76-131:InputFiles Jonathan$ /Users/Jonathan/llvm/test/
  cmake_debug_build/bin/Debug/llc -march=cpu0 -relocation-model=static -filetype=
  asm ch9_1.bc -o ch9_1.cpu0.s
  118-165-76-131:InputFiles Jonathan$ cat ch9_1.cpu0.s
  ...
    jsub  _Z5sum_iiiiiii
  ...

Run with ``llc -filetype=obj``, you can find the Cx of **“jsub Cx”** is 0 since 
the Cx is calculated by linker as below. 
Mips has the same 0 in it's jal instruction. 
The ch9_1_3.cpp and ch9_1_4.cpp are example code more for test. 

.. code-block:: bash

  // jsub _Z5sum_iiiiiii translate into 2B 00 00 00
  00F0: 2B 00 00 00 01 2D 00 34 00 ED 00 3C 09 DD 00 40 


Support features
-----------------

This section support features of struct type, variable number of arguments and 
dynamic stack allocation.

Run Chapter9_3 with ch9_2_1.cpp will get the error message as follows,

.. rubric:: lbdex/InputFiles/ch9_2_1.cpp
.. literalinclude:: ../lbdex/InputFiles/ch9_2_1.cpp
    :start-after: /// start


.. code-block:: bash

  JonathantekiiMac:InputFiles Jonathan$ clang -target mips-unknown-linux-gnu -c 
  ch9_2_1.cpp -emit-llvm -o ch9_2_1.bc
  JonathantekiiMac:InputFiles Jonathan$ /Users/Jonathan/llvm/test/
  cmake_debug_build/bin/Debug/llc -march=cpu0 -relocation-model=pic -filetype=asm 
  ch9_2_1.bc -o ch9_2_1.cpu0.s
  ...
  sret virtual register not created in the entry block
  ...

Run Chapter9_3/ with ch9_3.cpp to get the following error,

.. rubric:: lbdex/InputFiles/ch9_3.cpp
.. literalinclude:: ../lbdex/InputFiles/ch9_3.cpp
    :start-after: /// start

.. code-block:: bash

  118-165-78-230:InputFiles Jonathan$ clang -target mips-unknown-linux-gnu -c 
  ch9_3.cpp -emit-llvm -o ch9_3.bc
  118-165-78-230:InputFiles Jonathan$ /Users/Jonathan/llvm/test/cmake_debug_build/
  bin/Debug/llc -march=cpu0 -relocation-model=pic -filetype=asm ch9_3.bc -o 
  ch9_3.cpu0.s
  LLVM ERROR: Cannot select: 0x7f8b6902fd10: ch = vastart 0x7f8b6902fa10, 
  0x7f8b6902fb10, 0x7f8b6902fc10 [ORD=9] [ID=22]
    0x7f8b6902fb10: i32 = FrameIndex<5> [ORD=7] [ID=9]
  In function: _Z5sum_iiz


.. rubric:: lbdex/InputFiles/ch9_4.cpp
.. literalinclude:: ../lbdex/InputFiles/ch9_4.cpp
    :start-after: /// start


Run Chapter9_3 with ch9_4.cpp will get the following error.

.. code-block:: bash

  118-165-72-242:InputFiles Jonathan$ clang -target mips-unknown-linux-gnu -I/
  Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/
  SDKs/MacOSX10.8.sdk/usr/include/ -c ch9_4.cpp -emit-llvm -o ch9_4.bc
  118-165-72-242:InputFiles Jonathan$ /Users/Jonathan/llvm/test/cmake_debug_build/
  bin/Debug/llc -march=cpu0 -relocation-model=pic -filetype=asm ch9_4.bc -o 
  ch9_4.cpu0.s
  LLVM ERROR: Cannot select: 0x7ffd8b02ff10: i32,ch = dynamic_stackalloc 
  0x7ffd8b02f910:1, 0x7ffd8b02fe10, 0x7ffd8b02c010 [ORD=12] [ID=48]
    0x7ffd8b02fe10: i32 = and 0x7ffd8b02fc10, 0x7ffd8b02fd10 [ORD=12] [ID=47]
      0x7ffd8b02fc10: i32 = add 0x7ffd8b02fa10, 0x7ffd8b02fb10 [ORD=12] [ID=46]
        0x7ffd8b02fa10: i32 = shl 0x7ffd8b02f910, 0x7ffd8b02f510 [ID=45]
          0x7ffd8b02f910: i32,ch = load 0x7ffd8b02ee10, 0x7ffd8b02e310, 
          0x7ffd8b02b310<LD4[%1]> [ID=44]
            0x7ffd8b02e310: i32 = FrameIndex<1> [ORD=3] [ID=10]
            0x7ffd8b02b310: i32 = undef [ORD=1] [ID=2]
          0x7ffd8b02f510: i32 = Constant<2> [ID=25]
        0x7ffd8b02fb10: i32 = Constant<7> [ORD=12] [ID=16]
      0x7ffd8b02fd10: i32 = Constant<-8> [ORD=12] [ID=17]
    0x7ffd8b02c010: i32 = Constant<0> [ORD=12] [ID=8]
  In function: _Z5sum_iiiiiii


Structure type support
~~~~~~~~~~~~~~~~~~~~~~~

Chapter9_4/ with the following code added to support the structure type in 
function call. 

.. rubric:: lbdex/Chapter9_4/Cpu0ISelLowering.cpp
.. literalinclude:: ../../../lib/Target/Cpu0/Cpu0ISelLowering.cpp
    :start-after: //  Lower helper functions
    :end-before: //  Misc Lower Operation implementation
.. literalinclude:: ../../../lib/Target/Cpu0/Cpu0ISelLowering.cpp
    :start-after: #include "Cpu0GenCallingConv.inc"
    :end-before: // lbd document - mark - before LowerCall

.. code-block:: c++

  SDValue
  Cpu0TargetLowering::LowerCall(TargetLowering::CallLoweringInfo &CLI,
                  SmallVectorImpl<SDValue> &InVals) const {
    ...
    // Walk the register/memloc assignments, inserting copies/loads.
    for (unsigned i = 0, e = ArgLocs.size(); i != e; ++i) {
      ...
      // ByVal Arg.
      if (Flags.isByVal()) {
        ...
        WriteByValArg(ByValChain, Chain, DL, RegsToPass, MemOpChains, LastFI,
              MFI, DAG, Arg, VA, Flags, getPointerTy(),
              Subtarget->isLittle());
        ...
      }
      ...
    }
    ...
  }

.. literalinclude:: ../../../lib/Target/Cpu0/Cpu0ISelLowering.cpp
    :start-after: Formal Arguments Calling Convention Implementation
    :end-before: /// LowerFormalArguments - transform physical registers into 

.. code-block:: c++

  ...
  SDValue
  Cpu0TargetLowering::LowerFormalArguments(SDValue Chain,
                       CallingConv::ID CallConv,
                       bool isVarArg,
                      const SmallVectorImpl<ISD::InputArg> &Ins,
                       SDLoc DL, SelectionDAG &DAG,
                       SmallVectorImpl<SDValue> &InVals)
                        const {
    ...
    for (unsigned i = 0, e = ArgLocs.size(); i != e; ++i, ++FuncArg) {
      ...
      if (Flags.isByVal()) {
        assert(Flags.getByValSize() &&
           "ByVal args of size 0 should have been ignored by front-end."); 
        unsigned NumWords = (Flags.getByValSize() + 3) / 4;
        LastFI = MFI->CreateFixedObject(NumWords * 4, VA.getLocMemOffset(),
                        true);
        SDValue FIN = DAG.getFrameIndex(LastFI, getPointerTy());
        InVals.push_back(FIN);
        ReadByValArg(MF, Chain, DL, OutChains, DAG, NumWords, FIN, VA, Flags,
               &*FuncArg);
        continue;
      }
      ...
    }
    // The cpu0 ABIs for returning structs by value requires that we copy
    // the sret argument into $v0 for the return. Save the argument into
    // a virtual register so that we can access it from the return points.
    if (DAG.getMachineFunction().getFunction()->hasStructRetAttr()) {
      unsigned Reg = Cpu0FI->getSRetReturnReg();
      if (!Reg) {
        Reg = MF.getRegInfo().createVirtualRegister(getRegClassFor(MVT::i32));
        Cpu0FI->setSRetReturnReg(Reg);
      }
      SDValue Copy = DAG.getCopyToReg(DAG.getEntryNode(), DL, Reg, InVals[0]);
      Chain = DAG.getNode(ISD::TokenFactor, DL, MVT::Other, Copy, Chain);
    }
    ...
  }


In addition to above code, we have defined the calling convention at early of 
this chapter as follows,

.. rubric:: lbdex/Chapter9_4/Cpu0CallingConv.td
.. code-block:: c++

  def RetCC_Cpu0EABI : CallingConv<[
    // i32 are returned in registers V0, V1, A0, A1
    CCIfType<[i32], CCAssignToReg<[V0, V1, A0, A1]>>
  ]>;


It meaning for the return value, we keep it in registers V0, V1, A0, A1 if the return 
value didn't over 4 registers size; If it over 4 registers size, cpu0 will save 
them with pointer.
For explanation, let's run Chapter9_4/ with ch9_2_1.cpp and explain with this 
example.

.. code-block:: bash

  JonathantekiiMac:InputFiles Jonathan$ cat ch9_2_1.cpu0.s
    .section .mdebug.abi32
    .previous
    .file "ch9_2_1.bc"
    .text
    .globl  _Z7getDatev
    .align  2
    .type _Z7getDatev,@function
    .ent  _Z7getDatev             # @_Z7getDatev
  _Z7getDatev:
    .cfi_startproc
    .frame  $sp,0,$lr
    .mask   0x00000000,0
    .set  noreorder
    .cpload $t9
    .set  nomacro
  # BB#0:
    ld  $2, 0($sp)        // $2 is 192($sp)
    ld  $3, %got(gDate)($gp)  // $3 is &gDate
    ld  $4, 20($3)        // save gDate contents to 212..192($sp)
    st  $4, 20($2)
    ld  $4, 16($3)
    st  $4, 16($2)
    ld  $4, 12($3)
    st  $4, 12($2)
    ld  $4, 8($3)
    st  $4, 8($2)
    ld  $4, 4($3)
    st  $4, 4($2)
    ld  $3, 0($3)
    st  $3, 0($2)
    ret $lr
    .set  macro
    .set  reorder
    .end  _Z7getDatev
  $tmp0:
    .size _Z7getDatev, ($tmp0)-_Z7getDatev
    .cfi_endproc
  
    .globl  _Z8copyDate4Date
    .align  2
    .type _Z8copyDate4Date,@function
    .ent  _Z8copyDate4Date        # @_Z8copyDate4Date
  _Z8copyDate4Date:
    .cfi_startproc
    .frame  $sp,0,$lr
    .mask   0x00000000,0
    .set  noreorder
    .set  nomacro
  # BB#0:
    st  $5, 4($sp)
    ld  $2, 0($sp)        // $2 = 168($sp)
    ld  $3, 24($sp)
    st  $3, 20($2)        // copy date1, 24..4($sp), to date2, 
    ld  $3, 20($sp)       //  188..168($sp)
    st  $3, 16($2)
    ld  $3, 16($sp)
    st  $3, 12($2)
    ld  $3, 12($sp)
    st  $3, 8($2)
    ld  $3, 8($sp)
    st  $3, 4($2)
    ld  $3, 4($sp)
    st  $3, 0($2)
    ret $lr
    .set  macro
    .set  reorder
    .end  _Z8copyDate4Date
  $tmp1:
    .size _Z8copyDate4Date, ($tmp1)-_Z8copyDate4Date
    .cfi_endproc
  
    .globl  _Z8copyDateP4Date
    .align  2
    .type _Z8copyDateP4Date,@function
    .ent  _Z8copyDateP4Date       # @_Z8copyDateP4Date
  _Z8copyDateP4Date:
    .cfi_startproc
    .frame  $sp,8,$lr
    .mask   0x00000000,0
    .set  noreorder
    .set  nomacro
  # BB#0:
    addiu $sp, $sp, -8
  $tmp3:
    .cfi_def_cfa_offset 8
    ld  $2, 8($sp)        // $2 = 120($sp of main) date2
    ld  $3, 12($sp)       // $3 = 192($sp of main) date1
    st  $3, 0($sp)
    ld  $4, 20($3)        // copy date1, 212..192($sp of main), 
    st  $4, 20($2)        //  to date2, 140..120($sp of main)
    ld  $4, 16($3)
    st  $4, 16($2)
    ld  $4, 12($3)
    st  $4, 12($2)
    ld  $4, 8($3)
    st  $4, 8($2)
    ld  $4, 4($3)
    st  $4, 4($2)
    ld  $3, 0($3)
    st  $3, 0($2)
    addiu $sp, $sp, 8
    ret $lr
    .set  macro
    .set  reorder
    .end  _Z8copyDateP4Date
  $tmp4:
    .size _Z8copyDateP4Date, ($tmp4)-_Z8copyDateP4Date
    .cfi_endproc
  
    .globl  _Z8copyTime4Time
    .align  2
    .type _Z8copyTime4Time,@function
    .ent  _Z8copyTime4Time        # @_Z8copyTime4Time
  _Z8copyTime4Time:
    .cfi_startproc
    .frame  $sp,64,$lr
    .mask   0x00000000,0
    .set  noreorder
    .set  nomacro
  # BB#0:
    addiu $sp, $sp, -64
  $tmp6:
    .cfi_def_cfa_offset 64
    ld  $2, 68($sp)       // save 8..0 ($sp of main) to 24..16($sp)
    st  $2, 20($sp)
    ld  $2, 64($sp)
    st  $2, 16($sp)
    ld  $2, 72($sp)
    st  $2, 24($sp)
    st  $2, 40($sp)       // save 8($sp of main) to 40($sp)
    ld  $2, 20($sp)       // time1.minute, save time1.minute and 
    st  $2, 36($sp)       //  time1.second to 36..32($sp)
    ld  $2, 16($sp)       // time1.second
    st  $2, 32($sp)
    ld  $2, 40($sp)       // $2 = 8($sp of main) = time1.hour
    st  $2, 56($sp)       // copy time1 to 56..48($sp)
    ld  $2, 36($sp)
    st  $2, 52($sp)
    ld  $2, 32($sp)
    st  $2, 48($sp)
    ld  $2, 48($sp)       // copy time1 to 8..0($sp)
    ld  $3, 52($sp)
    ld  $4, 56($sp)
    st  $4, 8($sp)
    st  $3, 4($sp)
    st  $2, 0($sp)
    ld  $2, 0($sp)        // put time1 to $2, $3 and $4 ($v0, $v1 and $a0)
    ld  $3, 4($sp)
    ld  $4, 8($sp)
    addiu $sp, $sp, 64
    ret $lr
    .set  macro
    .set  reorder
    .end  _Z8copyTime4Time
  $tmp7:
    .size _Z8copyTime4Time, ($tmp7)-_Z8copyTime4Time
    .cfi_endproc
  
    .globl  _Z8copyTimeP4Time
    .align  2
    .type _Z8copyTimeP4Time,@function
    .ent  _Z8copyTimeP4Time       # @_Z8copyTimeP4Time
  _Z8copyTimeP4Time:
    .cfi_startproc
    .frame  $sp,40,$lr
    .mask   0x00000000,0
    .set  noreorder
    .set  nomacro
  # BB#0:
    addiu $sp, $sp, -40
  $tmp9:
    .cfi_def_cfa_offset 40
    ld  $2, 40($sp)       // 216($sp of main)
    st  $2, 16($sp)
    ld  $3, 8($2)       // copy time1, 224..216($sp of main) to 
    st  $3, 32($sp)       //  32..24($sp), 8..0($sp) and $2, $3, $4
    ld  $3, 4($2)
    st  $3, 28($sp)
    ld  $2, 0($2)
    st  $2, 24($sp)
    ld  $2, 24($sp)
    ld  $3, 28($sp)
    ld  $4, 32($sp)
    st  $4, 8($sp)
    st  $3, 4($sp)
    st  $2, 0($sp)
    ld  $2, 0($sp)
    ld  $3, 4($sp)
    ld  $4, 8($sp)
    addiu $sp, $sp, 40
    ret $lr
    .set  macro
    .set  reorder
    .end  _Z8copyTimeP4Time
  $tmp10:
    .size _Z8copyTimeP4Time, ($tmp10)-_Z8copyTimeP4Time
    .cfi_endproc
  
    .globl  _Z20test_func_arg_structv
    .align  2
    .type _Z20test_func_arg_structv,@function
    .ent  _Z20test_func_arg_structv                    # @main
  _Z20test_func_arg_structv:
    .cfi_startproc
    .frame  $sp,248,$lr
    .mask   0x00004180,-4
    .set  noreorder
    .cpload $t9
    .set  nomacro
  # BB#0:
    addiu $sp, $sp, -248
  $tmp13:
    .cfi_def_cfa_offset 248
    st  $lr, 244($sp)           # 4-byte Folded Spill
    st  $8, 240($sp)            # 4-byte Folded Spill
    st  $7, 236($sp)            # 4-byte Folded Spill
  $tmp14:
    .cfi_offset 14, -4
  $tmp15:
    .cfi_offset 8, -8
  $tmp16:
    .cfi_offset 7, -12
    .cprestore  16
    addiu $7, $zero, 0
    st  $7, 232($sp)
    ld  $2, %got($_ZZ4mainE5time1)($gp)
    addiu $2, $2, %lo($_ZZ4mainE5time1)
    ld  $3, 8($2)     // save initial value to time1, 224..216($sp)
    st  $3, 224($sp)
    ld  $3, 4($2)
    st  $3, 220($sp)
    ld  $2, 0($2)
    st  $2, 216($sp)
    addiu $8, $sp, 192
    st  $8, 0($sp)      // *(0($sp)) = 192($sp)
    ld  $t9, %call16(_Z7getDatev)($gp) // copy gDate contents to date1, 212..192($sp)
    jalr  $t9
    ld  $gp, 16($sp)
    ld  $2, 212($sp)    // copy 212..192($sp) to 164..144($sp)
    st  $2, 164($sp)
    ld  $2, 208($sp)
    st  $2, 160($sp)
    ld  $2, 204($sp)
    st  $2, 156($sp)
    ld  $2, 200($sp)
    st  $2, 152($sp)
    ld  $2, 196($sp)
    st  $2, 148($sp)
    ld  $2, 192($sp)
    st  $2, 144($sp)
    ld  $2, 164($sp)    // copy 164..144($sp) to 24..4($sp)
    st  $2, 24($sp)
    ld  $2, 160($sp)
    st  $2, 20($sp)
    ld  $2, 156($sp)
    st  $2, 16($sp)
    ld  $2, 152($sp)
    st  $2, 12($sp)
    ld  $2, 148($sp)
    st  $2, 8($sp)
    ld  $2, 144($sp)
    st  $2, 4($sp)
    addiu $2, $sp, 168
    st  $2, 0($sp)      // *0($sp) = 168($sp)
    ld  $t9, %call16(_Z8copyDate4Date)($gp)
    jalr  $t9
    ld  $gp, 16($sp)
    st  $8, 4($sp)      // 4($sp) = 192($sp) date1
    addiu $2, $sp, 120
    st  $2, 0($sp)      // *0($sp) = 120($sp) date2
    ld  $t9, %call16(_Z8copyDateP4Date)($gp)
    jalr  $t9
    ld  $gp, 16($sp)
    ld  $2, 224($sp)    // save time1 to arguments passing location, 
    st  $2, 96($sp)     //  8..0($sp)
    ld  $2, 220($sp)
    st  $2, 92($sp)
    ld  $2, 216($sp)
    st  $2, 88($sp)
    ld  $2, 88($sp)
    ld  $3, 92($sp)
    ld  $4, 96($sp)
    st  $4, 8($sp)
    st  $3, 4($sp)
    st  $2, 0($sp)
    ld  $t9, %call16(_Z8copyTime4Time)($gp)
    jalr  $t9
    ld  $gp, 16($sp)
    st  $3, 76($sp)     // save return value time2 from $2, $3, $4 to
    st  $2, 72($sp)     //   80..72($sp) and 112..104($sp)
    st  $4, 80($sp)
    ld  $2, 72($sp)
    ld  $3, 76($sp)
    ld  $4, 80($sp)
    st  $4, 112($sp)
    st  $3, 108($sp)
    st  $2, 104($sp)
    addiu $2, $sp, 216
    st  $2, 0($sp)      // *(0($sp)) = 216($sp)
    ld  $t9, %call16(_Z8copyTimeP4Time)($gp)
    jalr  $t9
    ld  $gp, 16($sp)
    st  $3, 44($sp)     // save return value time3 from $2, $3, $4 to
    st  $2, 40($sp)     //  48..44($sp) 64..56($sp)
    st  $4, 48($sp)
    ld  $2, 40($sp)
    ld  $3, 44($sp)
    ld  $4, 48($sp)
    st  $4, 64($sp)
    st  $3, 60($sp)
    st  $2, 56($sp)
    add $2, $zero, $7   // return 0 by $2, ($7 is 0)
                
    ld  $7, 236($sp)            # 4-byte Folded Reload // restore callee saved
    ld  $8, 240($sp)            # 4-byte Folded Reload //  registers $s0, $s1 
    ld  $lr, 244($sp)           # 4-byte Folded Reload //  ($7, $8)
    addiu $sp, $sp, 248
    ret $lr
    .set  macro
    .set  reorder
    .end  _Z20test_func_arg_structv
  $tmp17:
    .size _Z20test_func_arg_structv, ($tmp17)-_Z20test_func_arg_structv
    .cfi_endproc
  
    .type gDate,@object           # @gDate
    .data
    .globl  gDate
    .align  2
  gDate:
    .4byte  2012                    # 0x7dc
    .4byte  10                      # 0xa
    .4byte  12                      # 0xc
    .4byte  1                       # 0x1
    .4byte  2                       # 0x2
    .4byte  3                       # 0x3
    .size gDate, 24
  
    .type gTime,@object           # @gTime
    .globl  gTime
    .align  2
  gTime:
    .4byte  2                       # 0x2
    .4byte  20                      # 0x14
    .4byte  30                      # 0x1e
    .size gTime, 12
  
    .type $_Z20test_func_arg_structvE5time1,@object # @_Z20test_func_arg_structvE5time1
    .section  .rodata,"a",@progbits
    .align  2
  $_Z20test_func_arg_structvE5time1:
    .4byte  1                       # 0x1
    .4byte  10                      # 0xa
    .4byte  12                      # 0xc
    .size $_Z20test_func_arg_structvE5time1, 12


In LowerCall(), Flags.isByVal() will be true if the outgoing arguments over 4 
registers size, then it will call WriteByValArg(..., getPointerTy(), ...) to 
save those arguments to stack as offset. 
For example code of ch9_2_1.cpp, Flags.isByVal() is true for copyDate(date1) 
outgoing arguments, since the date1 is type of Date which contains 6 integers 
(year, month, day, hour, minute, second). 
But Flags.isByVal() is false for copyTime(time1) since type Time is a struct 
contains 3 integers (hour, minute, second).
So, if you mark WriteByValArg(..., getPointerTy(), ...), the result will 
missing the following code in caller, main(),

.. code-block:: bash

    ld  $2, 164($sp)    // copy 164..144($sp) to 24..4($sp)
    st  $2, 24($sp)
    ld  $2, 160($sp)
    st  $2, 20($sp)
    ld  $2, 156($sp)
    st  $2, 16($sp)
    ld  $2, 152($sp)
    st  $2, 12($sp)
    ld  $2, 148($sp)
    st  $2, 8($sp)
    ld  $2, 144($sp)
    st  $2, 4($sp)		// will missing the above code
    
    addiu $2, $sp, 168
    st  $2, 0($sp)      // *0($sp) = 168($sp)
    ld  $t9, %call16(_Z8copyDate4Date)($gp)

In LowerFormalArguments(), the "if (Flags.isByVal())" getting the incoming 
arguments which corresponding the outgoing arguments of LowerCall().

LowerFormalArguments() is called when a function is entered while LowerReturn() 
is called when a function is left, reference [#]_.
The former save the return register to virtual register while the later load the 
virtual register back to return register. 
Since the return value is "struct type" and over 4 registers size, it save 
pointer (struct address) to return register.
List the code and their effect as follows,

.. rubric:: lbdex/Chapter9_4/Cpu0ISelLowering.cpp
.. code-block:: c++

  SDValue
  Cpu0TargetLowering::LowerFormalArguments(SDValue Chain,
                       CallingConv::ID CallConv,
                       bool isVarArg,
                      const SmallVectorImpl<ISD::InputArg> &Ins,
                       SDLoc DL, SelectionDAG &DAG,
                       SmallVectorImpl<SDValue> &InVals)
                        const {
    ...
  #if 1 // Incomming. Without this, it will use $3 instead of $2 as return 
    // register. The cpu0 ABIs for returning structs by value requires that we 
    // copy the sret argument into $v0 for the return. Save the argument into
    // a virtual register so that we can access it from the return points.
    if (DAG.getMachineFunction().getFunction()->hasStructRetAttr()) {
      unsigned Reg = Cpu0FI->getSRetReturnReg();
      if (!Reg) {
        Reg = MF.getRegInfo().createVirtualRegister(getRegClassFor(MVT::i32));
        Cpu0FI->setSRetReturnReg(Reg);
      }
      SDValue Copy = DAG.getCopyToReg(DAG.getEntryNode(), DL, Reg, InVals[0]);
      Chain = DAG.getNode(ISD::TokenFactor, DL, MVT::Other, Copy, Chain);
    }
  #endif // lbd document - mark - endif - hasStructRetAttr()
    ...
  }

.. code-block:: bash

    addiu $2, $sp, 168
    st  $2, 0($sp)      // *0($sp) = 168($sp); LowerFormalArguments(): 
                        //  return register is $2, virtual register is 
                        //  0($sp)
    ld  $t9, %call16(_Z8copyDate4Date)($gp)


.. rubric:: lbdex/Chapter3_6/Cpu0ISelLowering.cpp
.. code-block:: c++

  SDValue
  Cpu0TargetLowering::LowerReturn(SDValue Chain,
                  CallingConv::ID CallConv, bool isVarArg,
                  const SmallVectorImpl<ISD::OutputArg> &Outs,
                  const SmallVectorImpl<SDValue> &OutVals,
                  SDLoc DL, SelectionDAG &DAG) const {
    ...
  #if 1 // structure return begin. Without this, it will use $3 instead of $2 
    // as return register. The cpu0 ABIs for returning structs by value requires 
    // that we copy the sret argument into $v0 for the return. We saved the 
    // argument into a virtual register in the entry block, so now we copy the 
    // value out and into $v0.
    if (DAG.getMachineFunction().getFunction()->hasStructRetAttr()) {
      MachineFunction &MF      = DAG.getMachineFunction();
      Cpu0FunctionInfo *Cpu0FI = MF.getInfo<Cpu0FunctionInfo>();
      unsigned Reg = Cpu0FI->getSRetReturnReg();

      if (!Reg)
        llvm_unreachable("sret virtual register not created in the entry block");
      SDValue Val = DAG.getCopyFromReg(Chain, DL, Reg, getPointerTy());

      Chain = DAG.getCopyToReg(Chain, DL, Cpu0::V0, Val, Flag);
      Flag = Chain.getValue(1);
      RetOps.push_back(DAG.getRegister(Cpu0::V0, getPointerTy()));
    }
  #endif // structure return end
    ...
  }

.. code-block:: bash

    ...
    .file	"ch9_2_1.bc"
    ...
    .globl  _Z8copyDateP4Date
    .align  2
    .type _Z8copyDateP4Date,@function
    .ent  _Z8copyDate4Date        # @_Z8copyDate4Date
  _Z8copyDate4Date:
    .cfi_startproc
    .frame  $sp,0,$lr
    .mask   0x00000000,0
    .set  noreorder
    .set  nomacro
  # BB#0:
    st  $5, 4($sp)
    ld  $2, 0($sp)        // $2 = 168($sp); LowerReturn(): virtual 
                          //  register is 0($sp), return register is $2 
    ld  $3, 24($sp)
    st  $3, 20($2)        // copy date1, 24..4($sp), to date2, 
    ld  $3, 20($sp)       //  188..168($sp)
    st  $3, 16($2)
    ld  $3, 16($sp)
    st  $3, 12($2)
    ld  $3, 12($sp)
    st  $3, 8($2)
    ld  $3, 8($sp)
    st  $3, 4($2)
    ld  $3, 4($sp)
    st  $3, 0($2)
    ret $lr
    .set  macro
    .set  reorder
    .end  _Z8copyDate4Date

The ch9_2_2.cpp include C++ class "Date" implementation. 
It can be translated into cpu0 backend too since the front end (clang in this 
example) translate them into C language form.
If you mark the "hasStructRetAttr() if" part from both of above functions, 
the output cpu0 code for ch9_2_1.cpp will use $3 instead of $2 as return 
register as follows,

.. code-block:: bash

    ...
    .file	"ch9_2_1.bc"
    ...
    .globl  _Z8copyDateP4Date
    .align  2
    .type _Z8copyDateP4Date,@function
    .ent  _Z8copyDateP4Date       # @_Z8copyDateP4Date
  _Z8copyDateP4Date:
    .cfi_startproc
    .frame  $sp,8,$lr
    .mask   0x00000000,0
    .set  noreorder
    .set  nomacro
  # BB#0:
    addiu $sp, $sp, -8
  $tmp3:
    .cfi_def_cfa_offset 8
    ld  $2, 12($sp)
    st  $2, 0($sp)
    ld  $4, 20($2)
    ld  $3, 8($sp)
    st  $4, 20($3)
    ld  $4, 16($2)
    st  $4, 16($3)
    ld  $4, 12($2)
    st  $4, 12($3)
    ld  $4, 8($2)
    st  $4, 8($3)
    ld  $4, 4($2)
    st  $4, 4($3)
    ld  $2, 0($2)
    st  $2, 0($3)
    addiu $sp, $sp, 8
    ret $lr
    .set  macro
    .set  reorder
    .end  _Z8copyDateP4Date


Variable number of arguments
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Until now, we support fixed number of arguments in formal function definition 
(Incoming Arguments). 
This subsection support variable number of arguments since C language support 
this feature.

Run Chapter9_4/ with ch9_3.cpp as well as clang option, 
**clang -target mips-unknown-linux-gnu**, to get the following result,

.. code-block:: bash

  118-165-76-131:InputFiles Jonathan$ clang -target mips-unknown-linux-gnu -c 
  ch9_3.cpp -emit-llvm -o ch9_3.bc
  118-165-76-131:InputFiles Jonathan$ /Users/Jonathan/llvm/test/
  cmake_debug_build/bin/Debug/llc -march=cpu0 -relocation-model=pic -filetype=asm 
  ch9_3.bc -o ch9_3.cpu0.s
  118-165-76-131:InputFiles Jonathan$ cat ch9_3.cpu0.s
    .section .mdebug.abi32
    .previous
    .file "ch9_3.bc"
    .text
    .globl  _Z5sum_iiz
    .align  2
    .type _Z5sum_iiz,@function
    .ent  _Z5sum_iiz              # @_Z5sum_iiz
  _Z5sum_iiz:
    .frame  $sp,24,$lr
    .mask   0x00000000,0
    .set  noreorder
    .set  nomacro
  # BB#0:
    addiu $sp, $sp, -24
    ld  $2, 24($sp)     // amount
    st  $2, 20($sp)     // amount
    addiu $2, $zero, 0
    st  $2, 16($sp)     // i
    st  $2, 12($sp)     // val
    st  $2, 8($sp)      // sum
    addiu $3, $sp, 28
    st  $3, 4($sp)      // arg_ptr = 2nd argument = &arg[1], 
                // since &arg[0] = 24($sp)
    st  $2, 16($sp)
  $BB0_1:                                 # =>This Inner Loop Header: Depth=1
    ld  $2, 20($sp)
    ld  $3, 16($sp)
    cmp $3, $2        // compare(i, amount)
    jge $BB0_4
    jmp $BB0_2
  $BB0_2:                                 #   in Loop: Header=BB0_1 Depth=1 
                // i < amount
    ld  $2, 4($sp)
    addiu $3, $2, 4   // arg_ptr  + 4
    st  $3, 4($sp)  
    ld  $2, 0($2)     // *arg_ptr
    st  $2, 12($sp)
    ld  $3, 8($sp)      // sum
    add $2, $3, $2      // sum += *arg_ptr
    st  $2, 8($sp)
  # BB#3:                                 #   in Loop: Header=BB0_1 Depth=1
                // i >= amount
    ld  $2, 16($sp)
    addiu $2, $2, 1   // i++
    st  $2, 16($sp)
    jmp $BB0_1
  $BB0_4:
    addiu $sp, $sp, 24
    ret $lr
    .set  macro
    .set  reorder
    .end  _Z5sum_iiz
  $tmp1:
    .size _Z5sum_iiz, ($tmp1)-_Z5sum_iiz
  
    .globl  _Z11test_varargv
    .align  2
    .type _Z11test_varargv,@function
    .ent  _Z11test_varargv                    # @_Z11test_varargv
  _Z11test_varargv:
    .frame  $sp,88,$lr
    .mask   0x00004000,-4
    .set  noreorder
    .cpload $t9
    .set  nomacro
  # BB#0:
    addiu $sp, $sp, -88
    st  $lr, 84($sp)            # 4-byte Folded Spill
    .cprestore  32
    addiu $2, $zero, 0
    st  $2, 80($sp)
    addiu $3, $zero, 5
    st  $3, 24($sp)
    addiu $3, $zero, 4
    st  $3, 20($sp)
    addiu $3, $zero, 3
    st  $3, 16($sp)
    addiu $3, $zero, 2
    st  $3, 12($sp)
    addiu $3, $zero, 1
    st  $3, 8($sp)
    st  $2, 4($sp)
    addiu $2, $zero, 6
    st  $2, 0($sp)
    ld  $t9, %call16(_Z5sum_iiz)($gp)
    jalr  $t9
    ld  $gp, 32($sp)
    st  $2, 76($sp)
    ld  $lr, 84($sp)            # 4-byte Folded Reload
    addiu $sp, $sp, 88
    ret $lr
    .set  macro
    .set  reorder
    .end  _Z11test_varargv
  $tmp4:
    .size _Z11test_varargv, ($tmp4)-_Z11test_varargv


The analysis of output ch9_3.cpu0.s as above in comment. 
As above code, in # BB#0, we get the first argument **“amount”** from 
**“ld $2, 24($sp)”** since the stack size of the callee function 
**“_Z5sum_iiz()”** is 24. And then set argument pointer, arg_ptr, to 28($sp), 
&arg[1]. 
Next, check i < amount in block $BB0_1. If  i < amount, than enter into $BB0_2. 
In $BB0_2, it do sum += \*arg_ptr as well as arg_ptr+=4.
In # BB#3, do i+=1.

To support variable number of arguments, the following code needed to 
add in Chapter9_4/. 
The ch9_3_2.cpp is C++ template example code, it can be translated into cpu0 
backend code too.

.. rubric:: lbdex/Chapter9_4/Cpu0ISelLowering.h
.. code-block:: c++

    class Cpu0TargetLowering : public TargetLowering  {
    ...
    private:
      ...
      SDValue LowerVASTART(SDValue Op, SelectionDAG &DAG) const;
      ...
    }

.. rubric:: lbdex/Chapter9_4/Cpu0ISelLowering.cpp
.. code-block:: c++

  Cpu0TargetLowering::
  Cpu0TargetLowering(Cpu0TargetMachine &TM)
    : TargetLowering(TM, new Cpu0TargetObjectFile()),
    Subtarget(&TM.getSubtarget<Cpu0Subtarget>()) {
    ...
    setOperationAction(ISD::VASTART,            MVT::Other, Custom);
    ...
    // Support va_arg(): variable numbers (not fixed numbers) of arguments 
    //  (parameters) for function all
    setOperationAction(ISD::VAARG,             MVT::Other, Expand);
    setOperationAction(ISD::VACOPY,            MVT::Other, Expand);
    setOperationAction(ISD::VAEND,             MVT::Other, Expand);
    ...
  }
  ...
  
  SDValue Cpu0TargetLowering::
  LowerOperation(SDValue Op, SelectionDAG &DAG) const
  {
    switch (Op.getOpcode())
    {
    ...
    case ISD::VASTART:            return LowerVASTART(Op, DAG);
    }
    return SDValue();
  }
  
  ...
  SDValue Cpu0TargetLowering::LowerVASTART(SDValue Op, SelectionDAG &DAG) const {
    MachineFunction &MF = DAG.getMachineFunction();
    Cpu0FunctionInfo *FuncInfo = MF.getInfo<Cpu0FunctionInfo>();
  
    SDLoc DL = SDLoc(Op);
    SDValue FI = DAG.getFrameIndex(FuncInfo->getVarArgsFrameIndex(),
                   getPointerTy());
  
    // vastart just stores the address of the VarArgsFrameIndex slot into the
    // memory location argument.
    const Value *SV = cast<SrcValueSDNode>(Op.getOperand(2))->getValue();
    return DAG.getStore(Op.getOperand(0), DL, FI, Op.getOperand(1),
              MachinePointerInfo(SV), false, false, 0);
  }
  ...
  SDValue
  Cpu0TargetLowering::LowerFormalArguments(SDValue Chain,
                       CallingConv::ID CallConv,
                       bool isVarArg,
                      const SmallVectorImpl<ISD::InputArg> &Ins,
                       SDLoc DL, SelectionDAG &DAG,
                       SmallVectorImpl<SDValue> &InVals)
                        const {
    ...
    if (isVarArg) {
      unsigned NumOfRegs = 0;
      int FirstRegSlotOffset = 0; // offset of $a0's slot.
      unsigned RegSize = Cpu0::CPURegsRegClass.getSize();
      int RegSlotOffset = FirstRegSlotOffset + ArgLocs.size() * RegSize;

      // Offset of the first variable argument from stack pointer.
      int FirstVaArgOffset;

      FirstVaArgOffset = RegSlotOffset;

      // Record the frame index of the first variable argument
      // which is a value necessary to VASTART.
      LastFI = MFI->CreateFixedObject(RegSize, FirstVaArgOffset, true);
      Cpu0FI->setVarArgsFrameIndex(LastFI);
    }
    ...
  }


.. rubric:: lbdex/InputFiles/ch9_3_2.cpp
.. literalinclude:: ../lbdex/InputFiles/ch9_3_2.cpp
    :start-after: /// start

Mips qemu reference [#]_, you can download and run it with gcc to verify the 
result with printf() function at this point. 
We will verify the code correction in chapter 
"Run backend" through the CPU0 Verilog language machine.


Dynamic stack allocation support
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Even though C language very rare to use dynamic stack allocation, there are
languages use it frequently. The following C example code use it.

Chapter9_4 support dynamic stack allocation with the following code added.

.. rubric:: lbdex/Chapter9_4/Cpu0FrameLowering.cpp
.. code-block:: c++

  void Cpu0FrameLowering::emitPrologue(MachineFunction &MF) const {
    ...
    unsigned FP = Cpu0::FP;
    unsigned ZERO = Cpu0::ZERO;
    unsigned ADDu = Cpu0::ADDu;
    ...
    // if framepointer enabled, set it to point to the stack pointer.
    if (hasFP(MF)) {
      // Insert instruction "move $fp, $sp" at this location.
      BuildMI(MBB, MBBI, dl, TII.get(ADDu), FP).addReg(SP).addReg(ZERO);
  
      // emit ".cfi_def_cfa_register $fp"
      MCSymbol *SetFPLabel = MMI.getContext().CreateTempSymbol();
      BuildMI(MBB, MBBI, dl,
              TII.get(TargetOpcode::PROLOG_LABEL)).addSym(SetFPLabel);
      DstML = MachineLocation(FP);
      SrcML = MachineLocation(MachineLocation::VirtualFP);
      Moves.push_back(MachineMove(SetFPLabel, DstML, SrcML));
    }
    ...
  }
  
  void Cpu0FrameLowering::emitEpilogue(MachineFunction &MF,
                                   MachineBasicBlock &MBB) const {
    ...
    unsigned FP = Cpu0::FP;
    unsigned ZERO = Cpu0::ZERO;
    unsigned ADDu = Cpu0::ADDu;
    ...
  
    // if framepointer enabled, restore the stack pointer.
    if (hasFP(MF)) {
      // Find the first instruction that restores a callee-saved register.
      MachineBasicBlock::iterator I = MBBI;
  
      for (unsigned i = 0; i < MFI->getCalleeSavedInfo().size(); ++i)
        --I;
  
      // Insert instruction "move $sp, $fp" at this location.
      BuildMI(MBB, I, dl, TII.get(ADDu), SP).addReg(FP).addReg(ZERO);
    } // lbd document - mark - emitEpilogue() if (hasFP(MF))
    ...
  }

.. rubric:: lbdex/Chapter9_4/Cpu0ISelLowering.cpp
.. code-block:: c++

  Cpu0TargetLowering::
  Cpu0TargetLowering(Cpu0TargetMachine &TM)
    : TargetLowering(TM, new Cpu0TargetObjectFile()),
      Subtarget(&TM.getSubtarget<Cpu0Subtarget>()) {
    ...
    setOperationAction(ISD::DYNAMIC_STACKALLOC, MVT::i32,  Expand);
    ...
    setStackPointerRegisterToSaveRestore(Cpu0::SP);
    ...
  }

.. rubric:: lbdex/Chapter9_4/Cpu0RegisterInfo.cpp
.. code-block:: c++

  // pure virtual method
  BitVector Cpu0RegisterInfo::
  getReservedRegs(const MachineFunction &MF) const {
    ...
    // Reserve FP if this function should have a dedicated frame pointer register.
    if (MF.getTarget().getFrameLowering()->hasFP(MF)) {
      Reserved.set(Cpu0::FP);
    }
    ...
  }

Run Chapter9_4 with ch9_4.cpp will get the following correct result.

.. code-block:: bash

  118-165-72-242:InputFiles Jonathan$ clang -I/Applications/Xcode.app/Contents/
  Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.9.sdk/usr/include/ 
  -c ch9_4.cpp -emit-llvm -o ch9_4.bc
  118-165-72-242:InputFiles Jonathan$ llvm-dis ch9_4.bc -o ch9_4.ll
  118-165-72-242:InputFiles Jonathan$ cat ch9_4.ll
  ; ModuleID = 'ch9_4.bc'
  target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-
  f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:
  32:64-S128"
  target triple = "x86_64-apple-macosx10.8.0"
  
  define i32 @_Z5sum_iiiiiii(i32 %x1, i32 %x2, i32 %x3, i32 %x4, i32 %x5, i32 %x6)
   nounwind uwtable ssp {
    ...
    %10 = alloca i8, i64 %9	// int *b = (int*)alloca(sizeof(int) * x1);
    %11 = bitcast i8* %10 to i32*
    store i32* %11, i32** %b, align 8
    %12 = load i32** %b, align 8
    store i32 1111, i32* %12, align 4	// *b = 1111;
    ...
  }
  ...

  118-165-72-242:InputFiles Jonathan$ /Users/Jonathan/llvm/test/cmake_debug_build/
  bin/Debug/llc -march=cpu0 -relocation-model=pic -filetype=asm ch9_4.bc -o 
  ch9_4.cpu0.s
  118-165-72-242:InputFiles Jonathan$ cat ch9_4.cpu0.s 
  ...
  _Z10weight_sumiiiiii:
  	.cfi_startproc
  	.frame	$fp,80,$lr
  	.mask 	0x00004080,-4
  	.set	noreorder
  	.cpload	$t9
  	.set	nomacro
  # BB#0:
  	addiu	$sp, $sp, -80
  $tmp6:
  	.cfi_def_cfa_offset 80
  	st	$lr, 76($sp)            # 4-byte Folded Spill
  	st	$7, 72($sp)             # 4-byte Folded Spill
  $tmp7:
  	.cfi_offset 14, -4
  $tmp8:
  	.cfi_offset 7, -8
  	add	$fp, $sp, $zero
  $tmp9:
  	.cfi_def_cfa_register 11
  	.cprestore	24
  	ld	$7, %got(__stack_chk_guard)($gp)
  	ld	$2, 0($7)
  	st	$2, 68($fp)
  	ld	$2, 80($fp)
  	st	$2, 64($fp)
  	ld	$2, 84($fp)
  	st	$2, 60($fp)
  	ld	$2, 88($fp)
  	st	$2, 56($fp)
  	ld	$2, 92($fp)
  	st	$2, 52($fp)
  	ld	$2, 96($fp)
  	st	$2, 48($fp)
  	ld	$2, 100($fp)
  	st	$2, 44($fp)
  	ld	$2, 64($fp)	// int *b = (int*)alloca(sizeof(int) * x1);
  	shl	$2, $2, 2
  	addiu	$2, $2, 7
  	addiu	$3, $zero, -8
  	and	$2, $2, $3
  	subu	$2, $sp, $2
  	add	$sp, $zero, $2	// set sp to the bottom of alloca area
  	st	$2, 40($fp)
  	addiu	$3, $zero, 1111
  	st	$3, 0($2)
  	ld	$2, 64($fp)
  	ld	$3, 60($fp)
  	ld	$4, 56($fp)
  	ld	$5, 52($fp)
  	ld	$t9, 48($fp)
  	ld	$t0, 44($fp)
  	st	$t0, 20($sp)
  	shl	$t9, $t9, 1
  	st	$t9, 16($sp)
  	st	$5, 12($sp)
  	st	$4, 8($sp)
  	st	$3, 4($sp)
  	addiu	$3, $zero, 6
  	mul	$2, $2, $3
  	st	$2, 0($sp)
  	ld	$t9, %call16(_Z3sumiiiiii)($gp)
  	jalr	$t9
  	ld	$gp, 24($fp)
  	st	$2, 36($fp)
  	ld	$3, 0($7)
  	ld	$4, 68($fp)
  	bne	$3, $4, $BB1_2
  # BB#1:                                 # %SP_return
  	add	$sp, $fp, $zero
  	ld	$7, 72($sp)             # 4-byte Folded Reload
  	ld	$lr, 76($sp)            # 4-byte Folded Reload
  	addiu	$sp, $sp, 80
  	ret	$2
  $BB1_2:                                 # %CallStackCheckFailBlk
  	ld	$t9, %call16(__stack_chk_fail)($gp)
  	jalr	$t9
  	ld	$gp, 24($fp)
  	.set	macro
  	.set	reorder
  	.end	_Z10weight_sumiiiiii
  $tmp10:
  	.size	_Z10weight_sumiiiiii, ($tmp10)-_Z10weight_sumiiiiii
  	.cfi_endproc
  ...

As you can see, the dynamic stack allocation needs frame pointer register **fp**
support. As :num:`Figure #funccall-f4`, the sp is adjusted to (sp - 56) when it 
entered the function as usual by instruction **addiu $sp, $sp, -56**. 
Next, the fp is set to sp where is the position just above alloca() spaces area 
when meet instruction **addu $fp, $sp, $zero**. 
After that, the sp is changed to the area just below of alloca().
Remind, the alloca() area which the b point to, 
**"*b = (int*)alloca(sizeof(int) * x1)"**, is 
allocated at run time since the spaces is variable size which depend on x1 
variable and cannot be calculated at link time. 

:num:`Figure #funccall-f5` depicted how the stack pointer changes back to the 
caller stack bottom. As above, the **fp** is set to the address just above of 
alloca(). 
The first step is changing the sp to fp by instruction **addu $sp, $fp, $zero**.
Next, sp is changed back to caller stack bottom by instruction 
**addiu $sp, $sp, 56**.

.. _funccall-f4:
.. figure:: ../Fig/funccall/4.png
    :height: 279 px
    :width: 535 px
    :scale: 100 %
    :align: center

    Frame pointer changes when enter function

.. _funccall-f5:
.. figure:: ../Fig/funccall/5.png
    :height: 265 px
    :width: 476 px
    :scale: 100 %
    :align: center

    Stack pointer changes when exit function
    
.. _funccall-f6:
.. figure:: ../Fig/funccall/6.png
    :height: 394 px
    :width: 539 px
    :scale: 100 %
    :align: center

    fp and sp access areas

Use fp to keep the old stack pointer value is not the only solution. 
Actually, we can keep the alloca() spaces size on a specific memory address 
and the sp can back to the the old sp by add the alloca() spaces size. 
Most ABI like Mips
and ARM access the above area of alloca() by fp and the below area of alloca()
by sp, as :num:`Figure #funccall-f6` depicted. The reason for this definition 
is the speed for local variable access. Since the RISC CPU use immediate offset
for load and store as below, using fp and sp for access both areas of
local variables have better performance compare to use the sp only.

.. code-block:: bash

  	ld	$2, 64($fp)
  	st	$3, 4($sp)
  	
Cpu0 use fp and sp to access the above and below areas of alloca() too. 
As ch9_4.cpu0.s, it access local variable (above of alloca()) by fp offset
and outgoing arguments (below of alloca()) by sp offset.



Summary of this chapter
------------------------

Until now, we have 6,000 lines of source code around in the end of this chapter. 
The cpu0 backend code now can take care the integer function call and control 
statement just like the llvm front end tutorial example code. 
Look back the chapter of “Back end structure”, there are 3,100 lines of source 
code with taking three instructions only. 
With this 95% more of code, it can translate tens of instructions, global 
variable, control flow statement and function call.
Now the cpu0 backend is not just a toy. 
It can translate some of the C++ OOP language into Cpu0 instructions without 
much effort in backend.
Because the most complex things in language, such as C++ syntex, is handled by 
front end. 
LLVM is a real structure following the compiler theory, any backend of LLVM can 
benefit from this structure.
The best part of 3 tiers compiler structure is the backend will grow up 
automatically through the front end support languages more and more if the 
front end has not add any new IR for a new language.


.. [#] http://jonathan2251.github.io/lbd/ctrlflow.html#risc-cpu-knowledge

.. [#] https://www.dropbox.com/sh/2pkh1fewlq2zag9/OHnrYn2nOs/doc/MIPSproAssemblyLanguageProgrammerGuide 

.. [#] http://www.linux-mips.org/pub/linux/mips/doc/ABI/mipsabi.pdf

.. [#] http://jonathan2251.github.io/lbd/llvmstructure.html#target-registration

.. [#] http://jonathan2251.github.io/lbd/globalvar.html#global-variable

.. [#] http://en.wikipedia.org/wiki/Tail_call

.. [#] http://developer.mips.com/clang-llvm/

.. [#] section "4.5.1 Calling Conventions" of tricore_llvm.pdf
