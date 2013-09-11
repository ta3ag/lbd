.. _sec-addingmoresupport:

Adding arithmetic and local pointer support
===========================================

This chapter adds more cpu0 arithmetic instructions support first.
The logic operation **“not”** support and translation in 
`section Operator “not” !`_. The `section Display llvm IR nodes with Graphviz`_ 
will show you the DAG optimization steps and their corresponding ``llc`` 
display options. 
These DAG optimization steps result can be displayed by the graphic tool of 
Graphviz which supply very useful information with graphic view. 
You will appreciate Graphviz support in debug, we think.  
The `section Local variable pointer`_ introduce you the local variable pointer 
translation.
Finally, `section Operator mod, %`_ take care the C operator %.


**+, -, \*, <<,** and **>>**
-----------------------------

Appending the following code to Cpu0InstrInfo.td and Cpu0Schedule.td in 
Chapter4_1/ to support operators **+, -, \*, <<,** and **>>**.

.. rubric:: LLVMBackendTutorialExampleCode/Chapter4_1/Cpu0InstrInfo.td
.. code-block:: c++

  def shamt     : Operand<i32>;
  ...
  // shamt field must fit in 5 bits.
  def immZExt5 : ImmLeaf<i32, [{return Imm == (Imm & 0x1f);}]>;
  ...
  // Arithmetic and logical instructions with 3 register operands.
  class ArithLogicR<bits<8> op, string instr_asm, SDNode OpNode,
      InstrItinClass itin, RegisterClass RC, bit isComm = 0>:
  FA<op, (outs RC:$ra), (ins RC:$rb, RC:$rc),
   !strconcat(instr_asm, "\t$ra, $rb, $rc"),
   [(set RC:$ra, (OpNode RC:$rb, RC:$rc))], itin> {
  let shamt = 0;
  let isCommutable = isComm;  // e.g. add rb rc =  add rc rb
  let isReMaterializable = 1;
  }
  ...
  // Shifts
  class shift_rotate_imm<bits<8> op, bits<4> isRotate, string instr_asm,
         SDNode OpNode, PatFrag PF, Operand ImmOpnd,
         RegisterClass RC>:
  FA<op, (outs RC:$ra), (ins RC:$rb, ImmOpnd:$shamt),
   !strconcat(instr_asm, "\t$ra, $rb, $shamt"),
   [(set RC:$ra, (OpNode RC:$rb, PF:$shamt))], IIAlu> {
  let rc = isRotate;
  let shamt = shamt;
  }
  
  // 32-bit shift instructions.
  class shift_rotate_imm32<bits<8> func, bits<4> isRotate, string instr_asm,
         SDNode OpNode>:
  shift_rotate_imm<func, isRotate, instr_asm, OpNode, immZExt5, shamt, CPURegs>;

  class shift_rotate_reg<bits<8> op, bits<4> isRotate, string instr_asm,
                         SDNode OpNode, RegisterClass RC>:
    FA<op, (outs RC:$ra), (ins CPURegs:$rb, RC:$rc),
       !strconcat(instr_asm, "\t$ra, $rb, $rc"),
       [(set RC:$ra, (OpNode RC:$rb, CPURegs:$rc))], IIAlu> {
    let shamt = 0;
  }
  ...
  /// Arithmetic Instructions (3-Operand, R-Type)
  ...
  def SUBu    : ArithLogicR<0x12, "subu", sub, IIAlu, CPURegs>;
  def ADD     : ArithLogicR<0x13, "add", add, IIAlu, CPURegs, 1>;
  def SUB     : ArithLogicR<0x14, "sub", sub, IIAlu, CPURegs, 1>;
  def MUL     : ArithLogicR<0x17, "mul", mul, IIImul, CPURegs, 1>;

  /// Shift Instructions
  // sra is IR node for ashr llvm IR instruction of .bc
  def ROL     : shift_rotate_imm32<0x1b, 0x01, "rol", rotl>;
  def ROR     : shift_rotate_imm32<0x1c, 0x01, "ror", rotr>;
  def SRA     : shift_rotate_imm32<0x1d, 0x00, "sra", sra>;
  def SHL     : shift_rotate_imm32<0x1e, 0x00, "shl", shl>;
  // srl is IR node for lshr llvm IR instruction of .bc
  def SHR     : shift_rotate_imm32<0x1f, 0x00, "shr", srl>;
  def SRAV    : shift_rotate_reg<0x20, 0x00, "srav", sra, CPURegs>;
  def SHLV    : shift_rotate_reg<0x21, 0x00, "shlv", shl, CPURegs>;
  def SHRV    : shift_rotate_reg<0x22, 0x00, "shrv", srl, CPURegs>;


.. rubric:: LLVMBackendTutorialExampleCode/Chapter4_1/Cpu0Schedule.td
.. code-block:: c++

  ...
  def IMULDIV : FuncUnit;
  ...
  def IIImul       : InstrItinClass;
  def IIIdiv       : InstrItinClass;
  ...
  // http://llvm.org/docs/doxygen/html/structllvm_1_1InstrStage.html 
  def Cpu0GenericItineraries : ProcessorItineraries<[ALU, IMULDIV], [], [
  ...
  InstrItinData<IIImul       , [InstrStage<17, [IMULDIV]>]>,
  InstrItinData<IIIdiv       , [InstrStage<38, [IMULDIV]>]>
  ]>;


