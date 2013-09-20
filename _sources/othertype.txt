.. _sec-othertypesupport:

Other data type
=================

Until now, we only handle the type int and long of 32 bits long. 
This chapter introduce other type such as pointer, char, long long which are not
32 bits size.
 
 
Local variable pointer
-----------------------

To support pointer to local variable, add this code fragment in 
Cpu0InstrInfo.td and Cpu0InstPrinter.cpp as follows,

.. rubric:: LLVMBackendTutorialExampleCode/Chapter7_1/Cpu0InstrInfo.td
.. code-block:: c++

  def mem_ea : Operand<i32> {
    let PrintMethod = "printMemOperandEA";
    let MIOperandInfo = (ops CPURegs, simm16);
    let EncoderMethod = "getMemEncoding";
  }
  ...
  class EffectiveAddress<string instr_asm, RegisterClass RC, Operand Mem> :
    FMem<0x09, (outs RC:$ra), (ins Mem:$addr),
       instr_asm, [(set RC:$ra, addr:$addr)], IIAlu>;
  ...
  // FrameIndexes are legalized when they are operands from load/store
  // instructions. The same not happens for stack address copies, so an
  // add op with mem ComplexPattern is used and the stack address copy
  // can be matched. It's similar to Sparc LEA_ADDRi
  def LEA_ADDiu : EffectiveAddress<"addiu\t$ra, $addr", CPURegs, mem_ea> {
    let isCodeGenOnly = 1;
  }
  
.. rubric:: LLVMBackendTutorialExampleCode/Chapter7_1/Cpu0InstPrinter.td
.. code-block:: c++

  void Cpu0InstPrinter::
  printMemOperandEA(const MCInst *MI, int opNum, raw_ostream &O) {
    // when using stack locations for not load/store instructions
    // print the same way as all normal 3 operand instructions.
    printOperand(MI, opNum, O);
    O << ", ";
    printOperand(MI, opNum+1, O);
    return;
  }

Run ch7_1.cpp with code Chapter7_1/ which support pointer to local variable, 
will get result as follows,

.. rubric:: LLVMBackendTutorialExampleCode/InputFiles/ch7_1.cpp
.. literalinclude:: ../LLVMBackendTutorialExampleCode/InputFiles/ch7_1.cpp
    :start-after: /// start

.. code-block:: bash

  118-165-66-82:InputFiles Jonathan$ clang -target mips-unknown-linux-gnu -c 
  ch7_1.cpp -emit-llvm -o ch7_1.bc
  118-165-66-82:InputFiles Jonathan$ /Users/Jonathan/llvm/test/cmake_
  debug_build/bin/Debug/llc -march=cpu0 -relocation-model=pic -filetype=asm 
    ch7_1.bc -o ch7_1.cpu0.s
    118-165-66-82:InputFiles Jonathan$ cat ch7_1.cpu0.s 
	  .section .mdebug.abi32
	  .previous
	  .file	"ch7_1.bc"
	  .text
	  .globl	_Z18test_local_pointerv
	  .align	2
	  .type	_Z18test_local_pointerv,@function
	  .ent	_Z18test_local_pointerv # @_Z18test_local_pointerv
  _Z18test_local_pointerv:
	  .cfi_startproc
	  .frame	$fp,16,$lr
	  .mask 	0x00000800,-4
	  .set	noreorder
	  .set	nomacro
  # BB#0:                                 # %entry
	  addiu	$sp, $sp, -16
  $tmp3:
	  .cfi_def_cfa_offset 16
	  st	$fp, 12($sp)            # 4-byte Folded Spill
  $tmp4:
	  .cfi_offset 11, -4
	  addu	$fp, $sp, $zero
  $tmp5:
	  .cfi_def_cfa_register 11
	  addiu	$2, $zero, 3
	  st	$2, 8($fp)
	  addiu	$2, $fp, 8
	  st	$2, 0($fp)
	  ld	$2, 8($fp)
	  addu	$sp, $fp, $zero
	  ld	$fp, 12($sp)            # 4-byte Folded Reload
	  addiu	$sp, $sp, 16
	  ret	$lr
	  .set	macro
	  .set	reorder
	  .end	_Z18test_local_pointerv
  $tmp6:
	  .size	_Z18test_local_pointerv, ($tmp6)-_Z18test_local_pointerv
	  .cfi_endproc


char, short int and bool
--------------------------

