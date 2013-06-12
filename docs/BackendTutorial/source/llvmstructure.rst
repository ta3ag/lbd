.. _sec-llvmstructure:

Cpu0 Instruction Set and LLVM Target Description
================================================

Before you begin this tutorial, you should know that you can always try to develop your 
own backend by porting code from existing backends.  The majority of the code you will 
want to investigate can be found in the /lib/Target directory of your root LLVM 
installation. As most major RISC instruction sets have some similarities, this may be the 
avenue you might try if you are an experienced programmer and knowledgable of compiler 
backends.

On the other hand, there is a steep learning curve and you may easily get stuck 
debugging your new backend. You can easily spend a lot of time tracing which 
methods are callbacks of some function, or which are calling some overridden 
method deep in the LLVM codebase - and with a codebase as large as LLVM, all of this 
can easily become difficult to keep track of. This tutorial will help you work through 
this process while learning the fundamentals of LLVM backend design. It will show 
you what is necessary to get your first backend functional and complete, and it 
should help you understand how to debug your backend when it produces incorrect machine 
code using output provided by the compiler.

This section details the Cpu0 instruction set and the structure of LLVM. 
The LLVM structure information is adapted from Chris Lattner's LLVM chapter of the 
Architecture of Open Source Applications book [#aosa-book]_. You can read 
the original article from the AOSA website if you prefer. Finally, you will begin to 
create a new LLVM backend by writing register and instruction definitions in the 
Target Description files which will be used in next section.

Cpu0 Processor Architecture Details
-----------------------------------

This subsection is based on materials available here [#cpu0-chinese]_ (Chinese)
and [#cpu0-english]_ (English).

Brief introduction
++++++++++++++++++

Cpu0 is a 32-bit architecture. It has 16 general purpose registers (R0, ..., R15), the 
Instruction Register (IR), the memory access registers MAR & MDR. Its structure is 
illustrated in :num:`Figure #llvmstructure-f1` below.

.. _llvmstructure-f1: 
.. figure:: ../Fig/llvmstructure/1.png
	:align: center

	Architectural block diagram of the Cpu0 processor


The registers are used for the following purposes:

============	===========
Register		Description
============	===========
IR				Instruction register
R0				Constant register, value is 0
R1-R11			General-purpose registers
R12				Status Word register (SW)
R13				Stack Pointer register (SP)
R14				Link Register (LR)
R15				Program Counter (PC)
MAR				Memory Address Register (MAR)
MDR				Memory Data Register (MDR)
HI				High part of MULT result
LO				Low part of MULT result
============	===========

The Cpu0 Instruction Set
++++++++++++++++++++++++

The Cpu0 instruction set can be divided into three types: L-type instructions, which are 
generally associated with memory operations, A-type instructions for arithmetic 
operations, and J-type instructions that are typically used when altering control flow 
(i.e. jumps).  :num:`Figure #llvmstructure-f2` illustrates how the bitfields are broken 
down for each type of instruction.

.. _llvmstructure-f2: 
.. figure:: ../Fig/llvmstructure/2.png
	:align: center

	Cpu0's three instruction formats

The following table details the Cpu0 instruction set:

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
	* - A
	  - CMP
	  - 10
	  - Compare
	  - CMP Ra, Rb
	  - SW <= (Ra cond Rb) [#cond-note]_
	* - A
	  - MOV
	  - 12
	  - Move
	  - MOV Ra, Rb
	  - Ra <= Rb
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
	  - SRA
	  - 1B
	  - Shift right
	  - SHR Ra, Rb, Cx
	  - Ra <= (h80000000|Rb>>Cx)
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
   	* - J
	  - JEQ
	  - 20
	  - Jump if equal (==)
	  - JEQ Cx
	  - if SW(==), PC <= PC + Cx
   	* - J
	  - JNE
	  - 21
	  - Jump if not equal (!=)
	  - JNE Cx
	  - if SW(!=), PC <= PC + Cx
   	* - J
	  - JLT
	  - 22
	  - Jump if less than (<)
	  - JLT Cx
	  - if SW(<), PC <= PC + Cx
   	* - J
	  - JGT
	  - 23
	  - Jump if greater than (>)
	  - JGT Cx
	  - if SW(>), PC <= PC + Cx
   	* - J
	  - JLE
	  - 24
	  - Jump if less than or equals (<=)
	  - JLE Cx
	  - if SW(<=), PC <= PC + Cx
   	* - J
	  - JGE
	  - 25
	  - Jump if greater than or equals (>=)
	  - JGE Cx
	  - if SW(>=), PC <= PC + Cx
   	* - J
	  - JMP
	  - 26
	  - Jump (unconditional)
	  - JMP Cx
	  - PC <= PC + Cx
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
   	* - A
	  - PUSH
	  - 30
	  - Push word
	  - PUSH Ra
	  - [SP] <= Ra; SP -= 4
   	* - A
	  - POP
	  - 31
	  - Pop word
	  - POP Ra
	  - Ra <= [SP]; SP += 4
   	* - A
	  - PUSHB
	  - 32
	  - Push byte
	  - PUSHB Ra
	  - [SP] <= (byte)Ra; SP -= 4
   	* - A
	  - POPB
	  - 33
	  - Pop word
	  - POP Ra
	  - Ra <= (byte)[SP]; SP += 4
   	* - L
	  - MFHI
	  - 40
	  - Move HI to GPR
	  - MFHI Ra
	  - Ra <= HI
   	* - L
	  - MFLO
	  - 41
	  - Move LO to GPR
	  - MFLO Ra
	  - Ra <= LO
   	* - L
	  - MTHI
	  - 42
	  - Move GPR to HI
	  - MTHI Ra
	  - HI <= Ra
   	* - L
	  - MTLO
	  - 43
	  - Move GPR to LO
	  - MTLO Ra
	  - LO <= Ra
   	* - L
	  - MULT
	  - 50
	  - Multiply for 64 bits result
	  - MULT Ra, Rb
	  - (HI,LO) <= MULT(Ra,Rb)
   	* - L
	  - MULTU
	  - 51
	  - MULT for unsigned 64 bits
	  - MULTU Ra, Rb
	  - (HI,LO) <= MULTU(Ra,Rb)




The Status Register
+++++++++++++++++++

The Cpu0 status word register (SW) contains the state of the Negative (N), Zero (Z), 
Carry (C), Overflow (V), and Interrupt (I), Trap (T), and Mode (M) boolean flags. 
The bit layout of the SW register is shown in :num:`Figure #llvmstructure-f3` below.

.. _llvmstructure-f3: 
.. figure:: ../Fig/llvmstructure/3.png
	:align: center

	Cpu0 status word (SW) register

When a CMP Ra, Rb instruction executes, the condition flags will change. For example:

-	If Ra > Rb, then N = 0, Z = 0
-	If Ra < Rb, then N = 1, Z = 0
-	If Ra = Rb, then N = 0, Z = 1

The direction (i.e. taken/not taken) of the conditional jump instructions JGT, JLT, JGE, 
JLE, JEQ, JNE is determined by the N and Z flags in the SW register.

Cpu0's Stages of Instruction Execution
++++++++++++++++++++++++++++++++++++++

The Cpu0 architecture has a three-stage pipeline.  The stages are instruction fetch (IF), 
decode (D), and execute (EX), and they occur in that order.  Here is a description of 
what happens in the processor:

1) Instruction fetch

-	The Cpu0 fetches the instruction pointed to by the Program Counter (PC) into the 
	Instruction Register (IR): IR = [PC].
-	The PC is then updated to point to the next instruction: PC = PC + 4.

2) Decode

-	The control unit decodes the instruction stored in IR, which routes necessary data
	stored in registers to the ALU, and sets the ALU's operation mode based on the 
	current instruction's opcode.

3) Execute

-	The ALU executes the operation designated by the control unit upon data in registers. 
	After the ALU is done, the result is stored in the destination register. 

LLVM Structure
--------------

The text in this and the following section comes from the AOSA chapter on LLVM written 
by Chris Lattner [#aosa-book]_.

The most popular design for a traditional static compiler (like most C 
compilers) is the three phase design whose major components are the front end, 
the optimizer and the back end, as seen in :num:`Figure #llvmstructure-f6`. 
The front end parses source code, checking it for errors, and builds a 
language-specific Abstract Syntax Tree (AST) to represent the input code. 
The AST is optionally converted to a new representation for optimization, and 
the optimizer and back end are run on the code.

.. _llvmstructure-f6: 
.. figure:: ../Fig/llvmstructure/6.png
	:align: center

	Three Major Components of a Three Phase Compiler

The optimizer is responsible for doing a broad variety of transformations to 
try to improve the code's running time, such as eliminating redundant 
computations, and is usually more or less independent of language and target. 
The back end (also known as the code generator) then maps the code onto the 
target instruction set. 
In addition to making correct code, it is responsible for generating good code 
that takes advantage of unusual features of the supported architecture. 
Common parts of a compiler back end include instruction selection, register 
allocation, and instruction scheduling.

This model applies equally well to interpreters and JIT compilers. 
The Java Virtual Machine (JVM) is also an implementation of this model, which 
uses Java bytecode as the interface between the front end and optimizer.

The most important win of this classical design comes when a compiler decides 
to support multiple source languages or target architectures. 
If the compiler uses a common code representation in its optimizer, then a 
front end can be written for any language that can compile to it, and a back 
end can be written for any target that can compile from it, as shown in 
:num:`Figure #llvmstructure-f7`.

.. _llvmstructure-f7: 
.. figure:: ../Fig/llvmstructure/7.png
	:align: center
	:width: 600px

	Retargetablity

With this design, porting the compiler to support a new source language (e.g., 
Algol or BASIC) requires implementing a new front end, but the existing 
optimizer and back end can be reused. 
If these parts weren't separated, implementing a new source language would 
require starting over from scratch, so supporting N targets and M source 
languages would need N*M compilers.

Another advantage of the three-phase design (which follows directly from 
retargetability) is that the compiler serves a broader set of programmers than 
it would if it only supported one source language and one target. 
For an open source project, this means that there is a larger community of 
potential contributors to draw from, which naturally leads to more enhancements 
and improvements to the compiler. 
This is the reason why open source compilers that serve many communities (like 
GCC) tend to generate better optimized machine code than narrower compilers 
like FreePASCAL. 
This isn't the case for proprietary compilers, whose quality is directly 
related to the project's budget. 
For example, the Intel ICC Compiler is widely known for the quality of code it 
generates, even though it serves a narrow audience.

A final major win of the three-phase design is that the skills required to 
implement a front end are different than those required for the optimizer and 
back end. 
Separating these makes it easier for a "front-end person" to enhance and 
maintain their part of the compiler. 
While this is a social issue, not a technical one, it matters a lot in 
practice, particularly for open source projects that want to reduce the barrier 
to contributing as much as possible.

The most important aspect of its design is the LLVM Intermediate Representation 
(IR), which is the form it uses to represent code in the compiler. 
LLVM IR is designed to host mid-level analyses and transformations that you 
find in the optimizer section of a compiler. 
It was designed with many specific goals in mind, including supporting 
lightweight runtime optimizations, cross-function/interprocedural 
optimizations, whole program analysis, and aggressive restructuring 
transformations, etc. 
The most important aspect of it, though, is that it is itself defined as a 
first class language with well-defined semantics. 
To make this concrete, here is a simple example of a .ll file:

.. code-block:: c++

  define i32 @add1(i32 %a, i32 %b) {
  entry:
    %tmp1 = add i32 %a, %b
    ret i32 %tmp1
  }
  define i32 @add2(i32 %a, i32 %b) {
  entry:
    %tmp1 = icmp eq i32 %a, 0
    br i1 %tmp1, label %done, label %recurse
  recurse:
    %tmp2 = sub i32 %a, 1
    %tmp3 = add i32 %b, 1
    %tmp4 = call i32 @add2(i32 %tmp2, i32 %tmp3)
    ret i32 %tmp4
  done:
    ret i32 %b
  }
  // This LLVM IR corresponds to this C code, which provides two different ways to
  //  add integers:
  unsigned add1(unsigned a, unsigned b) {
    return a+b;
  }
  // Perhaps not the most efficient way to add two numbers.
  unsigned add2(unsigned a, unsigned b) {
    if (a == 0) return b;
    return add2(a-1, b+1);
  }

As you can see from this example, LLVM IR is a low-level RISC-like virtual 
instruction set. 
Like a real RISC instruction set, it supports linear sequences of simple 
instructions like add, subtract, compare, and branch. 
These instructions are in three address form, which means that they take some 
number of inputs and produce a result in a different register. 
LLVM IR supports labels and generally looks like a weird form of assembly 
language.

Unlike most RISC instruction sets, LLVM is strongly typed with a simple type 
system (e.g., i32 is a 32-bit integer, i32** is a pointer to pointer to 32-bit 
integer) and some details of the machine are abstracted away. 
For example, the calling convention is abstracted through call and ret 
instructions and explicit arguments. 
Another significant difference from machine code is that the LLVM IR doesn't 
use a fixed set of named registers, it uses an infinite set of temporaries 
named with a % character.

Beyond being implemented as a language, LLVM IR is actually defined in three 
isomorphic forms: the textual format above, an in-memory data structure 
inspected and modified by optimizations themselves, and an efficient and dense 
on-disk binary "bitcode" format. 
The LLVM Project also provides tools to convert the on-disk format from text to 
binary: llvm-as assembles the textual .ll file into a .bc file containing the 
bitcode goop and llvm-dis turns a .bc file into a .ll file.

The intermediate representation of a compiler is interesting because it can be 
a "perfect world" for the compiler optimizer: unlike the front end and back end 
of the compiler, the optimizer isn't constrained by either a specific source 
language or a specific target machine. 
On the other hand, it has to serve both well: it has to be designed to be easy 
for a front end to generate and be expressive enough to allow important 
optimizations to be performed for real targets.
	

.td: LLVM's Target Description Files
------------------------------------

The "mix and match" approach allows target authors to choose what makes sense 
for their architecture and permits a large amount of code reuse across 
different targets. 
This brings up another challenge: each shared component needs to be able to 
reason about target specific properties in a generic way. 
For example, a shared register allocator needs to know the register file of 
each target and the constraints that exist between instructions and their 
register operands. 
LLVM's solution to this is for each target to provide a target description 
in a declarative domain-specific language (a set of .td files) processed by the 
tblgen tool. 
The (simplified) build process for the x86 target is shown in 
:num:`Figure #llvmstructure-f8`.

.. _llvmstructure-f8: 
.. figure:: ../Fig/llvmstructure/8.png
	:align: center
	:width: 600px

	Simplified x86 Target Definition

The different subsystems supported by the .td files allow target authors to 
build up the different pieces of their target. 
For example, the x86 back end defines a register class that holds all of its 
32-bit registers named "GR32" (in the .td files, target specific definitions 
are all caps) like this:

.. code-block:: c++

  def GR32 : RegisterClass<[i32], 32,
    [EAX, ECX, EDX, ESI, EDI, EBX, EBP, ESP,
     R8D, R9D, R10D, R11D, R14D, R15D, R12D, R13D]> { ... }


Creating the Initial Cpu0 .td Files
-----------------------------------

As has been discussed in the previous section, LLVM uses target description files 
(which use the .td file extension) to describe various components of a target's backend. 
For example, these .td files may describe a target's register set, instruction set, 
scheduling information for instructions, and calling conventions.  When your backend is 
being compiled, the tablegen tool that ships with LLVM will translate these .td files 
into C++ source code written to files that have a .inc extension.  Please refer 
to [#tablegen]_ for more information regarding how to use tablegen.

Every backend has a .td which defines some target information, including what other .td 
files are used by the backend.  These files have a similar syntax to C++. For Cpu0, the 
target description file is called Cpu0.td, which is shown below:

.. rubric:: LLVMBackendTutorialExampleCode/Chapter2/Cpu0.td
.. literalinclude:: ../LLVMBackendTutorialExampleCode/Chapter2/Cpu0.td
    :linenos:

Cpu0.td includes a few other .td files.  Cpu0RegisterInfo.td (shown below) describes the 
Cpu0's set of registers.  In this file, we see that registers have been given names, i.e.
``def PC`` indicates that there is a register called PC.  Also, there is a register class 
named ``CPURegs`` that contains all of the other registers.  You may have multiple 
register classes (see the X86 backend, for example) which can help you if certain 
instructions can only write to specific registers.  In this case, there is only one set 
of general purpose registers for Cpu0, and some registers that are reserved so that they 
are not modified by instructions during execution.

.. rubric:: LLVMBackendTutorialExampleCode/Chapter2/Cpu0RegisterInfo.td
.. literalinclude:: ../LLVMBackendTutorialExampleCode/Chapter2/Cpu0RegisterInfo.td
    :linenos:


In C++, classes typically provide a structure to lay out some data and functions, 
while definitions are used to allocate memory for specific instances of a class.  For 
example:

.. code-block:: c++

  class Date {  // declare Date
    int year, month, day;
  }; 
  Date birthday;  // define birthday, an instance of Date

The class ``Date`` has the members ``year``, ``month``, and ``day``, however these do not  
yet belong to an actual object.  By defining an instance of ``Date`` called ``birthday``, 
you have allocated memory for a specific object, and can set the ``year``, ``month``, and 
``day`` of this instance of the class.

In .td files, classes describe the structure of how data is laid out, while definitions 
act as the specific instances of the classes.  If we look back at the Cpu0RegisterInfo.td 
file, we see a class called ``Cpu0Reg<string n>`` which is derived from the 
``Register<n>`` class provided by LLVM.  ``Cpu0Reg`` inherits all the fields that exist 
in the ``Register`` class, and also adds a new field called ``Num`` which is four bits 
wide.

The ``def`` keyword is used to create instances of classes.  In the following line, the 
ZERO register is defined as a member of the ``Cpu0GPRReg`` class:

.. code-block:: c++

	def ZERO : Cpu0GPRReg< 0, "ZERO">, DwarfRegNum<[0]>;

The ``def ZERO`` indicates the name of this register.  ``< 0, "ZERO">`` are the 
parameters used when creating this specific instance of the ``Cpu0GPRReg`` class, thus 
the four bit ``Num`` field is set to 0, and the string ``n`` is set to ``ZERO``.

As the register lives in the ``Cpu0`` namespace, you can refer to the ZERO register in 
C++ code in a backend using ``Cpu0::ZERO``.

.. todo:: I might want to re-edit the following paragraph

Notice the use of the ``let`` expressions: these allow you to override values that are 
initially defined in a superclass. For example, ``let Namespace = “Cpu0”`` in the 
``Cpu0Reg`` class will override the default namespace declared in ``Register`` class. 
The Cpu0RegisterInfo.td also defines that ``CPURegs`` is an instance of the class 
``RegisterClass``, which is an built-in LLVM class.  A ``RegisterClass`` is a set of 
``Register`` instances, thus ``CPURegs`` can be described as a set of registers.

.. end editing 1/23

The cpu0 instructions td is named to Cpu0InstrInfo.td which contents as follows,

.. rubric:: LLVMBackendTutorialExampleCode/Chapter2/Cpu0InstrInfo.td
.. literalinclude:: ../LLVMBackendTutorialExampleCode/Chapter2/Cpu0InstrInfo.td
    :linenos:


The Cpu0InstrFormats.td is included by Cpu0InstInfo.td as follows,

.. rubric:: LLVMBackendTutorialExampleCode/Chapter2/Cpu0InstrFormats.td
.. literalinclude:: ../LLVMBackendTutorialExampleCode/Chapter2/Cpu0InstrFormats.td
    :linenos:


ADDiu is class ArithLogicI inherited from FL, can expand and get member value 
as follows,

.. code-block:: c++

  def ADDiu   : ArithLogicI<0x09, "addiu", add, simm16, immSExt16, CPURegs>;
  
  /// Arithmetic and logical instructions with 2 register operands.
  class ArithLogicI<bits<8> op, string instr_asm, SDNode OpNode,
            Operand Od, PatLeaf imm_type, RegisterClass RC> :
    FL<op, (outs RC:$ra), (ins RC:$rb, Od:$imm16),
     !strconcat(instr_asm, "\t$ra, $rb, $imm16"),
     [(set RC:$ra, (OpNode RC:$rb, imm_type:$imm16))], IIAlu> {
    let isReMaterializable = 1;
  }
  
  So,
  op = 0x09
  instr_asm = “addiu”
  OpNode = add
  Od = simm16
  imm_type = immSExt16
  RC = CPURegs

Expand with FL further,


.. code-block:: c++

   :  FL<op, (outs RC:$ra), (ins RC:$rb, Od:$imm16),
     !strconcat(instr_asm, "\t$ra, $rb, $imm16"), 
     [(set RC:$ra, (OpNode RC:$rb, imm_type:$imm16))], IIAlu>
  
  class FL<bits<8> op, dag outs, dag ins, string asmstr, list<dag> pattern, 
       InstrItinClass itin>: Cpu0Inst<outs, ins, asmstr, pattern, itin, FrmL>
  { 
    bits<4>  ra; 
    bits<4>  rb; 
    bits<16> imm16; 
  
    let Opcode = op; 
  
    let Inst{23-20} = ra; 
    let Inst{19-16} = rb; 
    let Inst{15-0}  = imm16; 
  }
  
  So,
  op = 0x09
  outs = CPURegs:$ra
  ins = CPURegs:$rb,simm16:$imm16
  asmstr = "addiu\t$ra, $rb, $imm16"
  pattern = [(set CPURegs:$ra, (add RC:$rb, immSExt16:$imm16))]
  itin = IIAlu
  
  Members are,
  ra = CPURegs:$ra
  rb = CPURegs:$rb
  imm16 = simm16:$imm16
  Opcode = 0x09;
  Inst{23-20} = CPURegs:$ra; 
  Inst{19-16} = CPURegs:$rb; 
  Inst{15-0}  = simm16:$imm16; 

Expand with Cpu0Inst further,

.. code-block:: c++

  class FL<bits<8> op, dag outs, dag ins, string asmstr, list<dag> pattern, 
       InstrItinClass itin>: Cpu0Inst<outs, ins, asmstr, pattern, itin, FrmL>
  
  class Cpu0Inst<dag outs, dag ins, string asmstr, list<dag> pattern, 
           InstrItinClass itin, Format f>: Instruction 
  { 
    field bits<32> Inst; 
    Format Form = f; 
  
    let Namespace = "Cpu0"; 
  
    let Size = 4; 
  
    bits<8> Opcode = 0; 
  
    // Top 8 bits are the 'opcode' field 
    let Inst{31-24} = Opcode; 
  
    let OutOperandList = outs; 
    let InOperandList  = ins; 
  
    let AsmString   = asmstr; 
    let Pattern     = pattern; 
    let Itinerary   = itin; 
  
    // 
    // Attributes specific to Cpu0 instructions... 
    // 
    bits<4> FormBits = Form.Value; 
  
    // TSFlags layout should be kept in sync with Cpu0InstrInfo.h. 
    let TSFlags{3-0}   = FormBits; 
  
    let DecoderNamespace = "Cpu0"; 
  
    field bits<32> SoftFail = 0; 
  }
  
  So,
  outs = CPURegs:$ra
  ins = CPURegs:$rb,simm16:$imm16
  asmstr = "addiu\t$ra, $rb, $imm16"
  pattern = [(set CPURegs:$ra, (add RC:$rb, immSExt16:$imm16))]
  itin = IIAlu
  f =  FrmL
  
  Members are,
  Inst{31-24} = 0x09; 
  OutOperandList = CPURegs:$ra 
  InOperandList  = CPURegs:$rb,simm16:$imm16
  AsmString = "addiu\t$ra, $rb, $imm16"
  Pattern = [(set CPURegs:$ra, (add RC:$rb, immSExt16:$imm16))]
  Itinerary = IIAlu
  
  Summary with all members are, 
  // Inherited from parent like Instruction
  Namespace = "Cpu0";
  DecoderNamespace = "Cpu0";
  Inst{31-24} = 0x08; 
  Inst{23-20} = CPURegs:$ra; 
  Inst{19-16} = CPURegs:$rb; 
  Inst{15-0}  = simm16:$imm16; 
  OutOperandList = CPURegs:$ra 
  InOperandList  = CPURegs:$rb,simm16:$imm16
  AsmString = "addiu\t$ra, $rb, $imm16"
  Pattern = [(set CPURegs:$ra, (add RC:$rb, immSExt16:$imm16))]
  Itinerary = IIAlu
  // From Cpu0Inst
  Opcode = 0x09;
  // From FL
  ra = CPURegs:$ra
  rb = CPURegs:$rb
  imm16 = simm16:$imm16

It's a lousy process. 
Similarly, LD and ST instruction definition can be expanded in this way. 
Please notify the Pattern =  
[(set CPURegs:$ra, (add RC:$rb, immSExt16:$imm16))] which include keyword 
**“add”**. 
We will use it in DAG transformations later. 


Write cmake file
----------------

Target/Cpu0 directory has two files CMakeLists.txt and LLVMBuild.txt, 
contents as follows,

.. rubric:: LLVMBackendTutorialExampleCode/Chapter2/CMakeLists.txt
.. literalinclude:: ../LLVMBackendTutorialExampleCode/Chapter2/CMakeLists.txt
    :linenos:

.. rubric:: LLVMBackendTutorialExampleCode/Chapter2/LLVMBuild.txt
.. literalinclude:: ../LLVMBackendTutorialExampleCode/Chapter2/LLVMBuild.txt
    :linenos:


CMakeLists.txt is the make information for cmake, # is comment.
LLVMBuild.txt files are written in a simple variant of the INI or configuration 
file format. 
Comments are prefixed by ``#`` in both files. 
We explain the setting for these 2 files in comments. 
Please spend a little time to read it.

Both CMakeLists.txt and LLVMBuild.txt coexist in sub-directories 
``MCTargetDesc`` and ``TargetInfo``. 
Their contents indicate they will generate Cpu0Desc and Cpu0Info libraries. 
After building, you will find three libraries: ``libLLVMCpu0CodeGen.a``, 
``libLLVMCpu0Desc.a`` and ``libLLVMCpu0Info.a`` in lib/ of your build 
directory. 
For more details please see 
"Building LLVM with CMake" [#cmake]_ and 
"LLVMBuild Guide" [#llvmbuild]_.

Target Registration
-------------------

You must also register your target with the TargetRegistry, which is what other 
LLVM tools use to be able to lookup and use your target at runtime. 
The TargetRegistry can be used directly, but for most targets there are helper 
templates which should take care of the work for you.

All targets should declare a global Target object which is used to represent 
the target during registration. 
Then, in the target's TargetInfo library, the target should define that object 
and use the RegisterTarget template to register the target. 
For example, the file TargetInfo/Cpu0TargetInfo.cpp register TheCpu0Target for 
big endian and TheCpu0elTarget for little endian, as follows.

.. rubric:: LLVMBackendTutorialExampleCode/Chapter2/TargetInfo/Cpu0TargetInfo.cpp
.. literalinclude:: ../LLVMBackendTutorialExampleCode/Chapter2/TargetInfo/Cpu0TargetInfo.cpp
    :linenos:


Files Cpu0TargetMachine.cpp and MCTargetDesc/Cpu0MCTargetDesc.cpp just define 
the empty initialize function since we register nothing in them for this moment.

.. rubric:: LLVMBackendTutorialExampleCode/Chapter2/MCTargetDesc/Cpu0MCTargetDesc.h
.. literalinclude:: ../LLVMBackendTutorialExampleCode/Chapter2/MCTargetDesc/Cpu0MCTargetDesc.h
    :linenos:

.. rubric:: LLVMBackendTutorialExampleCode/Chapter2/MCTargetDesc/Cpu0MCTargetDesc.cpp
.. literalinclude:: ../LLVMBackendTutorialExampleCode/Chapter2/MCTargetDesc/Cpu0MCTargetDesc.cpp
    :linenos:


Please see "Target Registration" [#target-reg]_ for reference.


Build libraries and td
----------------------

The llvm source code is put in /Users/Jonathan/llvm/release/src and have llvm 
release-build in /Users/Jonathan/llvm/release/configure_release_build. 
About how to build llvm, please refer [#clang]_. 
We made a copy from /Users/Jonathan/llvm/release/src to 
/Users/Jonathan/llvm/test/src for working with my Cpu0 target back end.
Sub-directories src is for source code and cmake_debug_build is for debug 
build directory.

Except directory src/lib/Target/Cpu0, there are a couple of files modified to 
support cpu0 new Target. 
Please check files in src_files_modify/src_files_modified/src/. 

You can update your llvm working copy and find the modified files by 
command,

.. code-block:: bash

  cp -rf LLVMBackendTutorialExampleCode/src_files_modified/src_files_modified/src/
  * yourllvm/workingcopy/sourcedir/.
  
  118-165-78-230:test Jonathan$ pwd
  /Users/Jonathan/test
  118-165-78-230:test Jonathan$ grep -R "cpu0" src
  src/cmake/config-ix.cmake:elseif (LLVM_NATIVE_ARCH MATCHES "cpu0")
  src/include/llvm/ADT/Triple.h:#undef cpu0
  src/include/llvm/ADT/Triple.h:    cpu0,    // Gamma add
  src/include/llvm/ADT/Triple.h:    cpu0el,
  src/include/llvm/Support/ELF.h:  EF_CPU0_ARCH_32R2 = 0x70000000, // cpu032r2
  src/include/llvm/Support/ELF.h:  EF_CPU0_ARCH_64R2 = 0x80000000, // cpu064r2
  src/lib/Support/Triple.cpp:  case cpu0:    return "cpu0";
  ...


Now, run the ``cmake`` command and Xcode to build td (the following cmake command is 
for my setting),

.. code-block:: bash

  118-165-78-230:test Jonathan$ cmake -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_
  C_COMPILER=clang -DCMAKE_BUILD_TYPE=Debug  -G "Unix Makefiles" ../src/
  
  -- Targeting Cpu0 
  ...
  -- Targeting XCore 
  -- Configuring done 
  -- Generating done 
  -- Build files have been written to: /Users/Jonathan/llvm/test/cmake_debug
  _build 
  
  118-165-78-230:test Jonathan$

After build, you can type command ``llc –version`` to find the cpu0 backend,

.. code-block:: bash

  118-165-78-230:test Jonathan$ /Users/Jonathan/llvm/test/cmake_debug_build/bin/
  Debug/llc --version
  LLVM (http://llvm.org/):
  ...
    Registered Targets: 
    arm      - ARM 
    cellspu  - STI CBEA Cell SPU [experimental] 
    cpp      - C++ backend 
    cpu0     - Cpu0 
    cpu0el   - Cpu0el 
  ...

The ``llc -version`` can display **“cpu0”** and **“cpu0el”** message, because 
the following code from file TargetInfo/Cpu0TargetInfo.cpp what in 
"section Target Registration" [#asadasd]_ we made. 
List them as follows again,

.. rubric:: LLVMBackendTutorialExampleCode/Chapter2/TargetInfo/Cpu0TargetInfo.cpp
.. literalinclude:: ../LLVMBackendTutorialExampleCode/Chapter2/TargetInfo/Cpu0TargetInfo.cpp
    :linenos:


Let's build LLVMBackendTutorialExampleCode/Chapter2 code as follows,

.. code-block:: bash

  118-165-75-57:ExampleCode Jonathan$ pwd
  /Users/Jonathan/llvm/test/src/lib/Target/Cpu0/ExampleCode
  118-165-75-57:ExampleCode Jonathan$ sh removecpu0.sh 
  118-165-75-57:ExampleCode Jonathan$ cp -rf LLVMBackendTutorialExampleCode/Chapter2/
  * ../.

  118-165-75-57:cmake_debug_build Jonathan$ pwd
  /Users/Jonathan/llvm/test/cmake_debug_build
  118-165-75-57:cmake_debug_build Jonathan$ rm -rf lib/Target/Cpu0/*
  118-165-75-57:cmake_debug_build Jonathan$ cmake -DCMAKE_CXX_COMPILER=clang++ 
  -DCMAKE_C_COMPILER=clang -DCMAKE_BUILD_TYPE=Debug -G "Xcode" ../src/
  ...
  -- Targeting Cpu0
  ...
  -- Targeting XCore
  -- Configuring done
  -- Generating done
  -- Build files have been written to: /Users/Jonathan/llvm/test/cmake_debug_build

Now try to do ``llc`` command to compile input file ch3.cpp as follows,

.. rubric:: LLVMBackendTutorialExampleCode/InputFiles/ch3.cpp
.. literalinclude:: ../LLVMBackendTutorialExampleCode/InputFiles/ch3.cpp
    :lines: 8-
    :linenos:


First step, compile it with clang and get output ch3.bc as follows,

.. code-block:: bash

  [Gamma@localhost InputFiles]$ clang -c ch3.cpp -emit-llvm -o ch3.bc

Next step, transfer bitcode .bc to human readable text format as follows,

.. code-block:: bash

  118-165-78-230:test Jonathan$ llvm-dis ch3.bc -o ch3.ll 
  
  // ch3.ll
  ; ModuleID = 'ch3.bc' 
  target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f3
  2:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:6
  4-S128" 
  target triple = "x86_64-unknown-linux-gnu" 
  
  define i32 @main() nounwind uwtable { 
    %1 = alloca i32, align 4 
    store i32 0, i32* %1 
    ret i32 0 
  }

Now, compile ch3.bc into ch3.cpu0.s, we get the error message as follows,

.. code-block:: c++

  118-165-78-230:InputFiles Jonathan$ /Users/Jonathan/llvm/test/cmake_debug_build/
  bin/Debug/llc -march=cpu0 -relocation-model=pic -filetype=asm ch3.bc -o 
  ch3.cpu0.s
  Assertion failed: (target.get() && "Could not allocate target machine!"), 
  function main, file /Users/Jonathan/llvm/test/src/tools/llc/llc.cpp, 
  line 271.
  ...

Currently we just define target td files (Cpu0.td, Cpu0RegisterInfo.td, ...). 
According to LLVM structure, we need to define our target machine and include 
those td related files. 
The error message say we didn't define our target machine.


.. commenting out this subsection

	this subsection was originally between Stages of Cpu0 pipeline and LLVM Structure
	
	Replace ldi instruction by addiu instruction
	++++++++++++++++++++++++++++++++++++++++++++
	
	We have recognized the ldi instruction is a bad design and replace it with mips 
	instruction addiu. 
	The reason we replace ldi with addiu is that ldi use only one register even 
	though ldi is L type format and has two registers, as :ref:`llvmstructure_f4`. 
	Mips addiu which allow programmer to do load constant to register like ldi, 
	and add constant to a register. So, it's powerful and fully contains the ldi 
	ability. 
	These two instructions format as :ref:`llvmstructure_f4` and :ref:`llvmstructure_f5`.
	
	.. _llvmstructure_f4: 
	.. figure:: ../Fig/llvmstructure/4.png
		:align: center
	
		Cpu0 ldi instruction

	.. _llvmstructure_f5: 
	.. figure:: ../Fig/llvmstructure/5.png
		:align: center
	
		Mips addiu instruction format

	From :ref:`llvmstructure_f4` and :ref:`llvmstructure_f5`, you can find ldi $Ra, 
	5 can be replaced by addiu $Ra, $zero, 5. 
	And more, addiu can do addiu $Ra, $Rb, 5 which add $Rb and 5 then save to $Ra, 
	but ldi cannot. 
	As a cpu design, it's common to redesign CPU instruction when find a better 
	solution during design the compiler backend for that CPU. 
	So, we add addiu instruction to cpu0. 
	The cpu0 is my brother's work, I will find time to talk with him.
.. end subsection


.. [#cpu0-chinese] Original Cpu0 architecture and ISA details (Chinese). http://ccckmit.wikidot.com/ocs:cpu0

.. [#cpu0-english] English translation of Cpu0 description. http://translate.google.com.tw/translate?js=n&prev=_t&hl=zh-TW&ie=UTF-8&layout=2&eotf=1&sl=zh-CN&tl=en&u=http://ccckmit.wikidot.com/ocs:cpu0

.. [#cond-note] Conditions include the following comparisons: >, >=, ==, !=, <=, <. SW is actually set by the subtraction of the two register operands, and the flags indicate which conditions are present.

.. [#aosa-book] Chris Lattner, **LLVM**. Published in The Architecture of Open Source Applications. http://www.aosabook.org/en/llvm.html

.. [#tablegen] http://llvm.org/docs/TableGenFundamentals.html

.. [#cmake] http://llvm.org/docs/CMake.html

.. [#llvmbuild] http://llvm.org/docs/LLVMBuild.html

.. [#target-reg] http://llvm.org/docs/WritingAnLLVMBackend.html#target-registration

.. [#clang] http://clang.llvm.org/get_started.html

.. [#asadasd] http://jonathan2251.github.com/lbd/llvmstructure.html#target-registration