In RISC CPU like Mips, the multiply/divide function unit and add/sub/logic unit 
are designed from two different hardware circuits, and more, their data path is 
separate. We think the cpu0 is the same even though no explanation in it's web 
site.
So, these two function units can be executed at same time (instruction level 
parallelism). Reference [#]_ for instruction itineraries.

This version can process **+, -, \*, <<,** and **>>** operators in C 
language. 
The corresponding llvm IR instructions are **add, sub, mul, 
shl, ashr**. 
The **'ashr'** instruction (arithmetic shift right) returns the first operand 
shifted to the right a specified number of bits with sign extension. 
In brief, we call **ashr** is “shift with sign extension fill”.

.. note:: **ashr**

  Example:
    <result> = ashr i32 4, 1   ; yields {i32}:result = 2
    
    <result> = ashr i8 -2, 1   ; yields {i8}:result = -1
    
    <result> = ashr i32 1, 32  ; undefined

The C operator **>>** for negative operand is dependent on implementation. 
Most compiler translate it into “shift with sign extension fill”, for example, 
Mips **sra** is the instruction. 
Following is the Micosoft web site explanation,

.. note:: **>>**, Microsoft Specific

  The result of a right shift of a signed negative quantity is implementation 
  dependent. 
  Although Microsoft C++ propagates the most-significant bit to fill vacated 
  bit positions, there is no guarantee that other implementations will do 
  likewise.

In addition to **ashr**, the other instruction “shift with zero filled” 
**lshr** in llvm (Mips implement lshr with instruction **srl**) has the 
following meaning. 

.. note:: **lshr**

  Example:
  <result> = lshr i8 -2, 1   ; yields {i8}:result = 0x7FFFFFFF 
  
In llvm, IR node **sra** is defined for ashr IR instruction, node **srl** is 
defined for lshr instruction (I don't know why don't use ashr and lshr as the 
IR node name directly). Summary as the Table: C operator >> implementation.


.. table:: C operator >> implementation

  ======================================= ======================  =====================================
  Description                             Shift with zero filled  Shift with signed extension filled
  ======================================= ======================  =====================================
  symbol in .bc                           lshr                    ashr
  symbol in IR node                       srl                     sra
  Mips instruction                        srl                     sra
  Cpu0 instruction                        shr                     sra
  signed example before x >> 1            0xfffffffe i.e. -2      0xfffffffe i.e. -2
  signed example after x >> 1             0x7fffffff i.e 2G-1     0xffffffff i.e. -1
  unsigned example before x >> 1          0xfffffffe i.e. 4G-2    0xfffffffe i.e. 4G-2
  unsigned example after x >> 1           0x7fffffff i.e 2G-1     0xffffffff i.e. 4G-1
  ======================================= ======================  =====================================
  
**lshr:** Logical SHift Right

**ashr:** Arithmetic SHift right

**srl:**  Shift Right Logically

**sra:**  Shift Right Arithmetically

**shr:**  SHift Right


If we consider the x >> 1 definition is x = x/2 for compiler implementation.
As you can see from Table: C operator >> implementation, **lshr** is failed on 
some signed value (such as -2). In the same way, **ashr** is failed on some 
unsigned value (such as 4G-2). So, in order to satisfy this definition in 
both signed and unsigned integer of x, we need these two instructions, 
**lshr** and **ashr**.

.. table:: C operator << implementation

  ======================================= ======================
  Description                             Shift with zero filled
  ======================================= ======================
  symbol in .bc                           shl
  symbol in IR node                       shl
  Mips instruction                        sll
  Cpu0 instruction                        shl
  signed example before x << 1            0x40000000 i.e. 1G
  signed example after x << 1             0x80000000 i.e -2G
  unsigned example before x << 1          0x40000000 i.e. 1G
  unsigned example after x << 1           0x80000000 i.e 2G
  ======================================= ======================

Again, consider the x << 1 definition is x = x*2. 
From Table: C operator << implementation, we see **lshr** satisfy the unsigned 
x=1G but failed on signed x=1G. It's fine since the 2G is out of 32 bits signed 
integer range (-2G ~ 2G-1). 
For the overflow case, no way to keep the correct result in register. So, any 
value in register is OK. You can check the **lshr** satisfy x = x*2 for x << 1 
when the x result is not out of range, no matter operand x is signed or unsigned 
integer.

Micorsoft implementation references as [#]_.

The sub-section "‘ashr‘ Instruction" and sub-section "‘lshr‘ Instruction" of 
[#]_.

The srav, shlv and shrv are for two virtual registers instructions while the 
sra, ... are for 1 virtual registers and 1 constant input operands.

Now, let's build Chapter4_1/ and run with input file ch4_1_1.cpp as follows,

.. rubric:: LLVMBackendTutorialExampleCode/InputFiles/ch4_1_1.cpp
.. literalinclude:: ../LLVMBackendTutorialExampleCode/InputFiles/ch4_1_1.cpp
  :start-after: // test_math()
  :linenos:

.. code-block:: bash

  118-165-78-12:InputFiles Jonathan$ clang -c ch4_1_1.cpp -emit-llvm -o ch4_1_1.bc
  118-165-78-12:InputFiles Jonathan$ llvm-dis ch4_1_1.bc -o -
  ...
  ; Function Attrs: nounwind uwtable
  define i32 @_Z9test_mathv() #0 {
  entry:
    %a = alloca i32, align 4
    %b = alloca i32, align 4
    %c = alloca i32, align 4
    %d = alloca i32, align 4
    %e = alloca i32, align 4
    %f = alloca i32, align 4
    %a1 = alloca i32, align 4
    %e1 = alloca i32, align 4
    %f1 = alloca i32, align 4
    store i32 5, i32* %a, align 4
    store i32 2, i32* %b, align 4
    store i32 0, i32* %c, align 4
    store i32 0, i32* %d, align 4
    store i32 0, i32* %f, align 4
    store i32 -5, i32* %a1, align 4
    store i32 0, i32* %e1, align 4
    store i32 0, i32* %f1, align 4
    %0 = load i32* %a, align 4
    %1 = load i32* %b, align 4
    %add = add nsw i32 %0, %1
    store i32 %add, i32* %c, align 4
    %2 = load i32* %a, align 4
    %3 = load i32* %b, align 4
    %sub = sub nsw i32 %2, %3
    store i32 %sub, i32* %d, align 4
    %4 = load i32* %a, align 4
    %5 = load i32* %b, align 4
    %mul = mul nsw i32 %4, %5
    store i32 %mul, i32* %d, align 4
    %6 = load i32* %a, align 4
    %shl = shl i32 %6, 2
    store i32 %shl, i32* %e, align 4
    %7 = load i32* %a1, align 4
    %shl1 = shl i32 %7, 2
    store i32 %shl1, i32* %e1, align 4
    %8 = load i32* %a, align 4
    %shr = ashr i32 %8, 2
    store i32 %shr, i32* %f, align 4
    %9 = load i32* %a1, align 4
    %shr2 = lshr i32 %9, 2
    store i32 %shr2, i32* %f1, align 4
    %10 = load i32* %a, align 4
    %shl3 = shl i32 1, %10
    store i32 %shl3, i32* %e, align 4
    %11 = load i32* %a1, align 4
    %shl4 = shl i32 1, %11
    store i32 %shl4, i32* %e1, align 4
    %12 = load i32* %a, align 4
    %shr5 = lshr i32 -2147483648, %12
    store i32 %shr5, i32* %f, align 4
    %13 = load i32* %b, align 4
    %14 = load i32* %a, align 4
    %shr6 = ashr i32 %13, %14
    store i32 %shr6, i32* %f1, align 4
    %15 = load i32* %c, align 4
    ret i32 %15
  }
  
  118-165-78-12:InputFiles Jonathan$ /Users/Jonathan/llvm/test/cmake_debug_build/
  bin/Debug/llc -march=cpu0 -relocation-model=pic -filetype=asm ch4_1_1.bc -o -
	  .section .mdebug.abi32
	  .previous
	  .file	"ch4_1_1.bc"
	  .text
	  .globl	_Z9test_mathv
	  .align	2
	  .type	_Z9test_mathv,@function
	  .ent	_Z9test_mathv           # @_Z9test_mathv
  _Z9test_mathv:
	  .cfi_startproc
	  .frame	$fp,40,$lr
	  .mask 	0x00000000,0
	  .set	noreorder
	  .set	nomacro
  # BB#0:                                 # %entry
	  addiu	$sp, $sp, -40
  $tmp1:
	  .cfi_def_cfa_offset 40
	  addiu	$2, $zero, 5
	  st	$2, 36($fp)
	  addiu	$2, $zero, 2
	  st	$2, 32($fp)
	  addiu	$2, $zero, 0
	  st	$2, 28($fp)
	  st	$2, 24($fp)
	  st	$2, 16($fp)
	  addiu	$3, $zero, -5
	  st	$3, 12($fp)
	  st	$2, 8($fp)
	  st	$2, 4($fp)
	  ld	$2, 32($fp)
	  ld	$3, 36($fp)
	  addu	$2, $3, $2
	  st	$2, 28($fp)
	  ld	$2, 32($fp)
	  ld	$3, 36($fp)
	  subu	$2, $3, $2
	  st	$2, 24($fp)
	  ld	$2, 32($fp)
	  ld	$3, 36($fp)
	  mul	$2, $3, $2
	  st	$2, 24($fp)
	  ld	$2, 36($fp)
	  shl	$2, $2, 2
	  st	$2, 20($fp)
	  ld	$2, 12($fp)
	  shl	$2, $2, 2
	  st	$2, 8($fp)
	  ld	$2, 36($fp)
	  sra	$2, $2, 2
	  st	$2, 16($fp)
	  ld	$2, 12($fp)
	  shr	$2, $2, 2
	  st	$2, 4($fp)
	  addiu	$2, $zero, 1
	  ld	$3, 36($fp)
	  shlv	$3, $2, $3
	  st	$3, 20($fp)
	  ld	$3, 12($fp)
	  shlv	$2, $2, $3
	  st	$2, 8($fp)
	  lui	$2, 32768
	  ld	$3, 36($fp)
	  shrv	$2, $2, $3
	  st	$2, 16($fp)
	  ld	$2, 36($fp)
	  ld	$3, 32($fp)
	  srav	$2, $3, $2
	  st	$2, 4($fp)
	  addiu	$sp, $sp, 40
	  ret	$lr
	  .set	macro
	  .set	reorder
	  .end	_Z9test_mathv
  $tmp2:
	  .size	_Z9test_mathv, ($tmp2)-_Z9test_mathv
	  .cfi_endproc



Display llvm IR nodes with Graphviz
------------------------------------

The previous section, display the DAG translation process in text on terminal 
by ``llc -debug`` option. 
The ``llc`` also support the graphic display. 
The `section Install other tools on iMac`_ mentioned the web for ``llc`` 
graphic display information. 
The ``llc`` graphic display with tool Graphviz is introduced in this section. 
The graphic display is more readable by eye than display text in terminal. 
It's not necessary, but helps a lot especially when you are tired in tracking 
the DAG translation process. 
List the ``llc`` graphic support options from the sub-section "SelectionDAG 
Instruction Selection Process" of web [#]_ as follows,

.. note:: The ``llc`` Graphviz DAG display options

  -view-dag-combine1-dags displays the DAG after being built, before the 
  first optimization pass. 
  
  -view-legalize-dags displays the DAG before Legalization. 
  
  -view-dag-combine2-dags displays the DAG before the second optimization 
  pass. 
  
  -view-isel-dags displays the DAG before the Select phase. 
  
  -view-sched-dags displays the DAG before Scheduling. 
  
By tracking ``llc -debug``, you can see the DAG translation steps as follows,

.. code-block:: bash

  Initial selection DAG
  Optimized lowered selection DAG
  Type-legalized selection DAG
  Optimized type-legalized selection DAG
  Legalized selection DAG
  Optimized legalized selection DAG
  Instruction selection
  Selected selection DAG
  Scheduling
  ...


Let's run ``llc`` with option -view-dag-combine1-dags, and open the output 
result with Graphviz as follows,

.. code-block:: bash

  118-165-12-177:InputFiles Jonathan$ /Users/Jonathan/llvm/test/
  cmake_debug_build/bin/Debug/llc -view-dag-combine1-dags -march=cpu0 
  -relocation-model=pic -filetype=asm ch4_2.bc -o ch4_2.cpu0.s
  Writing '/tmp/llvm_84ibpm/dag.main.dot'...  done. 
  118-165-12-177:InputFiles Jonathan$ Graphviz /tmp/llvm_84ibpm/dag.main.dot 

It will show the /tmp/llvm_84ibpm/dag.main.dot as :num:`Figure #otherinst-f1`.

.. _otherinst-f1:
.. figure:: ../Fig/otherinst/1.png
  :height: 851 px
  :width: 687 px
  :scale: 100 %
  :align: center

  llc option -view-dag-combine1-dags graphic view
  
From :num:`Figure #otherinst-f1`, we can see the -view-dag-combine1-dags option is for 
Initial selection DAG. 
We list the other view options and their corresponding DAG translation stage as 
follows,

.. note:: ``llc`` Graphviz options and corresponding DAG translation stage

  -view-dag-combine1-dags: Initial selection DAG
  
  -view-legalize-dags: Optimized type-legalized selection DAG
  
  -view-dag-combine2-dags: Legalized selection DAG
  
  -view-isel-dags: Optimized legalized selection DAG
  
  -view-sched-dags: Selected selection DAG

The -view-isel-dags is important and often used by an llvm backend writer 
because it is the DAG before instruction selection. 
The backend programmer need to know what is the DAG for writing the pattern 
match instruction in target description file .td.

Operator mod, %
-----------------

The DAG of %
~~~~~~~~~~~~~

Example input code ch4_5.cpp which contains the C operator **“%”** and it's 
corresponding llvm IR, as follows,

.. rubric:: LLVMBackendTutorialExampleCode/InputFiles/ch4_5.cpp
.. literalinclude:: ../LLVMBackendTutorialExampleCode/InputFiles/ch4_5.cpp
  :lines: 4-
  :linenos:

.. code-block:: bash

  ; ModuleID = 'ch4_5.bc'
   target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:32:64-
   f32:32:32-f64:32:64-v64:64:64-v128:128:128-a0:0:64-f80:128:128-n8:16:32-S128"
  target triple = "i386-apple-macosx10.8.0"
  
  define i32 @main() nounwind ssp {
  entry:
  %retval = alloca i32, align 4
  %b = alloca i32, align 4
  store i32 0, i32* %retval
  store i32 11, i32* %b, align 4
  %0 = load i32* %b, align 4
  %add = add nsw i32 %0, 1
  %rem = srem i32 %add, 12
  store i32 %rem, i32* %b, align 4
  %1 = load i32* %b, align 4
  ret i32 %1
  }


LLVM **srem** is the IR corresponding **“%”**, reference sub-section 
"srem instruction" of [3]_. 
Copy the reference as follows,

.. note:: **'srem'** Instruction 

  Syntax:
  **<result> = srem <ty> <op1>, <op2>   ; yields {ty}:result**
    
  Overview:
  The **'srem'** instruction returns the remainder from the signed division of its 
  two operands. This instruction can also take vector versions of the values in 
  which case the elements must be integers.
  
  Arguments:
  The two arguments to the **'srem'** instruction must be integer or vector of 
  integer values. Both arguments must have identical types.
  
  Semantics:
  This instruction returns the remainder of a division (where the result is 
  either zero or has the same sign as the dividend, op1), not the modulo operator 
  (where the result is either zero or has the same sign as the divisor, op2) of 
  a value. For more information about the difference, see The Math Forum. For a 
  table of how this is implemented in various languages, please see Wikipedia: 
  modulo operation.
  
  Note that signed integer remainder and unsigned integer remainder are distinct 
  operations; for unsigned integer remainder, use **'urem'**.
  
  Taking the remainder of a division by zero leads to undefined behavior. 
  Overflow also leads to undefined behavior; this is a rare case, but can occur, 
  for example, by taking the remainder of a 32-bit division of -2147483648 by -1. 
  (The remainder doesn't actually overflow, but this rule lets srem be 
  implemented using instructions that return both the result of the division and 
  the remainder.)
  
  Example:
  <result> = **srem i32 4, %var**      ; yields {i32}:result = 4 % %var


Run Chapter3_4/ with input file ch4_5.bc via ``llc`` option –view-isel-dags as 
below, will get the following error message and the llvm DAG of 
:num:`Figure #otherinst-f2` below.

.. code-block:: bash

  118-165-79-37:InputFiles Jonathan$ /Users/Jonathan/llvm/test/
  cmake_debug_build/bin/Debug/llc -march=cpu0 -view-isel-dags -relocation-model=
  pic -filetype=asm ch4_5.bc -o ch4_5.cpu0.s
  ...
  LLVM ERROR: Cannot select: 0x7fa73a02ea10: i32 = mulhs 0x7fa73a02c610, 
  0x7fa73a02e910 [ID=12]
  0x7fa73a02c610: i32 = Constant<12> [ORD=5] [ID=7]
  0x7fa73a02e910: i32 = Constant<715827883> [ID=9]


.. _otherinst-f2:
.. figure:: ../Fig/otherinst/2.png
  :height: 629 px
  :width: 580 px
  :scale: 100 %
  :align: center

  ch4_5.bc DAG

LLVM replace srem divide operation with multiply operation in DAG optimization 
because DIV operation cost more in time than MUL. 
For example code **“int b = 11; b=(b+1)%12;”**, it translate into 
:num:`Figure #otherinst-f2`. 
We verify the result and explain it by calculate the value in each node. 
The 0xC*0x2AAAAAAB=0x2,00000004, (mulhs 0xC, 0x2AAAAAAAB) meaning get the Signed 
mul high word (32bits). 
Multiply with 2 operands of 1 word size generate the 2 word size of result 
(0x2, 0xAAAAAAAB). 
The high word result, in this case is 0x2. 
The final result (sub 12, 12) is 0 which match the statement (11+1)%12.

 
Arm solution
~~~~~~~~~~~~~

To run with ARM solution, change Cpu0InstrInfo.td and Cpu0ISelDAGToDAG.cpp from 
Chapter4_1/ as follows,

.. rubric:: LLVMBackendTutorialExampleCode/Chapter4_1/Cpu0InstrInfo.td
.. code-block:: c++

  /// Multiply and Divide Instructions.
  def SMMUL   : ArithLogicR<0x41, "smmul", mulhs, IIImul, CPURegs, 1>;
  def UMMUL   : ArithLogicR<0x42, "ummul", mulhu, IIImul, CPURegs, 1>;
  //def MULT    : Mult32<0x41, "mult", IIImul>;
  //def MULTu   : Mult32<0x42, "multu", IIImul>;

.. rubric:: LLVMBackendTutorialExampleCode/Chapter4_1/Cpu0ISelDAGToDAG.cpp
.. code-block:: c++

  #if 0
  /// Select multiply instructions.
  std::pair<SDNode*, SDNode*>
  Cpu0DAGToDAGISel::SelectMULT(SDNode *N, unsigned Opc, DebugLoc dl, EVT Ty,
                               bool HasLo, bool HasHi) {
    SDNode *Lo = 0, *Hi = 0;
    SDNode *Mul = CurDAG->getMachineNode(Opc, dl, MVT::Glue, N->getOperand(0),
                                         N->getOperand(1));
    SDValue InFlag = SDValue(Mul, 0);

    if (HasLo) {
      Lo = CurDAG->getMachineNode(Cpu0::MFLO, dl,
                                  Ty, MVT::Glue, InFlag);
      InFlag = SDValue(Lo, 1);
    }
    if (HasHi)
      Hi = CurDAG->getMachineNode(Cpu0::MFHI, dl,
                                  Ty, InFlag);

    return std::make_pair(Lo, Hi);
  }
  #endif

  /// Select instructions not customized! Used for
  /// expanded, promoted and normal instructions
  SDNode* Cpu0DAGToDAGISel::Select(SDNode *Node) {
  ...
    switch(Opcode) {
    default: break;
  #if 0
    case ISD::MULHS:
    case ISD::MULHU: {
      MultOpc = (Opcode == ISD::MULHU ? Cpu0::MULTu : Cpu0::MULT);
      return SelectMULT(Node, MultOpc, dl, NodeTy, false, true).second;
    }
  #endif
   ...
  }


Let's run above changes with ch4_5.cpp as well as ``llc -view-sched-dags`` option 
to get :num:`Figure #otherinst-f3`. 
Similarly, SMMUL get the high word of multiply result.

.. _otherinst-f3:
.. figure:: ../Fig/otherinst/3.png
  :height: 702 px
  :width: 687 px
  :scale: 100 %
  :align: center

  Translate ch4_5.bc into cpu0 backend DAG

Follows is the result of run above changes with ch4_5.bc.

.. code-block:: bash

  118-165-66-82:InputFiles Jonathan$ /Users/Jonathan/llvm/test/cmake_
  debug_build/bin/Debug/llc -march=cpu0 -relocation-model=pic -filetype=asm 
  ch4_5.bc -o ch4_5.cpu0.s
  118-165-71-252:InputFiles Jonathan$ cat ch4_5.cpu0.s 
	  .section .mdebug.abi32
	  .previous
	  .file	"ch4_5.bc"
	  .text
	  .globl	main
	  .align	2
	  .type	main,@function
	  .ent	main                    # @main
  main:
	  .cfi_startproc
	  .frame	$fp,8,$lr
	  .mask 	0x00000000,0
	  .set	noreorder
	  .set	nomacro
  # BB#0:                                 # %entry
	  addiu	$sp, $sp, -8
  $tmp1:
	  .cfi_def_cfa_offset 8
	  addiu	$2, $zero, 0
	  st	$2, 4($fp)
	  addiu	$2, $zero, 11
	  st	$2, 0($fp)
	  lui	$2, 10922
	  ori	$3, $2, 43691
	  addiu	$2, $zero, 12
	  smmul	$3, $2, $3
	  shr	$4, $3, 31
	  sra	$3, $3, 1
	  addu	$3, $3, $4
	  mul	$3, $3, $2
	  subu	$2, $2, $3
	  st	$2, 0($fp)
	  addiu	$sp, $sp, 8
	  ret	$lr
	  .set	macro
	  .set	reorder
	  .end	main
  $tmp2:
	  .size	main, ($tmp2)-main
	  .cfi_endproc


The other instruction UMMUL and llvm IR mulhu are unsigned int type for 
operator %. 
You can check it by unmark the **“unsigned int b = 11;”** in ch4_5.cpp.

Use SMMUL instruction to get the high word of multiplication result is adopted 
in ARM. 


Mips solution
~~~~~~~~~~~~~~

Mips use MULT instruction and save the high & low part to register HI and LO. 
After that, use mfhi/mflo to move register HI/LO to your general purpose 
register. 
ARM SMMUL is fast if you only need the HI part of result (it ignore the LO part 
of operation). ARM also provide SMULL (signed multiply long) to get the whole 
64 bits result.
If you need the LO part of result, you can use Cpu0 MUL instruction which only 
get the LO part of result. 
Chapter4_1/ is implemented with Mips MULT style. 
We choose it as the implementation of this book to add instructions as less as 
possible. This approach is better for Cpu0 to keep it as a tutorial architecture 
for school teaching purpose material, and apply Cpu0 as an engineer learning 
materials in compiler, system program and verilog CPU hardware design.
For Mips style implementation, we add the following code in 
Cpu0RegisterInfo.td, Cpu0InstrInfo.td and Cpu0ISelDAGToDAG.cpp. 
And list the related DAG nodes mulhs and mulhu which are used in Chapter4_1/ 
from TargetSelectionDAG.td.

.. rubric:: LLVMBackendTutorialExampleCode/Chapter4_1/Cpu0RegisterInfo.td
.. code-block:: c++

  // Hi/Lo registers
  def HI  : Register<"HI">, DwarfRegNum<[18]>;
  def LO  : Register<"LO">, DwarfRegNum<[19]>;
  ...
  // Hi/Lo Registers
  def HILO : RegisterClass<"Cpu0", [i32], 32, (add HI, LO)>;

  // Cpu0Schedule.td
  ...
  def IIHiLo       : InstrItinClass;
  ...
  def Cpu0GenericItineraries : ProcessorItineraries<[ALU, IMULDIV], [], [
  ...
  InstrItinData<IIHiLo       , [InstrStage<1,  [IMULDIV]>]>,
  ...
  ]>;

.. rubric:: LLVMBackendTutorialExampleCode/Chapter4_1/Cpu0InstrInfo.td
.. code-block:: c++

  // Mul, Div
  class Mult<bits<8> op, string instr_asm, InstrItinClass itin,
       RegisterClass RC, list<Register> DefRegs>:
  FL<op, (outs), (ins RC:$ra, RC:$rb),
    !strconcat(instr_asm, "\t$ra, $rb"), [], itin> {
  let imm16 = 0;
  let isCommutable = 1;
  let Defs = DefRegs;
  let neverHasSideEffects = 1;
  }
  
  class Mult32<bits<8> op, string instr_asm, InstrItinClass itin>:
  Mult<op, instr_asm, itin, CPURegs, [HI, LO]>;
  
  // Move from Hi/Lo
  class MoveFromLOHI<bits<8> op, string instr_asm, RegisterClass RC,
           list<Register> UseRegs>:
  FL<op, (outs RC:$ra), (ins),
     !strconcat(instr_asm, "\t$ra"), [], IIHiLo> {
  let rb = 0;
  let imm16 = 0;
  let Uses = UseRegs;
  let neverHasSideEffects = 1;
  }
  ...
  /// Multiply and Divide Instructions.
  def MULT    : Mult32<0x41, "mult", IIImul>;
  def MULTu   : Mult32<0x42, "multu", IIImul>;
    
  def MFHI    : MoveFromLOHI<0x46, "mfhi", CPURegs, [HI]>;
  def MFLO    : MoveFromLOHI<0x47, "mflo", CPURegs, [LO]>;
  
.. rubric:: LLVMBackendTutorialExampleCode/Chapter4_5_2/Cpu0ISelDAGToDAG.cpp
.. code-block:: c++

  /// Select multiply instructions.
  std::pair<SDNode*, SDNode*>
  Cpu0DAGToDAGISel::SelectMULT(SDNode *N, unsigned Opc, DebugLoc dl, EVT Ty,
                bool HasLo, bool HasHi) {
  SDNode *Lo = 0, *Hi = 0;
  SDNode *Mul = CurDAG->getMachineNode(Opc, dl, MVT::Glue, N->getOperand(0),
                      N->getOperand(1));
  SDValue InFlag = SDValue(Mul, 0);
  
  if (HasLo) {
    Lo = CurDAG->getMachineNode(Cpu0::MFLO, dl,
                  Ty, MVT::Glue, InFlag);
    InFlag = SDValue(Lo, 1);
  }
  if (HasHi)
    Hi = CurDAG->getMachineNode(Cpu0::MFHI, dl,
                  Ty, InFlag);
  
  return std::make_pair(Lo, Hi);
  }
  
  /// Select instructions not customized! Used for
  /// expanded, promoted and normal instructions
  SDNode* Cpu0DAGToDAGISel::Select(SDNode *Node) {
  unsigned Opcode = Node->getOpcode();
  DebugLoc dl = Node->getDebugLoc();
  ...
  EVT NodeTy = Node->getValueType(0);
  unsigned MultOpc;
  switch(Opcode) {
  default: break;
  
  case ISD::MULHS:
  case ISD::MULHU: {
    MultOpc = (Opcode == ISD::MULHU ? Cpu0::MULTu : Cpu0::MULT);
    return SelectMULT(Node, MultOpc, dl, NodeTy, false, true).second;
  }
  ...
  }
  
.. rubric:: include/llvm/Target/TargetSelectionDAG.td
.. code-block:: c++

  def mulhs    : SDNode<"ISD::MULHS"     , SDTIntBinOp, [SDNPCommutative]>;
  def mulhu    : SDNode<"ISD::MULHU"     , SDTIntBinOp, [SDNPCommutative]>;


  
Except the custom type, llvm IR operations of expand and promote type will call 
Cpu0DAGToDAGISel::Select() during instruction selection of DAG translation. 
In Select(), it return the HI part of multiplication result to HI register, 
for IR operations of mulhs or mulhu. 
After that, MFHI instruction move the HI register to cpu0 field "a" register, 
$ra. 
MFHI instruction is FL format and only use cpu0 field "a" register, we set 
the $rb and imm16 to 0. 
:num:`Figure #otherinst-f4` and ch4_5.cpu0.s are the result of compile ch4_5.bc.

.. _otherinst-f4:
.. figure:: ../Fig/otherinst/4.png
  :height: 807 px
  :width: 309 px
  :scale: 75 %
  :align: center

  DAG for ch4_5.bc with Mips style MULT

.. code-block:: bash

  118-165-66-82:InputFiles Jonathan$ cat ch4_5.cpu0.s 
    .section .mdebug.abi32
    .previous
    .file "ch4_5.bc"
    .text
    .globl  main
    .align  2
    .type main,@function
    .ent  main                    # @main
  main:
    .cfi_startproc
    .frame  $sp,8,$lr
    .mask   0x00000000,0
    .set  noreorder
    .set  nomacro
  # BB#0:
    addiu $sp, $sp, -8
  $tmp1:
    .cfi_def_cfa_offset 8
    addiu $2, $zero, 0
    st  $2, 4($sp)
    addiu $2, $zero, 11
    st  $2, 0($sp)
    addiu $2, $zero, 10922
    shl $2, $2, 16
    ori $3, $2, 43691
    addiu $2, $zero, 12
    mult  $2, $3
    mfhi  $3
    shr $4, $3, 31
    sra $3, $3, 1
    addu  $3, $3, $4
    mul $3, $3, $2
    sub $2, $2, $3
    st  $2, 0($sp)
    addiu $sp, $sp, 8
    ret $lr
    .set  macro
    .set  reorder
    .end  main
  $tmp2:
    .size main, ($tmp2)-main
    .cfi_endproc



Full support %
---------------

The sensitive readers may find the llvm using **“multiplication”** instead 
of **“div”** to get the **“%”** result just because our example use constant as 
divider, **“(b+1)%12”** in our example. 
If programmer use variable as the divider like **“(b+1)%a”**, then what will 
happen in our code. 
The answer is our code will has error to take care this. 
In `section Support arithmetic instructions`_, we use **“div a, b”** 
to hold the quotient part in register. 
The multiplication operator **“*”** need 64 bits of register to hold the result 
for two 32 bits of operands multiplication. 
We modify cpu0 to use the pair of registers LO and HI which just like Mips to 
solve this issue in last section. 
Now, it's time to modify cpu0 for integer **“divide”** operator again. 
We use LO and HI registers to hold the **"quotient"** and **"remainder"** and 
use instructions **“mflo”** and **“mfhi”** to get the result from LO or HI 
registers. 
With this solution, the **“c = a / b”** can be got by **“div a, b”** and 
**“mflo c”**; the **“c = a % b”** can be got by **“div a, b”** and **“mfhi c”**.
 
Chapter4_1/ support operator **“%”** and **“/”**. 
The code added in Chapter4_1/ as follows,

.. rubric:: LLVMBackendTutorialExampleCode/Chapter4_1/Cpu0InstrInfo.cpp
.. literalinclude:: ../LLVMBackendTutorialExampleCode/Chapter4_6/Cpu0InstrInfo.cpp
  :start-after: // Cpu0InstrInfo::copyPhysReg()
  :end-before: MachineInstr*
  :linenos:

.. rubric:: LLVMBackendTutorialExampleCode/Chapter4_1/Cpu0InstrInfo.h
.. literalinclude:: ../LLVMBackendTutorialExampleCode/Chapter4_6/Cpu0InstrInfo.h
  :start-after: virtual const Cpu0RegisterInfo &getRegisterInfo() const;
  :end-before: public:
  :linenos:

.. rubric:: LLVMBackendTutorialExampleCode/Chapter4_1/Cpu0InstrInfo.td
.. code-block:: c++

  def SDT_Cpu0DivRem     : SDTypeProfile<0, 2,
             [SDTCisInt<0>,
            SDTCisSameAs<0, 1>]>;
  ...
  // DivRem(u) nodes
  def Cpu0DivRem  : SDNode<"Cpu0ISD::DivRem", SDT_Cpu0DivRem,
         [SDNPOutGlue]>;
  def Cpu0DivRemU : SDNode<"Cpu0ISD::DivRemU", SDT_Cpu0DivRem,
         [SDNPOutGlue]>;
  ...
  class Div<SDNode opNode, bits<8> op, string instr_asm, InstrItinClass itin,
    RegisterClass RC, list<Register> DefRegs>:
  FL<op, (outs), (ins RC:$rb, RC:$rc),
   !strconcat(instr_asm, "\t$$zero, $rb, $rc"),
   [(opNode RC:$rb, RC:$rc)], itin> {
  let imm16 = 0;
  let Defs = DefRegs;
  }
  
  class Div32<SDNode opNode, bits<8> op, string instr_asm, InstrItinClass itin>:
  Div<opNode, op, instr_asm, itin, CPURegs, [HI, LO]>;
  ...
  class MoveToLOHI<bits<8> op, string instr_asm, RegisterClass RC,
       list<Register> DefRegs>:
  FL<op, (outs), (ins RC:$ra),
   !strconcat(instr_asm, "\t$ra"), [], IIHiLo> {
  let rb = 0;
  let imm16 = 0;
  let Defs = DefRegs;
  let neverHasSideEffects = 1;
  }
  ...
  def SDIV    : Div32<Cpu0DivRem, 0x43, "div", IIIdiv>;
  def UDIV    : Div32<Cpu0DivRemU, 0x44, "divu", IIIdiv>;
  ...
  def MTHI    : MoveToLOHI<0x48, "mthi", CPURegs, [HI]>;
  def MTLO    : MoveToLOHI<0x49, "mtlo", CPURegs, [LO]>;
  
.. rubric:: LLVMBackendTutorialExampleCode/Chapter4_1/Cpu0ISelLowering.cpp
.. code-block:: c++

  Cpu0TargetLowering::
  Cpu0TargetLowering(Cpu0TargetMachine &TM)
  : TargetLowering(TM, new TargetLoweringObjectFileELF()),
  Subtarget(&TM.getSubtarget<Cpu0Subtarget>()) {
  ...
  setOperationAction(ISD::SDIV, MVT::i32, Expand);
  setOperationAction(ISD::SREM, MVT::i32, Expand);
  setOperationAction(ISD::UDIV, MVT::i32, Expand);
  setOperationAction(ISD::UREM, MVT::i32, Expand);
  
  setTargetDAGCombine(ISD::SDIVREM);
  setTargetDAGCombine(ISD::UDIVREM);
  ...
  }
  ...
  static SDValue PerformDivRemCombine(SDNode *N, SelectionDAG& DAG,
          TargetLowering::DAGCombinerInfo &DCI,
          const Cpu0Subtarget* Subtarget) {
  if (DCI.isBeforeLegalizeOps())
  return SDValue();
  
  EVT Ty = N->getValueType(0);
  unsigned LO = Cpu0::LO;
  unsigned HI = Cpu0::HI;
  unsigned opc = N->getOpcode() == ISD::SDIVREM ? Cpu0ISD::DivRem :
              Cpu0ISD::DivRemU;
  DebugLoc dl = N->getDebugLoc();
  
  SDValue DivRem = DAG.getNode(opc, dl, MVT::Glue,
           N->getOperand(0), N->getOperand(1));
  SDValue InChain = DAG.getEntryNode();
  SDValue InGlue = DivRem;
  
  // insert MFLO
  if (N->hasAnyUseOfValue(0)) {
  SDValue CopyFromLo = DAG.getCopyFromReg(InChain, dl, LO, Ty,
            InGlue);
  DAG.ReplaceAllUsesOfValueWith(SDValue(N, 0), CopyFromLo);
  InChain = CopyFromLo.getValue(1);
  InGlue = CopyFromLo.getValue(2);
  }
  
  // insert MFHI
  if (N->hasAnyUseOfValue(1)) {
  SDValue CopyFromHi = DAG.getCopyFromReg(InChain, dl,
            HI, Ty, InGlue);
  DAG.ReplaceAllUsesOfValueWith(SDValue(N, 1), CopyFromHi);
  }
  
  return SDValue();
  }
  
  SDValue Cpu0TargetLowering::PerformDAGCombine(SDNode *N, DAGCombinerInfo &DCI)
  const {
  SelectionDAG &DAG = DCI.DAG;
  unsigned opc = N->getOpcode();
  
  switch (opc) {
  default: break;
  case ISD::SDIVREM:
  case ISD::UDIVREM:
  return PerformDivRemCombine(N, DAG, DCI, Subtarget);
  }
  
  return SDValue();
  }
  
.. rubric:: LLVMBackendTutorialExampleCode/Chapter4_1/Cpu0ISelLowering.h
.. code-block:: c++

  namespace llvm {
  namespace Cpu0ISD {
  enum NodeType {
    // Start the numbering from where ISD NodeType finishes.
    FIRST_NUMBER = ISD::BUILTIN_OP_END,
    Ret,
    // DivRem(u)
    DivRem,
    DivRemU
  };
  }
  ...

IR instruction **sdiv** stand for signed div while **udiv** is for unsigned div.

Run with ch4_6_2.cpp can get the "div" result for operator **“/”** but it cannot
be compiled at this point. It need the function call argument support in Chapter
8 of Function call. 
If run with ch4_6_1.cpp as below, cannot get the **“div”** for operator 
**“%”**. 
It still use **"multiplication"** instead of **"div"** in ch4_6_1.cpp because 
llvm do **“Constant Propagation Optimization”** on this. 
The ch4_6_2.cpp can get the **“div”** for **“%”** result since it make the 
llvm **“Constant Propagation Optimization”** useless in this. 

.. rubric:: LLVMBackendTutorialExampleCode/InputFiles/ch4_6_1.cpp
.. literalinclude:: ../LLVMBackendTutorialExampleCode/InputFiles/ch4_6_1.cpp
  :lines: 4-
  :linenos:
  
.. rubric:: LLVMBackendTutorialExampleCode/InputFiles/ch4_6_2.cpp
.. literalinclude:: ../LLVMBackendTutorialExampleCode/InputFiles/ch4_6_2.cpp
  :lines: 4-
  :linenos:

.. code-block:: bash

  118-165-77-79:InputFiles Jonathan$ clang -c ch4_6_2.cpp -emit-llvm -o ch4_6_2.bc
  118-165-77-79:InputFiles Jonathan$ /Users/Jonathan/llvm/test/cmake_
  debug_build/bin/Debug/llc -march=cpu0 -relocation-model=pic -filetype=asm 
  ch4_6_2.bc -o -
  ...
  div $zero, $3, $2
  mflo  $2
  ...

To explain how work with **“div”**, let's run Chapter8_4 with 
ch4_6_2.cpp as follows,

.. code-block:: bash

  118-165-83-58:InputFiles Jonathan$ clang -c ch4_6_2.cpp -I/Applications/Xcode.app/
  Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.8.sdk/usr/
  include/ -emit-llvm -o ch4_6_2.bc
  118-165-83-58:InputFiles Jonathan$ /Users/Jonathan/llvm/test/cmake_debug_build/bin/
  Debug/llc -march=cpu0 -relocation-model=pic -filetype=asm -debug ch4_6_2.bc -o -
  Args: /Users/Jonathan/llvm/test/cmake_debug_build/bin/Debug/llc -march=cpu0 
  -relocation-model=pic -filetype=asm -debug ch4_6_2.bc -o - 
  
  === _Z8test_modi
  Initial selection DAG: BB#0 '_Z8test_modi:'
  SelectionDAG has 21 nodes:
    0x7fed68410bc8: ch = EntryToken [ORD=1]
  
    0x7fed6882cb10: i32 = undef [ORD=1]
  
    0x7fed6882cd10: i32 = FrameIndex<0> [ORD=1]
  
    0x7fed6882ce10: i32 = Constant<0>
  
    0x7fed6882d110: i32 = FrameIndex<1> [ORD=2]
  
        0x7fed68410bc8: <multiple use>
          0x7fed68410bc8: <multiple use>
          0x7fed6882ca10: i32 = FrameIndex<-1> [ORD=1]
  
          0x7fed6882cb10: <multiple use>
        0x7fed6882cc10: i32,ch = load 0x7fed68410bc8, 0x7fed6882ca10, 
        0x7fed6882cb10<LD4[FixedStack-1]> [ORD=1]
  
        0x7fed6882cd10: <multiple use>
        0x7fed6882cb10: <multiple use>
      0x7fed6882cf10: ch = store 0x7fed68410bc8, 0x7fed6882cc10, 0x7fed6882cd10, 
      0x7fed6882cb10<ST4[%1]> [ORD=1]
  
      0x7fed6882d010: i32 = Constant<11> [ORD=2]
  
      0x7fed6882d110: <multiple use>
      0x7fed6882cb10: <multiple use>
    0x7fed6882d210: ch = store 0x7fed6882cf10, 0x7fed6882d010, 0x7fed6882d110, 
    0x7fed6882cb10<ST4[%b]> [ORD=2]
  
      0x7fed6882d210: <multiple use>
      0x7fed6882d110: <multiple use>
      0x7fed6882cb10: <multiple use>
    0x7fed6882d310: i32,ch = load 0x7fed6882d210, 0x7fed6882d110, 
    0x7fed6882cb10<LD4[%b]> [ORD=3]
  
      0x7fed6882d210: <multiple use>
      0x7fed6882cd10: <multiple use>
      0x7fed6882cb10: <multiple use>
    0x7fed6882d610: i32,ch = load 0x7fed6882d210, 0x7fed6882cd10, 
    0x7fed6882cb10<LD4[%1]> [ORD=5]
  
        0x7fed6882d310: <multiple use>
        0x7fed6882d610: <multiple use>
      0x7fed6882d810: ch = TokenFactor 0x7fed6882d310:1, 0x7fed6882d610:1 [ORD=7]
  
          0x7fed6882d310: <multiple use>
          0x7fed6882d410: i32 = Constant<1> [ORD=4]
  
        0x7fed6882d510: i32 = add 0x7fed6882d310, 0x7fed6882d410 [ORD=4]
  
        0x7fed6882d610: <multiple use>
      0x7fed6882d710: i32 = srem 0x7fed6882d510, 0x7fed6882d610 [ORD=6]
  
      0x7fed6882d110: <multiple use>
      0x7fed6882cb10: <multiple use>
    0x7fed6882fc10: ch = store 0x7fed6882d810, 0x7fed6882d710, 0x7fed6882d110, 
    0x7fed6882cb10<ST4[%b]> [ORD=7]
  
    0x7fed6882fe10: i32 = Register %V0
  
      0x7fed6882fc10: <multiple use>
      0x7fed6882fe10: <multiple use>
        0x7fed6882fc10: <multiple use>
        0x7fed6882d110: <multiple use>
        0x7fed6882cb10: <multiple use>
      0x7fed6882fd10: i32,ch = load 0x7fed6882fc10, 0x7fed6882d110, 
      0x7fed6882cb10<LD4[%b]> [ORD=8]
  
    0x7fed6882ff10: ch,glue = CopyToReg 0x7fed6882fc10, 0x7fed6882fe10, 
    0x7fed6882fd10
  
      0x7fed6882ff10: <multiple use>
      0x7fed6882fe10: <multiple use>
      0x7fed6882ff10: <multiple use>
    0x7fed68830010: ch = Cpu0ISD::Ret 0x7fed6882ff10, 0x7fed6882fe10, 
    0x7fed6882ff10:1
    
  Replacing.1 0x7fed6882fd10: i32,ch = load 0x7fed6882fc10, 0x7fed6882d110, 
  0x7fed6882cb10<LD4[%b]> [ORD=8]
  
  With: 0x7fed6882d710: i32 = srem 0x7fed6882d510, 0x7fed6882d610 [ORD=6]
   and 1 other values
  
  Replacing.1 0x7fed6882d310: i32,ch = load 0x7fed6882d210, 0x7fed6882d110, 
  0x7fed6882cb10<LD4[%b]> [ORD=3]
  
  With: 0x7fed6882d010: i32 = Constant<11> [ORD=2]
   and 1 other values
  
  Replacing.3 0x7fed6882d810: ch = TokenFactor 0x7fed6882d210, 
  0x7fed6882d610:1 [ORD=7]
  
  With: 0x7fed6882d610: i32,ch = load 0x7fed6882d210, 0x7fed6882cd10, 
  0x7fed6882cb10<LD4[%1]> [ORD=5]
  
  
  Replacing.3 0x7fed6882d510: i32 = add 0x7fed6882d010, 0x7fed6882d410 [ORD=4]
  
  With: 0x7fed6882d810: i32 = Constant<12>
  
  
  Replacing.1 0x7fed6882cc10: i32,ch = load 0x7fed68410bc8, 0x7fed6882ca10, 
  0x7fed6882cb10<LD4[FixedStack-1](align=8)> [ORD=1]
  
  With: 0x7fed6882cc10: i32,ch = load 0x7fed68410bc8, 0x7fed6882ca10, 
  0x7fed6882cb10<LD4[FixedStack-1](align=8)> [ORD=1]
   and 1 other values
  Optimized lowered selection DAG: BB#0 '_Z8test_modi:'
  SelectionDAG has 16 nodes:
    0x7fed68410bc8: ch = EntryToken [ORD=1]
  
    0x7fed6882cb10: i32 = undef [ORD=1]
  
    0x7fed6882cd10: i32 = FrameIndex<0> [ORD=1]
  
    0x7fed6882d110: i32 = FrameIndex<1> [ORD=2]
  
          0x7fed68410bc8: <multiple use>
            0x7fed68410bc8: <multiple use>
            0x7fed6882ca10: i32 = FrameIndex<-1> [ORD=1]
  
            0x7fed6882cb10: <multiple use>
          0x7fed6882cc10: i32,ch = load 0x7fed68410bc8, 0x7fed6882ca10, 
          0x7fed6882cb10<LD4[FixedStack-1](align=8)> [ORD=1]
  
          0x7fed6882cd10: <multiple use>
          0x7fed6882cb10: <multiple use>
        0x7fed6882cf10: ch = store 0x7fed68410bc8, 0x7fed6882cc10, 0x7fed6882cd10, 
        0x7fed6882cb10<ST4[%1]> [ORD=1]
  
        0x7fed6882d010: i32 = Constant<11> [ORD=2]
  
        0x7fed6882d110: <multiple use>
        0x7fed6882cb10: <multiple use>
      0x7fed6882d210: ch = store 0x7fed6882cf10, 0x7fed6882d010, 0x7fed6882d110, 
      0x7fed6882cb10<ST4[%b]> [ORD=2]
  
      0x7fed6882cd10: <multiple use>
      0x7fed6882cb10: <multiple use>
    0x7fed6882d610: i32,ch = load 0x7fed6882d210, 0x7fed6882cd10, 
    0x7fed6882cb10<LD4[%1]> [ORD=5]
  
      0x7fed6882d810: i32 = Constant<12>
  
      0x7fed6882d610: <multiple use>
    0x7fed6882d710: i32 = srem 0x7fed6882d810, 0x7fed6882d610 [ORD=6]
  
    0x7fed6882fe10: i32 = Register %V0
  
        0x7fed6882d610: <multiple use>
        0x7fed6882d710: <multiple use>
        0x7fed6882d110: <multiple use>
        0x7fed6882cb10: <multiple use>
      0x7fed6882fc10: ch = store 0x7fed6882d610:1, 0x7fed6882d710, 0x7fed6882d110, 
      0x7fed6882cb10<ST4[%b]> [ORD=7]
  
      0x7fed6882fe10: <multiple use>
      0x7fed6882d710: <multiple use>
    0x7fed6882ff10: ch,glue = CopyToReg 0x7fed6882fc10, 0x7fed6882fe10, 
    0x7fed6882d710
  
      0x7fed6882ff10: <multiple use>
      0x7fed6882fe10: <multiple use>
      0x7fed6882ff10: <multiple use>
    0x7fed68830010: ch = Cpu0ISD::Ret 0x7fed6882ff10, 0x7fed6882fe10, 
    0x7fed6882ff10:1
  
  Type-legalized selection DAG: BB#0 '_Z8test_modi:'
  SelectionDAG has 16 nodes:
    ...
    0x7fed6882d610: i32,ch = load 0x7fed6882d210, 0x7fed6882cd10, 
    0x7fed6882cb10<LD4[%1]> [ORD=5] [ID=-3]
  
      0x7fed6882d810: i32 = Constant<12> [ID=-3]
  
      0x7fed6882d610: <multiple use>
    0x7fed6882d710: i32 = srem 0x7fed6882d810, 0x7fed6882d610 [ORD=6] [ID=-3]
    ...
    
  Legalized selection DAG: BB#0 '_Z8test_modi:'
  SelectionDAG has 16 nodes:
    0x7fed68410bc8: ch = EntryToken [ORD=1] [ID=0]
  
    0x7fed6882cb10: i32 = undef [ORD=1] [ID=2]
  
    0x7fed6882cd10: i32 = FrameIndex<0> [ORD=1] [ID=3]
  
    0x7fed6882d110: i32 = FrameIndex<1> [ORD=2] [ID=5]
  
    0x7fed6882fe10: i32 = Register %V0 [ID=6]
    ...
      0x7fed6882d810: i32 = Constant<12> [ID=7]
  
      0x7fed6882d610: <multiple use>
    0x7fed6882ce10: i32,i32 = sdivrem 0x7fed6882d810, 0x7fed6882d610
  
  
  Optimized legalized selection DAG: BB#0 '_Z8test_modi:'
  SelectionDAG has 18 nodes:
    ...
      0x7fed6882d510: i32 = Register %HI
  
        0x7fed6882d810: i32 = Constant<12> [ID=7]
  
        0x7fed6882d610: <multiple use>
      0x7fed6882d410: glue = Cpu0ISD::DivRem 0x7fed6882d810, 0x7fed6882d610
  
    0x7fed6882d310: i32,ch,glue = CopyFromReg 0x7fed68410bc8, 0x7fed6882d510, 
    0x7fed6882d410
    ...
  
  ===== Instruction selection begins: BB#0 ''
  ...
  Selecting: 0x7fed6882d410: glue = Cpu0ISD::DivRem 0x7fed6882d810, 
  0x7fed6882d610 [ID=13]
  
  ISEL: Starting pattern match on root node: 0x7fed6882d410: glue = 
  Cpu0ISD::DivRem 0x7fed6882d810, 0x7fed6882d610 [ID=13]
  
    Initial Opcode index to 1355
    Morphed node: 0x7fed6882d410: i32,glue = SDIV 0x7fed6882d810, 0x7fed6882d610
  
  ISEL: Match complete!
  => 0x7fed6882d410: i32,glue = SDIV 0x7fed6882d810, 0x7fed6882d610
  ...


According above DAG translation message from ``llc -debug``, it do the 
following things:

1. Reduce DAG nodes in stage "Optimized lowered selection DAG" (Replacing ... 
   displayed before "Optimized lowered selection DAG: BB#0 '_Z8test_modi:entry'
   "). 
   Since SSA form has some redundant nodes for store and load, them can be 
   removed.

2. Change DAG srem to sdivrem in stage "Legalized selection DAG".

3. Change DAG sdivrem to Cpu0ISD::DivRem and in stage "Optimized legalized 
   selection DAG".

4. Add DAG "0x7fd25b830710: i32 = Register %HI" and "CopyFromReg 0x7fd25b410e18, 
   0x7fd25b830710, 0x7fd25b830910" in stage "Optimized legalized selection DAG".

Summary as Table: Stages for C operator % and Table: Functions handle the DAG 
translation and pattern match for C operator %.

.. table:: Stages for C operator %

  ==================================  ===================== =========================
  Stage                               IR/DAG/instruction    IR/DAG/instruction
  ==================================  ===================== =========================
  .bc                                 srem        
  Legalized selection DAG             sdivrem       
  Optimized legalized selection DAG   Cpu0ISD::DivRem       CopyFromReg xx, Hi, xx
  pattern match                       div                   mfhi
  ==================================  ===================== =========================


.. table:: Functions handle the DAG translation and pattern match for C operator %

  ====================================  ============================
  Translation                           Do by
  ====================================  ============================
  srem => sdivrem                       setOperationAction(ISD::SREM, MVT::i32, Expand);
  sdivrem => Cpu0ISD::DivRem            setTargetDAGCombine(ISD::SDIVREM);
  sdivrem => CopyFromReg xx, Hi, xx     PerformDivRemCombine();
  Cpu0ISD::DivRem => div                SDIV (Cpu0InstrInfo.td)
  CopyFromReg xx, Hi, xx => mfhi        MFLO (Cpu0InstrInfo.td)
  ====================================  ============================


Item 2 as above, is triggered by code 
"setOperationAction(ISD::SREM, MVT::i32, Expand);" in Cpu0ISelLowering.cpp. 
About **Expand** please ref. [#]_ and [#]_. Item 3 is triggered by code 
"setTargetDAGCombine(ISD::SDIVREM);" in Cpu0ISelLowering.cpp.
Item 4 is did by PerformDivRemCombine() which called by PerformDAGCombine() 
since the **%** corresponding **srem** 
make the "N->hasAnyUseOfValue(1)" to true in PerformDivRemCombine(). 
Then, it create "CopyFromReg 0x7fd25b410e18, 0x7fd25b830710, 0x7fd25b830910". 
When use **"/"** in C, it will make "N->hasAnyUseOfValue(0)" to ture.
For sdivrem, **sdiv** make "N->hasAnyUseOfValue(0)" true while **srem** make 
"N->hasAnyUseOfValue(1)" ture.

Above items will change the DAG when ``llc`` running. After that, the pattern 
match defined in Chapter4_6/Cpu0InstrInfo.td will translate **Cpu0ISD::DivRem** 
to **div**; and **"CopyFromReg 0x7fd25b410e18, Register %H, 0x7fd25b830910"** 
to **mfhi**.


Summary
--------

We support most of C operators in this chapter. 
Until now, we have around 3400 lines of source code with comments. 
With these 345 lines of source code added, it support the number of operators 
from three to over ten.

List C operators, IR of .bc, Optimized legalized selection DAG and Cpu0 
instructions implemented in this chapter in Table: Chapter 4 operators.

.. table:: Chapter 4 operators

  ==========  =================================  ====================================  ==========
  C           .bc                                Optimized legalized selection DAG     Cpu0
  ==========  =================================  ====================================  ==========
  \+          add                                add                                   addu
  \-          sub                                sub                                   subu
  \*          mul                                mul                                   mul
  /           sdiv                               Cpu0ISD::DivRem                       div
  -           udiv                               Cpu0ISD::DivRemU                      divu
  &, &&       and                                and                                   and
  \|, \|\|    or                                 or                                    or
  ^           xor                                xor                                   xor
  <<          shl                                shl                                   shl
  >>          - ashr                             - sra                                 - sra
              - lshr                             - srl                                 - shr
  !           - %tobool = icmp ne i32 %0, 0      - %lnot = (setcc %tobool, 0, seteq)   - %1 = (xor %tobool, 0)
              - %lnot = xor i1 %tobool, true     - %conv = (and %lnot, 1)              - %true = (addiu $r0, 1)
                                                                                       - %lnot = (xor %1, %true)
  -           - %conv = zext i1 %lnot to i32     - %conv = (and %lnot, 1)              - %conv = (and %lnot, 1)
  %           - srem                             - Cpu0ISD::DivRem                     - div
              - sremu                            - Cpu0ISD::DivRemU                    - divu
  ==========  =================================  ====================================  ==========


.. _section Operator “not” !:
  http://jonathan2251.github.com/lbd/otherinst.html#operator-not

.. _section Display llvm IR nodes with Graphviz:
  http://jonathan2251.github.com/lbd/otherinst.html#display-llvm-ir-nodes-
  with-graphviz

.. _section Adjust cpu0 instructions:
  http://jonathan2251.github.com/lbd/otherinst.html#adjust-cpu0-instructions

.. _section Local variable pointer:
  http://jonathan2251.github.com/lbd/otherinst.html#local-variable-pointer

.. _section Operator mod, %:
  http://jonathan2251.github.com/lbd/otherinst.html#operator-mod

.. _section Install other tools on iMac:
  http://jonathan2251.github.com/lbd/install.html#install-other-tools-on-imac

.. _section Support arithmetic instructions:
  http://jonathan2251.github.com/lbd/otherinst.html#support-arithmetic-
  instructions

.. [#] http://llvm.org/docs/doxygen/html/structllvm_1_1InstrStage.html

.. [#] http://msdn.microsoft.com/en-us/library/336xbhcz%28v=vs.80%29.aspx

.. [#] http://llvm.org/docs/LangRef.html.

.. [#] http://llvm.org/docs/CodeGenerator.html

.. [#] http://llvm.org/docs/WritingAnLLVMBackend.html#expand

.. [#] http://llvm.org/docs/CodeGenerator.html#selectiondag-legalizetypes-phase











Local variable pointer
-----------------------

To support pointer to local variable, add this code fragment in 
Cpu0InstrInfo.td and Cpu0InstPrinter.cpp as follows,

.. rubric:: LLVMBackendTutorialExampleCode/Chapter4_4/Cpu0InstrInfo.td
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
  
.. rubric:: LLVMBackendTutorialExampleCode/Chapter4_4/Cpu0InstPrinter.td
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

Run ch4_4.cpp with code Chapter4_4/ which support pointer to local variable, 
will get result as follows,

.. rubric:: LLVMBackendTutorialExampleCode/InputFiles/ch4_4.cpp
.. literalinclude:: ../LLVMBackendTutorialExampleCode/InputFiles/ch4_4.cpp
  :lines: 4-
  :linenos:

.. code-block:: bash

  118-165-66-82:InputFiles Jonathan$ clang -c ch4_4.cpp -emit-llvm -o ch4_4.bc
  118-165-66-82:InputFiles Jonathan$ /Users/Jonathan/llvm/test/cmake_
  debug_build/bin/Debug/llc -march=cpu0 -relocation-model=pic -filetype=asm 
  ch4_4.bc -o ch4_4.cpu0.s
  118-165-66-82:InputFiles Jonathan$ cat ch4_4.cpu0.s 
    .section .mdebug.abi32
    .previous
    .file "ch4_5.bc"
    .text
    .globl  main
    .align  2
    .type main,@function
    .ent  main            # @main
  main:
    .cfi_startproc
    .frame  $sp,16,$lr
    .mask 0x00000000,0
    .set  noreorder
    .set  nomacro
  # BB#0:
    addiu $sp, $sp, -16
  $tmp1:
    .cfi_def_cfa_offset 16
    addiu $2, $zero, 0
    st  $2, 12($sp)
    addiu $2, $zero, 3
    st  $2, 8($sp)
    addiu $2, $sp, 8
    st  $2, 0($sp)
    addiu $sp, $sp, 16
    ret $lr
    .set  macro
    .set  reorder
    .end  main
  $tmp2:
    .size main, ($tmp2)-main
    .cfi_endproc










Operator “not” !
-----------------

Files ch4_2.cpp and ch4_2.bc are the C source code for **“not”** boolean operator 
and it's corresponding llvm IR. List them as follows,

.. rubric:: LLVMBackendTutorialExampleCode/InputFiles/ch4_2.cpp
.. literalinclude:: ../LLVMBackendTutorialExampleCode/InputFiles/ch4_2.cpp
  :lines: 4-
  :linenos:

.. code-block:: bash

  ; ModuleID = 'ch4_2.bc'
  target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:32:64-
  f32:32:32-f64:32:64-v64:64:64-v128:128:128-a0:0:64-f80:128:128-n8:16:32-S128"
  target triple = "i386-apple-macosx10.8.0"
  
  define i32 @main() nounwind ssp {
  entry:
    %retval = alloca i32, align 4
    %a = alloca i32, align 4
    %b = alloca i32, align 4
    store i32 0, i32* %retval
    store i32 5, i32* %a, align 4
    store i32 0, i32* %b, align 4
    %0 = load i32* %a, align 4      // a = %0
    %tobool = icmp ne i32 %0, 0   // ne: stand for not egual
    %lnot = xor i1 %tobool, true
    %conv = zext i1 %lnot to i32  
    store i32 %conv, i32* %b, align 4
    %1 = load i32* %b, align 4
    ret i32 %1
  }

As above, b = !a, is translated into (xor (icmp ne i32 %0, 0), true). 
The %0 is the virtual register of variable **a** and the result of 
(icmp ne i32 %0, 0) is 1 bit size. 
To prove the translation is correct. 
Let's assume %0 != 0 first, then the (icmp ne i32 %0, 0) = 1 (or true), and 
(xor 1, 1) = 0. 
When %0 = 0, (icmp ne i32 %0, 0) = 0 (or false), and (xor 0, 1) = 1. 
So, the translation is correct. 
  
Now, let's run ch4_2.bc with Chapter4_1/ with ``llc -debug`` option to get result 
as follows,

.. code-block:: bash

  118-165-16-22:InputFiles Jonathan$ /Users/Jonathan/llvm/test/
  cmake_debug_build/bin/Debug/llc -march=cpu0 -debug -relocation-model=pic 
  -filetype=asm ch4_3.bc -o ch4_3.cpu0.s
  ...
  
  === main
  Initial selection DAG: BB#0 'main:entry'
  SelectionDAG has 20 nodes:
  ...
  0x7ffb7982ab10: <multiple use>
      0x7ffb7982ab10: <multiple use>
      0x7ffb7982a210: <multiple use>
      0x7ffb7982ac10: ch = setne [ORD=5]

    0x7ffb7982ad10: i1 = setcc 0x7ffb7982ab10, 0x7ffb7982a210, 0x7ffb7982ac10 
    [ORD=5]

    0x7ffb7982ae10: i1 = Constant<-1> [ORD=6]

    0x7ffb7982af10: i1 = xor 0x7ffb7982ad10, 0x7ffb7982ae10 [ORD=6]

  0x7ffb7982b010: i32 = zero_extend 0x7ffb7982af10 [ORD=7]
  ... 
  Replacing.3 0x7ffb7982af10: i1 = xor 0x7ffb7982ad10, 0x7ffb7982ae10 [ORD=6]

  With: 0x7ffb7982d210: i1 = setcc 0x7ffb7982ab10, 0x7ffb7982a210, 0x7ffb7982cf10

  Optimized lowered selection DAG: BB#0 'main:'
  SelectionDAG has 17 nodes:
  ...
   0x7ffb7982ab10: <multiple use>
      0x7ffb7982ab10: <multiple use>
      0x7ffb7982a210: <multiple use>
      0x7ffb7982cf10: ch = seteq

    0x7ffb7982d210: i1 = setcc 0x7ffb7982ab10, 0x7ffb7982a210, 0x7ffb7982cf10

    0x7ffb7982b010: i32 = zero_extend 0x7ffb7982d210 [ORD=7]
  ...
  Type-legalized selection DAG: BB#0 'main:entry'
  SelectionDAG has 18 nodes:
  ...
    0x7ffb7982ab10: <multiple use>
      0x7ffb7982ab10: <multiple use>
      0x7ffb7982a210: <multiple use>
      0x7ffb7982cf10: ch = seteq [ID=-3]

    0x7ffb7982ac10: i32 = setcc 0x7ffb7982ab10, 0x7ffb7982a210, 0x7ffb7982cf10
     [ID=-3]

    0x7ffb7982ad10: i32 = Constant<1> [ID=-3]

    0x7ffb7982ae10: i32 = and 0x7ffb7982ac10, 0x7ffb7982ad10 [ID=-3]
  ...
  ISEL: Starting pattern match on root node: 0x7ffb7982ac10: i32 = setcc 
  0x7ffb7982ab10, 0x7ffb7982a210, 0x7ffb7982cf10 [ID=14]
  
  Initial Opcode index to 0
  Match failed at index 0
  LLVM ERROR: Cannot select: 0x7ffb7982ac10: i32 = setcc 0x7ffb7982ab10, 
  0x7ffb7982a210, 0x7ffb7982cf10 [ID=14]
  0x7ffb7982ab10: i32,ch = load 0x7ffb7982aa10, 0x7ffb7982a710, 
  0x7ffb7982a410<LD4[%a]> [ORD=4] [ID=13]
  0x7ffb7982a710: i32 = FrameIndex<1> [ORD=2] [ID=5]
  0x7ffb7982a410: i32 = undef [ORD=1] [ID=3]
  0x7ffb7982a210: i32 = Constant<0> [ORD=1] [ID=1]
  In function: main


Summary as Table: C operator ! corresponding IR of .bc and IR of DAG.


.. table:: C operator ! corresponding IR of .bc and IR of DAG

  ============================= ================================= =============================
  IR of .bc                     Optimized lowered selection DAG   Type-legalized selection DAG
  ============================= ================================= =============================
  %tobool = icmp ne i32 %0, 0     
  %lnot = xor i1 %tobool, true  %lnot = (setcc %tobool, 0, seteq) %lnot = (setcc %tobool, 0, seteq)
  %conv = zext i1 %lnot to i32  %conv = (zero_extend %lnot)       %conv = (and %lnot, 1)
  ============================= ================================= =============================

From above DAG translation result of ``llc -debug``, we see the IRs are same in both 
stages of “Initial selection DAG” and “Optimized lowered selection DAG”.

The (setcc %0, 0, setne) and (xor %tobool, -1) in “Initial selection DAG” stage 
corresponding (icmp %0, 0, ne) and (xor %tobool, 1) in ch4_2.bc. 
The argument in xor is 1 bit size (1 and -1 are same, they are all represented 
by 1). 
The (zero_extend %lnot) of “Initial selection DAG” corresponding 
(zext i1 %lnot to i32) of ch4_2.bc. 
As above it translate 2 DAG nodes (setcc %0, 0, setne) and (xor %tobool, -1) 
into 1 DAG node (setcc %tobool, 0, seteq) in “Optimized lowered selection DAG” 
stage. 
This translation is right since for 1 bit size, (xor %tobool, 1) and 
(not %tobool) has same result; and (not (setcc %tobool, 0, setne)) is equal to 
(setcc %tobool, 0, seteq). 
In “Type-legalized selection DAG” stage, it translate (zero_extern i1 
%lnot to 32) into (and %lnot, 1). 
(zero_extern i1 %lnot to 32) just expand the %lnot to i32 32 bits result, so 
translate into (and %lnot, 1) is correct. 

Finally, according the DAG translation message, it fails at 
(setcc %tobool, 0, seteq).
Run it with Chapter4_2/ which added code to handle pattern 
(setcc %tobool, 0, seteq) as below, to get the following result.

.. rubric:: LLVMBackendTutorialExampleCode/Chapter4_2/Cpu0InstrInfo.td
.. code-block:: c++
  
  def : Pat<(not CPURegs:$in),
      (XOR CPURegs:$in, (ADDiu ZERO, 1))>;
  
  // setcc patterns
  multiclass SeteqPats<RegisterClass RC, Instruction XOROp> {
  def : Pat<(seteq RC:$lhs, RC:$rhs),
        (XOROp (XOROp RC:$lhs, RC:$rhs), (ADDiu ZERO, 1))>;
  }
  
  defm : SeteqPats<CPURegs, XOR>;


.. code-block:: bash

  118-165-78-230:InputFiles Jonathan$ /Users/Jonathan/llvm/test/cmake_debug_build/
  bin/Debug/llc -march=cpu0 -relocation-model=pic -debug -filetype=asm ch4_2.bc 
  -o ch4_2.cpu0.s
  ...
  ISEL: Starting pattern match on root node: 0x7fbc6902ac10: i32 = setcc 
  0x7fbc6902ab10, 0x7fbc6902a210, 0x7fbc6902cf10 [ID=14]
  
  Initial Opcode index to 365
  Created node: 0x7fbc6902af10: i32 = XOR 0x7fbc6902ab10, 0x7fbc6902a210
  
  Created node: 0x7fbc6902d510: i32 = ADDiu 0x7fbc6902d310, 0x7fbc6902d410
  
  Morphed node: 0x7fbc6902ac10: i32 = XOR 0x7fbc6902af10, 0x7fbc6902d510
  
  ISEL: Match complete!
  => 0x7fbc6902ac10: i32 = XOR 0x7fbc6902af10, 0x7fbc6902d510


Summary as Table: C operator ! corresponding IR of DAG and .


.. table:: C operator ! corresponding IR of Type-legalized selection DAG 
           (include and after this stage) and Cpu0 instructions

  ==============================================      ==========================
  Include and after Type-legalized selection DAG      Cpu0 instruction
  ==============================================      ==========================
  %lnot = (setcc %tobool, 0, seteq)                   - %1 = (xor %tobool, 0)
                                                      - %true = (addiu $r0, 1)
                                                      - %lnot = (xor %1, %true)
  %conv = (and %lnot, 1)                              %conv = (and %lnot, 1)
  ==============================================      ==========================

Chapter4_2/ defined seteq DAG pattern. 
It translate **%lnot = (setcc %tobool, 0, seteq)** into **%1 = (xor %tobool, 0)**, 
**%true = (addiu $r0, 1)** and **%lnot = (xor %1, %true)** in 
“Instruction selection” stage according the rules defined in Cpu0InstrInfo.td as 
above. 
This translation is right based on the following truth:

1. %lnot = 1 when %tobool = 0 and $lnot = 0 when %tobool!=0. 

2. %true = (addiu $r0, 1) always is 1 since $r0 is zero. %tobool is 0 or 1. 
   When %tobool = 0, %1 = 1 and %lnot = (xor %1, %true) = 0; when %tobool = 1, 
   %1 = 0 and %lnot = (xor 0, %true) = 1.

3. When %tobool = !0, %1 != 1 and %lnot = (xor %1, %true) != 0.

After xor, the IR (and %lnot, 1) is translated into Cpu0 (and $lnot, 1) which is 
defined before. 
List the asm file ch4_2.cpu0.s as below, you can check it with 
the final result. 

.. code-block:: bash

  118-165-16-22:InputFiles Jonathan$ cat ch4_2.cpu0.s
  ...
  # BB#0:
    addiu $sp, $sp, -16
  $tmp1:
    .cfi_def_cfa_offset 16
    addiu $2, $zero, 0
    st  $2, 12($sp)
    addiu $3, $zero, 5
    st  $3, 8($sp)
    st  $2, 4($sp)
    ld  $3, 8($sp)
    xor $2, $3, $2
    addiu $3, $zero, 1
    xor $2, $2, $3
    and $2, $2, $3
    st  $2, 4($sp)
    addiu $sp, $sp, 16
    ret $lr
    ...