To support signed/unsigned char and short int, we add the following code to 
Chapter7_1/.

.. rubric:: LLVMBackendTutorialExampleCode/Chapter7_1/Cpu0InstrInfo.td
.. code-block:: c++

  def sextloadi16_a   : AlignedLoad<sextloadi16>;
  def zextloadi16_a   : AlignedLoad<zextloadi16>;
  def extloadi16_a    : AlignedLoad<extloadi16>;
  ...
  def truncstorei16_a : AlignedStore<truncstorei16>;
  ...
  defm LB     : LoadM32<0x03, "lb",  sextloadi8>;
  defm LBu    : LoadM32<0x04, "lbu", zextloadi8>;
  defm SB     : StoreM32<0x05, "sb", truncstorei8>;
  defm LH     : LoadM32<0x06, "lh",  sextloadi16_a>;
  defm LHu    : LoadM32<0x07, "lhu", zextloadi16_a>;
  defm SH     : StoreM32<0x08, "sh", truncstorei16_a>;

Run Chapter7_1/ with ch7_2.cpp will get the following result.

.. rubric:: LLVMBackendTutorialExampleCode/InputFiles/ch7_2.cpp
.. literalinclude:: ../LLVMBackendTutorialExampleCode/InputFiles/ch7_2.cpp
    :start-after: /// start

.. code-block:: bash

  118-165-64-245:InputFiles Jonathan$ clang -target mips-unknown-linux-gnu -c 
  ch7_3.cpp -emit-llvm -o ch7_2.bc
  118-165-64-245:InputFiles Jonathan$ /Users/Jonathan/llvm/test/cmake_debug_build/
  bin/Debug/llc -march=cpu0 -relocation-model=pic -filetype=asm ch7_2.bc -o -
    .section .mdebug.abi32
    .previous
    .file "ch7_2.bc"
    .text
    .globl  _Z9test_charv
    .align  2
    .type _Z9test_charv,@function
    .ent  _Z9test_charv           # @_Z9test_charv
  _Z9test_charv:
    .frame  $fp,24,$lr
    .mask   0x00000000,0
    .set  noreorder
    .cpload $t9
    .set  nomacro
  # BB#0:                                 # %entry
    addiu $sp, $sp, -24
    lui $2, %got_hi(b)
    addu  $2, $2, $gp
    ld  $2, %got_lo(b)($2)
    lbu $3, 1($2)
    sb  $3, 20($fp)
    lbu $2, 1($2)
    sb  $2, 16($fp)
    ld  $2, %got($_ZZ9test_charvE5date1)($gp)
    addiu $2, $2, %lo($_ZZ9test_charvE5date1)
    lhu $3, 4($2)
    shl $3, $3, 16
    lhu $4, 6($2)
    or  $3, $3, $4
    st  $3, 12($fp) // store hour, minute and second on 12($sp)
    lhu $3, 2($2)
    lhu $2, 0($2)
    shl $2, $2, 16
    or  $2, $2, $3
    st  $2, 8($fp)    // store year, month and day on 8($sp)  
    lbu $2, 10($fp)   // m = date1.month;
    sb  $2, 4($fp)
    lbu $2, 14($fp)   // s = date1.second;
    sb  $2, 0($fp)
    addiu $sp, $sp, 24
    ret $lr
    .set  macro
    .set  reorder
    .end  _Z9test_charv
  $tmp1:
    .size _Z9test_charv, ($tmp1)-_Z9test_charv
  
    .type b,@object               # @b
    .data
    .globl  b
  b:
    .asciz   "abc"
    .size b, 4
  
    .type $_ZZ9test_charvE5date1,@object # @_ZZ9test_charvE5date1
    .section  .rodata.cst8,"aM",@progbits,8
    .align  1
  $_ZZ9test_charvE5date1:
    .2byte  2012                    # 0x7dc
    .byte 11                      # 0xb
    .byte 25                      # 0x19
    .byte 9                       # 0x9
    .byte 40                      # 0x28
    .byte 15                      # 0xf
    .space  1
    .size $_ZZ9test_charvE5date1, 8


To support load bool type, the following code added.

.. rubric:: LLVMBackendTutorialExampleCode/Chapter7_1/Cpu0ISelLowering.cpp
.. code-block:: c++

  Cpu0TargetLowering::
  Cpu0TargetLowering(Cpu0TargetMachine &TM)
    : TargetLowering(TM, new Cpu0TargetObjectFile()),
      Subtarget(&TM.getSubtarget<Cpu0Subtarget>()) {
    ...
    // Cpu0 does not have i1 type, so use i32 for
    // setcc operations results (slt, sgt, ...).
    setBooleanContents(ZeroOrOneBooleanContent);
    setBooleanVectorContents(ZeroOrNegativeOneBooleanContent);

    // Load extented operations for i1 types must be promoted
    setLoadExtAction(ISD::EXTLOAD,  MVT::i1,  Promote);
    setLoadExtAction(ISD::ZEXTLOAD, MVT::i1,  Promote);
    setLoadExtAction(ISD::SEXTLOAD, MVT::i1,  Promote);
    ...
  }

Above code setLoadExtAction() are work enough. The setBooleanContents() purpose
as following, but I don't know it well. Without it, the ch7_3.ll still works 
as below. 
The IR input file ch7_3.ll is used in testing here since the c++ version
need flow control which is not support here. File ch_run_backend.cpp include the
test fragment as below.

.. rubric:: include/llvm/Target/TargetLowering.h
.. code-block:: c++

    enum BooleanContent { // How the target represents true/false values.
      UndefinedBooleanContent,    // Only bit 0 counts, the rest can hold garbage.
      ZeroOrOneBooleanContent,        // All bits zero except for bit 0.
      ZeroOrNegativeOneBooleanContent // All bits equal to bit 0.
    };
  ...
  protected:
    /// setBooleanContents - Specify how the target extends the result of a
    /// boolean value from i1 to a wider type.  See getBooleanContents.
    void setBooleanContents(BooleanContent Ty) { BooleanContents = Ty; }
    /// setBooleanVectorContents - Specify how the target extends the result
    /// of a vector boolean value from a vector of i1 to a wider type.  See
    /// getBooleanContents.
    void setBooleanVectorContents(BooleanContent Ty) {
      BooleanVectorContents = Ty;
    }

.. rubric:: LLVMBackendTutorialExampleCode/InputFiles/ch7_3.ll
.. literalinclude:: ../LLVMBackendTutorialExampleCode/InputFiles/ch7_3.ll
    :start-after: /// start

.. code-block:: bash

    118-165-64-245:InputFiles Jonathan$ /Users/Jonathan/llvm/test/cmake_debug_build/
    bin/Debug/llc -march=cpu0 -relocation-model=pic -filetype=asm ch7_3.ll -o -

    .section .mdebug.abi32
    .previous
    .file "ch7_3.ll"
    .text
    .globl  verify_load_bool
    .align  2
    .type verify_load_bool,@function
    .ent  verify_load_bool        # @verify_load_bool
  verify_load_bool:
    .cfi_startproc
    .frame  $sp,8,$lr
    .mask   0x00000000,0
    .set  noreorder
    .set  nomacro
  # BB#0:                                 # %entry
    addiu $sp, $sp, -8
  $tmp1:
    .cfi_def_cfa_offset 8
    addiu $2, $zero, 1
    sb  $2, 7($sp)
    addiu $sp, $sp, 8
    ret $lr
    .set  macro
    .set  reorder
    .end  verify_load_bool
  $tmp2:
    .size verify_load_bool, ($tmp2)-verify_load_bool
    .cfi_endproc

.. rubric:: LLVMBackendTutorialExampleCode/InputFiles/ch_run_backend.cpp
.. code-block:: c++

  ...
  bool test_load_bool()
  {
    int a = 1;

    if (a < 0)
      return false;

    return true;
  }


long long
----------

Cpu0 borrow the Mips ABI which long is 32-bits and long long is 64-bits for C 
language type. To support long long, we add the following code to 
Chapter7_1/.

.. rubric:: LLVMBackendTutorialExampleCode/Chapter7_1/Cpu0ISelDAGToDAG.cpp
.. code-block:: c++

  /// Select instructions not customized! Used for
  /// expanded, promoted and normal instructions
  SDNode* Cpu0DAGToDAGISel::Select(SDNode *Node) {
    unsigned Opcode = Node->getOpcode();
    ...
    switch(Opcode) {
    default: break;
  
    case ISD::SUBE:
    case ISD::ADDE: {
      SDValue InFlag = Node->getOperand(2), CmpLHS;
      unsigned Opc = InFlag.getOpcode(); (void)Opc;
      assert(((Opc == ISD::ADDC || Opc == ISD::ADDE) ||
              (Opc == ISD::SUBC || Opc == ISD::SUBE)) &&
             "(ADD|SUB)E flag operand must come from (ADD|SUB)C/E insn");
  
      unsigned MOp;
      if (Opcode == ISD::ADDE) {
        CmpLHS = InFlag.getValue(0);
        MOp = Cpu0::ADDu;
      } else {
        CmpLHS = InFlag.getOperand(0);
        MOp = Cpu0::SUBu;
      }
  
      SDValue Ops[] = { CmpLHS, InFlag.getOperand(1) };
  
      SDValue LHS = Node->getOperand(0);
      SDValue RHS = Node->getOperand(1);
  
      EVT VT = LHS.getValueType();
      SDNode *StatusWord = CurDAG->getMachineNode(Cpu0::CMP, dl, VT, Ops);
      SDValue Constant1 = CurDAG->getTargetConstant(1, VT);
      SDNode *Carry = CurDAG->getMachineNode(Cpu0::ANDi, dl, VT, 
                                             SDValue(StatusWord,0), Constant1);
      SDNode *AddCarry = CurDAG->getMachineNode(Cpu0::ADDu, dl, VT,
                                                SDValue(Carry,0), RHS);
  
      return CurDAG->SelectNodeTo(Node, MOp, VT, MVT::Glue,
                                  LHS, SDValue(AddCarry,0));
    }
  
    /// Mul with two results
    case ISD::SMUL_LOHI:
    case ISD::UMUL_LOHI: {
      if (NodeTy == MVT::i32)
        MultOpc = (Opcode == ISD::UMUL_LOHI ? Cpu0::MULTu : Cpu0::MULT);
  
      std::pair<SDNode*, SDNode*> LoHi = SelectMULT(Node, MultOpc, dl, NodeTy,
                                                    true, true);
  
      if (!SDValue(Node, 0).use_empty())
        ReplaceUses(SDValue(Node, 0), SDValue(LoHi.first, 0));
  
      if (!SDValue(Node, 1).use_empty())
        ReplaceUses(SDValue(Node, 1), SDValue(LoHi.second, 0));
  
      return NULL;
    }
    ...
  }


Run Chapter7_1 with ch7_4.cpp to get the result as follows,

.. rubric:: LLVMBackendTutorialExampleCode/InputFiles/ch7_4.cpp
.. literalinclude:: ../LLVMBackendTutorialExampleCode/InputFiles/ch7_4.cpp
    :start-after: /// start

.. code-block:: bash

  1-160-134-62:InputFiles Jonathan$ clang -target mips-unknown-linux-gnu -c 
  ch7_4.cpp -emit-llvm -o ch7_4.bc
  1-160-134-62:InputFiles Jonathan$ /Users/Jonathan/llvm/test/cmake_debug_build/
  bin/Debug/llc -march=cpu0 -relocation-model=pic -filetype=asm ch7_4.bc -o -
    .section .mdebug.abi32
    .previous
    .file "ch7_4.bc"
    .text
    .globl  _Z13test_longlongv
    .align  2
    .type _Z13test_longlongv,@function
    .ent  _Z13test_longlongv      # @_Z13test_longlongv
  _Z13test_longlongv:
    .frame  $fp,72,$lr
    .mask   0x00000980,-4
    .set  noreorder
    .set  nomacro
  # BB#0:                                 # %entry
    addiu $sp, $sp, -72
    st  $fp, 68($sp)            # 4-byte Folded Spill
    st  $8, 64($sp)             # 4-byte Folded Spill
    st  $7, 60($sp)             # 4-byte Folded Spill
    addu  $fp, $sp, $zero
    addiu $2, $zero, 2
    st  $2, 52($fp)
    addiu $2, $zero, 3
    st  $2, 48($fp)
    addiu $2, $zero, 1
    st  $2, 44($fp)
    st  $2, 40($fp)
    lui $2, 768
    ori $2, $2, 4096
    st  $2, 36($fp)
    lui $2, 512
    ori $2, $2, 4096
    st  $2, 32($fp)
    ld  $2, 44($fp)
    ld  $3, 52($fp)
    addu  $4, $3, $2
    ld  $3, 48($fp)
    ld  $5, 40($fp)
    st  $4, 28($fp)
    cmp $sw, $4, $2
    andi  $2, $sw, 1
    addu  $2, $2, $5
    addu  $2, $3, $2
    st  $2, 24($fp)
    ld  $2, 44($fp)
    ld  $4, 52($fp)
    subu  $t9, $4, $2
    ld  $3, 48($fp)
    ld  $5, 40($fp)
    st  $t9, 20($fp)
    cmp $sw, $4, $2
    andi  $2, $sw, 1
    addu  $2, $2, $5
    subu  $2, $3, $2
    st  $2, 16($fp)
    ld  $2, 44($fp)
    ld  $3, 52($fp)
    multu $3, $2
    mflo  $t9
    mfhi  $4
    ld  $5, 48($fp)
    ld  $sw, 40($fp)
    st  $t9, 12($fp)
    mul $3, $3, $sw
    addu  $3, $4, $3
    mul $2, $5, $2
    addu  $2, $3, $2
    st  $2, 8($fp)
    ld  $2, 32($fp)
    ld  $3, 36($fp)
    mult  $3, $2
    mflo  $t9
    mfhi  $4
    st  $t9, 4($fp)
    st  $4, 0($fp)
    ld  $2, 20($fp)
    ld  $3, 28($fp)
    addu  $5, $3, $2
    ld  $8, 12($fp)
    addu  $7, $5, $8
    addu  $3, $7, $t9
    cmp $sw, $3, $t9
    andi  $t9, $sw, 1
    addu  $4, $t9, $4
    cmp $sw, $7, $8
    andi  $t9, $sw, 1
    ld  $sw, 8($fp)
    addu  $t9, $t9, $sw
    cmp $sw, $5, $2
    andi  $2, $sw, 1
    ld  $5, 16($fp)
    addu  $2, $2, $5
    ld  $5, 24($fp)
    addu  $2, $5, $2
    addu  $2, $2, $t9
    addu  $2, $2, $4
    addu  $sp, $fp, $zero
    ld  $7, 60($sp)             # 4-byte Folded Reload
    ld  $8, 64($sp)             # 4-byte Folded Reload
    ld  $fp, 68($sp)            # 4-byte Folded Reload
    addiu $sp, $sp, 72
    ret $lr
    .set  macro
    .set  reorder
    .end  _Z13test_longlongv
  $tmp3:
    .size _Z13test_longlongv, ($tmp3)-_Z13test_longlongv


float and double
-----------------

Cpu0 only has integer instructions at this point. For float operations, the clang
will call the library function to translate integer to float. This float (or 
double) function call for Cpu0 will be supported after the chapter of function 
call.


Array and struct support
-------------------------

LLVM use getelementptr to represent the array and struct type in C. 
Please reference section getelementptr of [#]_. 
For ch7_5.cpp, the llvm IR as follows,

.. rubric:: LLVMBackendTutorialExampleCode/InputFiles/ch7_5.cpp
.. literalinclude:: ../LLVMBackendTutorialExampleCode/InputFiles/ch7_5.cpp
    :start-after: /// start

.. code-block:: bash

  // ch7_5.ll
  ; ModuleID = 'ch7_5.bc'
  ...
  %struct.Date = type { i32, i32, i32 }
    
  @date = global %struct.Date { i32 2012, i32 10, i32 12 }, align 4
  @a = global [3 x i32] [i32 2012, i32 10, i32 12], align 4
    
  define i32 @main() nounwind ssp {
  entry:
    %retval = alloca i32, align 4
    %day = alloca i32, align 4
    %i = alloca i32, align 4
    store i32 0, i32* %retval
    %0 = load i32* getelementptr inbounds (%struct.Date* @date, i32 0, i32 2), 
    align 4
    store i32 %0, i32* %day, align 4
    %1 = load i32* getelementptr inbounds ([3 x i32]* @a, i32 0, i32 1), align 4
    store i32 %1, i32* %i, align 4
    ret i32 0
  }
    
Run Chapter6_1/ with ch7_5.bc on static mode will get the incorrect asm file as 
follows,

.. code-block:: bash

  1-160-134-62:InputFiles Jonathan$ /Users/Jonathan/llvm/test/cmake_debug_build/bin/
  Debug/llc -march=cpu0 -relocation-model=static -filetype=asm ch7_5.bc -o -
    .section .mdebug.abi32
    .previous
    .file "ch7_5.bc"
    .text
    .globl  _Z11test_structv
    .align  2
    .type _Z11test_structv,@function
    .ent  _Z11test_structv        # @_Z11test_structv
  _Z11test_structv:
    .frame  $fp,8,$lr
    .mask   0x00000000,0
    .set  noreorder
    .set  nomacro
  # BB#0:                                 # %entry
    addiu $sp, $sp, -8
    lui $2, %hi(date)
    addiu $2, $2, %lo(date)
      ld  $2, 0($2)   // the correct one is   ld  $2, 8($2)
    st  $2, 4($fp)
    lui $2, %hi(a)
    addiu $2, $2, %lo(a)
    ld  $2, 4($2)
    st  $2, 0($fp)
    addiu $sp, $sp, 8
    ret $lr
    .set  macro
    .set  reorder
    .end  _Z11test_structv
  $tmp1:
    .size _Z11test_structv, ($tmp1)-_Z11test_structv
  
    .type date,@object            # @date
    .data
    .globl  date
    .align  2
  date:
    .4byte  2012                    # 0x7dc
    .4byte  10                      # 0xa
    .4byte  12                      # 0xc
    .size date, 12
  
    .type a,@object               # @a
    .globl  a
    .align  2
  a:
    .4byte  2012                    # 0x7dc
    .4byte  10                      # 0xa
    .4byte  12                      # 0xc
    .size a, 12


For **“day = date.day”**, the correct one is **“ld $2, 8($2)”**, not 
**“ld $2, 0($2)”**, since date.day is offset 8(date). 
Type int is 4 bytes in cpu0, and the date.day has fields year and month before 
it. 
Let use debug option in llc to see what's wrong,

.. code-block:: bash

  jonathantekiimac:InputFiles Jonathan$ /Users/Jonathan/llvm/test/
  cmake_debug_build/bin/Debug/llc -march=cpu0 -debug -relocation-model=static 
  -filetype=asm ch6_2.bc -o ch6_2.cpu0.static.s
  ...
  === main
  Initial selection DAG: BB#0 'main:entry'
  SelectionDAG has 20 nodes:
    0x7f7f5b02d210: i32 = undef [ORD=1]
    
        0x7f7f5ac10590: ch = EntryToken [ORD=1]
    
        0x7f7f5b02d010: i32 = Constant<0> [ORD=1]
    
        0x7f7f5b02d110: i32 = FrameIndex<0> [ORD=1]
    
        0x7f7f5b02d210: <multiple use>
      0x7f7f5b02d310: ch = store 0x7f7f5ac10590, 0x7f7f5b02d010, 0x7f7f5b02d110, 
      0x7f7f5b02d210<ST4[%retval]> [ORD=1]
    
        0x7f7f5b02d410: i32 = GlobalAddress<%struct.Date* @date> 0 [ORD=2]
    
        0x7f7f5b02d510: i32 = Constant<8> [ORD=2]
    
      0x7f7f5b02d610: i32 = add 0x7f7f5b02d410, 0x7f7f5b02d510 [ORD=2]
    
      0x7f7f5b02d210: <multiple use>
    0x7f7f5b02d710: i32,ch = load 0x7f7f5b02d310, 0x7f7f5b02d610, 0x7f7f5b02d210
    <LD4[getelementptr inbounds (%struct.Date* @date, i32 0, i32 2)]> [ORD=3]
    
    0x7f7f5b02db10: i64 = Constant<4>
    
        0x7f7f5b02d710: <multiple use>
        0x7f7f5b02d710: <multiple use>
        0x7f7f5b02d810: i32 = FrameIndex<1> [ORD=4]
  
        0x7f7f5b02d210: <multiple use>
      0x7f7f5b02d910: ch = store 0x7f7f5b02d710:1, 0x7f7f5b02d710, 0x7f7f5b02d810,
       0x7f7f5b02d210<ST4[%day]> [ORD=4]
  
        0x7f7f5b02da10: i32 = GlobalAddress<[3 x i32]* @a> 0 [ORD=5]
    
        0x7f7f5b02dc10: i32 = Constant<4> [ORD=5]
    
      0x7f7f5b02dd10: i32 = add 0x7f7f5b02da10, 0x7f7f5b02dc10 [ORD=5]
    
      0x7f7f5b02d210: <multiple use>
    0x7f7f5b02de10: i32,ch = load 0x7f7f5b02d910, 0x7f7f5b02dd10, 0x7f7f5b02d210
    <LD4[getelementptr inbounds ([3 x i32]* @a, i32 0, i32 1)]> [ORD=6]
    
  ...
    
    
  Replacing.3 0x7f7f5b02dd10: i32 = add 0x7f7f5b02da10, 0x7f7f5b02dc10 [ORD=5]
    
  With: 0x7f7f5b030010: i32 = GlobalAddress<[3 x i32]* @a> + 4
    
    
  Replacing.3 0x7f7f5b02d610: i32 = add 0x7f7f5b02d410, 0x7f7f5b02d510 [ORD=2]
    
  With: 0x7f7f5b02db10: i32 = GlobalAddress<%struct.Date* @date> + 8
    
  Optimized lowered selection DAG: BB#0 'main:entry'
  SelectionDAG has 15 nodes:
    0x7f7f5b02d210: i32 = undef [ORD=1]
    
        0x7f7f5ac10590: ch = EntryToken [ORD=1]
    
        0x7f7f5b02d010: i32 = Constant<0> [ORD=1]
    
        0x7f7f5b02d110: i32 = FrameIndex<0> [ORD=1]
    
        0x7f7f5b02d210: <multiple use>
      0x7f7f5b02d310: ch = store 0x7f7f5ac10590, 0x7f7f5b02d010, 0x7f7f5b02d110, 
      0x7f7f5b02d210<ST4[%retval]> [ORD=1]
    
      0x7f7f5b02db10: i32 = GlobalAddress<%struct.Date* @date> + 8
    
      0x7f7f5b02d210: <multiple use>
    0x7f7f5b02d710: i32,ch = load 0x7f7f5b02d310, 0x7f7f5b02db10, 0x7f7f5b02d210
    <LD4[getelementptr inbounds (%struct.Date* @date, i32 0, i32 2)]> [ORD=3]
    
        0x7f7f5b02d710: <multiple use>
        0x7f7f5b02d710: <multiple use>
        0x7f7f5b02d810: i32 = FrameIndex<1> [ORD=4]
    
        0x7f7f5b02d210: <multiple use>
      0x7f7f5b02d910: ch = store 0x7f7f5b02d710:1, 0x7f7f5b02d710, 0x7f7f5b02d810,
       0x7f7f5b02d210<ST4[%day]> [ORD=4]
    
      0x7f7f5b030010: i32 = GlobalAddress<[3 x i32]* @a> + 4
    
      0x7f7f5b02d210: <multiple use>
    0x7f7f5b02de10: i32,ch = load 0x7f7f5b02d910, 0x7f7f5b030010, 0x7f7f5b02d210
    <LD4[getelementptr inbounds ([3 x i32]* @a, i32 0, i32 1)]> [ORD=6]
    
  ...


By ``llc -debug``, you can see the DAG translation process. 
As above, the DAG list 
for date.day (add GlobalAddress<[3 x i32]* @a> 0, Constant<8>) with 3 nodes is 
replaced by 1 node GlobalAddress<%struct.Date* @date> + 8. 
The DAG list for a[1] is same. 
The replacement occurs since TargetLowering.cpp::isOffsetFoldingLegal(...) 
return true in ``llc -static`` static addressing mode as below. 
In Cpu0 the **ld** instruction format is **“ld $r1, offset($r2)”** which 
meaning load $r2 address+offset to $r1. 
So, we just replace the isOffsetFoldingLegal(...) function by override 
mechanism as below.

.. rubric:: lib/CodeGen/SelectionDAG/TargetLowering.cpp
.. code-block:: c++

  bool
  TargetLowering::isOffsetFoldingLegal(const GlobalAddressSDNode *GA) const {
    // Assume that everything is safe in static mode.
    if (getTargetMachine().getRelocationModel() == Reloc::Static)
      return true;
    
    // In dynamic-no-pic mode, assume that known defined values are safe.
    if (getTargetMachine().getRelocationModel() == Reloc::DynamicNoPIC &&
       GA &&
       !GA->getGlobal()->isDeclaration() &&
       !GA->getGlobal()->isWeakForLinker())
    return true;
    
    // Otherwise assume nothing is safe.
    return false;
  }
    
.. rubric:: LLVMBackendTutorialExampleCode/Chapter7_1/Cpu0ISelLowering.cpp
.. code-block:: c++

  bool
  Cpu0TargetLowering::isOffsetFoldingLegal(const GlobalAddressSDNode *GA) const {
    // The Cpu0 target isn't yet aware of offsets.
    return false;
  }

Beyond that, we need to add the following code fragment to Cpu0ISelDAGToDAG.cpp,

.. rubric:: LLVMBackendTutorialExampleCode/Chapter7_1/Cpu0ISelDAGToDAG.cpp
.. code-block:: c++

  //  Cpu0ISelDAGToDAG.cpp
  /// ComplexPattern used on Cpu0InstrInfo
  /// Used on Cpu0 Load/Store instructions
  bool Cpu0DAGToDAGISel::
  SelectAddr(SDNode *Parent, SDValue Addr, SDValue &Base, SDValue &Offset) {
  ...
    // Addresses of the form FI+const or FI|const
    if (CurDAG->isBaseWithConstantOffset(Addr)) {
      ConstantSDNode *CN = dyn_cast<ConstantSDNode>(Addr.getOperand(1));
      if (isInt<16>(CN->getSExtValue())) {
    
        // If the first operand is a FI, get the TargetFI Node
        if (FrameIndexSDNode *FIN = dyn_cast<FrameIndexSDNode>
                                            (Addr.getOperand(0)))
          Base = CurDAG->getTargetFrameIndex(FIN->getIndex(), ValTy);
        else
          Base = Addr.getOperand(0);
    
        Offset = CurDAG->getTargetConstant(CN->getZExtValue(), ValTy);
        return true;
      }
    }
  }

Recall we have translated DAG list for date.day 
(add GlobalAddress<[3 x i32]* @a> 0, Constant<8>) into 
(add (add Cpu0ISD::Hi (Cpu0II::MO_ABS_HI), Cpu0ISD::Lo(Cpu0II::MO_ABS_LO)), 
Constant<8>) by the following code in Cpu0ISelLowering.cpp.

.. rubric:: LLVMBackendTutorialExampleCode/Chapter6_1/Cpu0ISelLowering.cpp
.. code-block:: c++

  // Cpu0ISelLowering.cpp
  SDValue Cpu0TargetLowering::LowerGlobalAddress(SDValue Op,
                                      SelectionDAG &DAG) const {
    ...
      // %hi/%lo relocation
      SDValue GAHi = DAG.getTargetGlobalAddress(GV, dl, MVT::i32, 0,
                                                Cpu0II::MO_ABS_HI);
      SDValue GALo = DAG.getTargetGlobalAddress(GV, dl, MVT::i32, 0,
                                                Cpu0II::MO_ABS_LO);
      SDValue HiPart = DAG.getNode(Cpu0ISD::Hi, dl, VTs, &GAHi, 1);
      SDValue Lo = DAG.getNode(Cpu0ISD::Lo, dl, MVT::i32, GALo);
      return DAG.getNode(ISD::ADD, dl, MVT::i32, HiPart, Lo);
    ...
  }

So, when the SelectAddr(...) of Cpu0ISelDAGToDAG.cpp is called. 
The Addr SDValue in SelectAddr(..., Addr, ...) is DAG list for date.day 
(add (add Cpu0ISD::Hi (Cpu0II::MO_ABS_HI), Cpu0ISD::Lo(Cpu0II::MO_ABS_LO)), 
Constant<8>). 
Since Addr.getOpcode() = ISD:ADD, Addr.getOperand(0) = 
(add Cpu0ISD::Hi (Cpu0II::MO_ABS_HI), Cpu0ISD::Lo(Cpu0II::MO_ABS_LO)) and 
Addr.getOperand(1).getOpcode() = ISD::Constant, the Base = SDValue 
(add Cpu0ISD::Hi (Cpu0II::MO_ABS_HI), Cpu0ISD::Lo(Cpu0II::MO_ABS_LO)) and 
Offset = Constant<8>. 
After set Base and Offset, the load DAG will translate the global address 
date.day into machine instruction **“ld $r1, 8($r2)”** in Instruction Selection 
stage.

Chapter7_1/ include these changes as above, you can run it with ch7_5.cpp to get 
the correct generated instruction **“ld $r1, 8($r2)”** for date.day access, as 
follows.


.. code-block:: bash

  ...
  ld  $2, 8($2)
  st  $2, 8($sp)
  addiu $2, $zero, %hi(a)
  shl $2, $2, 16
  addiu $2, $2, %lo(a)
  ld  $2, 4($2)


.. [#] http://llvm.org/docs/LangRef.html
