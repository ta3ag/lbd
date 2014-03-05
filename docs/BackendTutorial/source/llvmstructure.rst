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

This chapter details the Cpu0 instruction set and the structure of LLVM. 
The LLVM structure information is adapted from Chris Lattner's LLVM chapter of the 
Architecture of Open Source Applications book [#aosa-book]_. You can read 
the original article from the AOSA website if you prefer. Finally, you will begin to 
create a new LLVM backend by writing register and instruction definitions in the 
Target Description files which will be used in next chapter.

Cpu0 Processor Architecture Details
-----------------------------------

This section is based on materials available here [#cpu0-chinese]_ (Chinese)
and [#cpu0-english]_ (English).

Brief introduction
++++++++++++++++++

Cpu0 is a 32-bit architecture. It has 16 general purpose registers (R0, ..., 
R15), the Instruction Register (IR), the memory access registers MAR & MDR, 
MULT or DIV result registers HI & LO and Status Word Register SW. Its structure 
is illustrated in :num:`Figure #llvmstructure-f1` below.

.. _llvmstructure-f1: 
.. figure:: ../Fig/llvmstructure/1.png
  :align: center

  Architectural block diagram of the Cpu0 processor


The registers are used for the following purposes:

.. table:: Cpu0 registers purposes

  ============  ===========
  Register      Description
  ============  ===========
  IR            Instruction register
  R0            Constant register, value is 0
  R1-R9         General-purpose registers
  R10           Status Word Register (SW)
  R11           Global Pointer register (GP)
  R12           Frame Pointer register (FP)
  R13           Stack Pointer register (SP)
  R14           Link Register (LR)
  R15           Program Counter (PC)
  MAR           Memory Address Register (MAR)
  MDR           Memory Data Register (MDR)
  HI            High part of MULT result
  LO            Low part of MULT result
  ============  ===========

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

The Cpu0 has two ISA, the first ISA-I is cpu032I which hired CMP instruction 
from ARM; the second ISA-II is cpu032II which hire SLT instruction from Mips. 
The cpu032II include all cpu032I instruction set and add SLT, BEQ, ..., 
instructions. The main purpose to add cpu032II is for instruction set design 
explanation. As you will see in later chapter, the SLT instruction will has 
better performance than CMP old style instruction.
The following table details the cpu032I instruction set:

- First column F\.: meaning Format.

.. list-table:: cpu032I Instruction Set
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
    - Ra <= (byte)[Rb+Cx] [#lb-note]_
  * - L
    - LBu
    - 04
    - Load byte unsigned
    - LBu Ra, [Rb+Cx]
    - Ra <= (byte)[Rb+Cx] [#lb-note]_
  * - L
    - SB
    - 05
    - Store byte
    - SB Ra, [Rb+Cx]
    - [Rb+Cx] <= (byte)Ra
  * - A
    - LH
    - 06
    - Load half word
    - LH Ra, [Rb+Cx]
    - Ra <= (2bytes)[Rb+Cx] [#lb-note]_
  * - A
    - LHu
    - 07
    - Load half word unsigned
    - LHu Ra, [Rb+Cx]
    - Ra <= (2bytes)[Rb+Cx] [#lb-note]_
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
    - CMP
    - 10
    - Compare
    - CMP Ra, Rb
    - SW <= (Ra cond Rb) [#cond-note]_
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
  * - J
    - JEQ
    - 30
    - Jump if equal (==)
    - JEQ Cx
    - if SW(==), PC <= PC + Cx
  * - J
    - JNE
    - 31
    - Jump if not equal (!=)
    - JNE Cx
    - if SW(!=), PC <= PC + Cx
  * - J
    - JLT
    - 32
    - Jump if less than (<)
    - JLT Cx
    - if SW(<), PC <= PC + Cx
  * - J
    - JGT
    - 33
    - Jump if greater than (>)
    - JGT Cx
    - if SW(>), PC <= PC + Cx
  * - J
    - JLE
    - 34
    - Jump if less than or equals (<=)
    - JLE Cx
    - if SW(<=), PC <= PC + Cx
  * - J
    - JGE
    - 35
    - Jump if greater than or equals (>=)
    - JGE Cx
    - if SW(>=), PC <= PC + Cx
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
    - Indirect jump
    - JALR Rb
    - LR <= PC; PC <= Rb [#call-note]_
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
    - Divide unsigned
    - DIVU Ra, Rb
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
  * - L
    - MFSW
    - 50
    - Move SW to GPR
    - MFSW Ra
    - Ra <= SW
  * - L
    - MTSW
    - 51
    - Move GPR to SW
    - MTSW Ra
    - SW <= Ra


The following table details the cpu032II instruction set added:

.. list-table:: cpu032II Instruction Set
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

.. note:: **Cpu0 unsigned instructions**

  Like Mips, the mathematic unsigned instructions except DIVU, such as ADDu and 
  SUBu, are no overflow exception instructions. 
  The ADDu and SUBu handle both signed and unsigned integers well. 
  For example, (ADDu 1, -2) is -1; (ADDu 0x01, 
  0xfffffffe) is 0xffffffff = (4G - 1). If you treat the result is negative then
  it is -1. Otherwise, if you treat the result is positive then it's (+4G - 1).
  

The Status Register
+++++++++++++++++++

The Cpu0 status word register (SW) contains the state of the Negative (N), Zero (Z), 
Carry (C), Overflow (V), Debug (D), Mode (M), and Interrupt (I) flags. 
The bit layout of the SW register is shown in :num:`Figure #llvmstructure-f3` below.

.. _llvmstructure-f3: 
.. figure:: ../Fig/llvmstructure/3.png
  :align: center

  Cpu0 status word (SW) register

When a CMP Ra, Rb instruction executes, the condition flags will change. For example:

- If Ra > Rb, then N = 0, Z = 0
- If Ra < Rb, then N = 1, Z = 0
- If Ra = Rb, then N = 0, Z = 1

The direction (i.e. taken/not taken) of the conditional jump instructions JGT, JLT, JGE, 
JLE, JEQ, JNE is determined by the N and Z flags in the SW register.

Cpu0's Stages of Instruction Execution
++++++++++++++++++++++++++++++++++++++

The Cpu0 architecture has a three-stage pipeline.  The stages are instruction fetch (IF), 
decode (D), and execute (EX), and they occur in that order.  Here is a description of 
what happens in the processor:

1) Instruction fetch

- The Cpu0 fetches the instruction pointed to by the Program Counter (PC) into the 
  Instruction Register (IR): IR = [PC].
- The PC is then updated to point to the next instruction: PC = PC + 4.

2) Decode

- The control unit decodes the instruction stored in IR, which routes necessary data
  stored in registers to the ALU, and sets the ALU's operation mode based on the 
  current instruction's opcode.

3) Execute

- The ALU executes the operation designated by the control unit upon data in registers. 
  After the ALU is done, the result is stored in the destination register. 

Cpu0's Interrupt Vector
++++++++++++++++++++++++

.. table:: Cpu0's Interrupt Vector

  ========  ===========
  Address   type
  ========  ===========
  0x00      Reset
  0x04      Error Handle
  0x08      Interrupt
  ========  ===========


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
find in the optimizer chapter of a compiler. 
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
target description file is called Cpu0Other.td, which is shown below:

.. rubric:: lbdex/Chapter2/Cpu0Other.td
.. literalinclude:: ../../../lib/Target/Cpu0/Cpu0Other.td

.. rubric:: lbdex/Chapter2/Cpu0.td
.. literalinclude:: ../../../lib/Target/Cpu0/Cpu0.td
    :end-before: include "Cpu0CallingConv.td"
.. literalinclude:: ../../../lib/Target/Cpu0/Cpu0.td
    :start-after: include "Cpu0CallingConv.td"
    :end-before: // Without this will have error: 'cpu032I'
.. literalinclude:: ../../../lib/Target/Cpu0/Cpu0.td
    :start-after: // const char *Cpu0InstPrinter::getRegisterName(unsigned RegNo) {...}
    :end-before: let AssemblyParsers = [Cpu0AsmParser];
.. literalinclude:: ../../../lib/Target/Cpu0/Cpu0.td
    :start-after: let AssemblyParserVariants = [Cpu0AsmParserVariant];

Cpu0Other.td and Cpu0.td includes a few other .td files. 
Cpu0RegisterInfo.td (shown below) describes the 
Cpu0's set of registers.  In this file, we see that registers have been given names, i.e.
``def PC`` indicates that there is a register called PC.  Also, there is a register class 
named ``CPURegs`` that contains all of the other registers.  You may have multiple 
register classes (see the X86 backend, for example) which can help you if certain 
instructions can only write to specific registers.  In this case, there is only one set 
of general purpose registers for Cpu0, and some registers that are reserved so that they 
are not modified by instructions during execution.

.. rubric:: lbdex/Chapter2/Cpu0RegisterInfoGPROutForOther.td
.. literalinclude:: ../../../lib/Target/Cpu0/Cpu0RegisterInfoGPROutForOther.td

.. rubric:: lbdex/Chapter2/Cpu0RegisterInfo.td
.. literalinclude:: ../../../lib/Target/Cpu0/Cpu0RegisterInfo.td
    :end-before: // Hi/Lo registers number and name
.. literalinclude:: ../../../lib/Target/Cpu0/Cpu0RegisterInfo.td
    :start-after: def LO   : Register<"lo">, DwarfRegNum<[19]>;
    :end-before: // Hi/Lo Registers class
.. literalinclude:: ../../../lib/Target/Cpu0/Cpu0RegisterInfo.td
    :start-after: def HILO : RegisterClass<"Cpu0", [i32], 32, (add HI, LO)>;


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

The Cpu0 instructions td is named to Cpu0InstrInfo.td which contents as follows,

.. rubric:: lbdex/Chapter2/Cpu0InstrInfo.td
.. literalinclude:: ../../../lib/Target/Cpu0/Cpu0InstrInfo.td
    :end-before: def SDT_Cpu0DivRem       : SDTypeProfile<0, 2
.. literalinclude:: ../../../lib/Target/Cpu0/Cpu0InstrInfo.td
    :start-after: def Cpu0GPRel : SDNode<"Cpu0ISD::GPRel", SDTIntUnaryOp>;
    :end-before: // These are target-independent nodes, but have target-specific formats.
.. literalinclude:: ../../../lib/Target/Cpu0/Cpu0InstrInfo.td
    :start-after: def RelocPIC    :     Predicate<"TM.getRelocationModel() == Reloc::PIC_">;
    :end-before: // Instruction operand types
.. literalinclude:: ../../../lib/Target/Cpu0/Cpu0InstrInfo.td
    :start-after: // lbd document - mark - def calltarget
    :end-before: def shamt
.. literalinclude:: ../../../lib/Target/Cpu0/Cpu0InstrInfo.td
    :start-after: // lbd document - mark - def Cpu0MemAsmOperand
    :end-before: let ParserMatchClass = Cpu0MemAsmOperand;
.. literalinclude:: ../../../lib/Target/Cpu0/Cpu0InstrInfo.td
    :start-after: let ParserMatchClass = Cpu0MemAsmOperand;
    :end-before: def mem_ea : Operand<i32> {
.. literalinclude:: ../../../lib/Target/Cpu0/Cpu0InstrInfo.td
    :start-after: // lbd document - mark - def HI16
    :end-before: // Node immediate fits as 16-bit zero extended on target immediate.
.. literalinclude:: ../../../lib/Target/Cpu0/Cpu0InstrInfo.td
    :start-after: def immZExt5 : ImmLeaf<i32, [{return Imm == (Imm & 0x1f);}]>;
    :end-before: def sextloadi16_a   : AlignedLoad<sextloadi16>;
.. literalinclude:: ../../../lib/Target/Cpu0/Cpu0InstrInfo.td
    :start-after: def extloadi16_a    : AlignedLoad<extloadi16>;
    :end-before: def truncstorei16_a : AlignedStore<truncstorei16>;
.. literalinclude:: ../../../lib/Target/Cpu0/Cpu0InstrInfo.td
    :start-after: def truncstorei16_a : AlignedStore<truncstorei16>;
    :end-before: // Instructions specific format
.. literalinclude:: ../../../lib/Target/Cpu0/Cpu0InstrInfo.td
    :start-after: def store_a         : AlignedStore<store>;
    :end-before: // Arithmetic and logical instructions with 3 register operands.
.. literalinclude:: ../../../lib/Target/Cpu0/Cpu0InstrInfo.td
    :start-after: // lbd document - mark - class CmpInstr
    :end-before: // Shifts
.. literalinclude:: ../../../lib/Target/Cpu0/Cpu0InstrInfo.td
    :start-after: // lbd document - mark - class LoadUpper
    :end-before: // Conditional Branch, e.g. JEQ brtarget24
.. literalinclude:: ../../../lib/Target/Cpu0/Cpu0InstrInfo.td
    :start-after: // lbd document - mark - class UncondBranch
    :end-before: // Jump and Link (Call)
.. literalinclude:: ../../../lib/Target/Cpu0/Cpu0InstrInfo.td
    :start-after: ".cprestore\t$loc", []>;
    :end-before: // Pseudo Instruction definition
.. literalinclude:: ../../../lib/Target/Cpu0/Cpu0InstrInfo.td
    :start-after: def LoadAddr32Imm : LoadAddressImm<"la", shamt, GPROut>;
    :end-before: defm LB     : LoadM32<0x03, "lb",  sextloadi8>;
.. literalinclude:: ../../../lib/Target/Cpu0/Cpu0InstrInfo.td
    :start-after: defm SH     : StoreM32<0x08, "sh", truncstorei16_a>;
    :end-before: def ANDi    : ArithLogicI<0x0c, "andi", and, uimm16, immZExt16, CPURegs>;
.. literalinclude:: ../../../lib/Target/Cpu0/Cpu0InstrInfo.td
    :start-after: def RetLR : Cpu0Pseudo<(outs), (ins), "", [(Cpu0Ret)]>;
    :end-before: def IRET    : JumpFR<0x3d, "iret", GPROut>;
.. literalinclude:: ../../../lib/Target/Cpu0/Cpu0InstrInfo.td
    :start-after: // lbd document - mark - def LEA_ADDiu
    :end-before: def : Pat<(i32 immZExt16:$in),

The Cpu0InstrFormats.td is included by Cpu0InstInfo.td as follows,

.. rubric:: lbdex/Chapter2/Cpu0InstrFormats.td
.. literalinclude:: ../../../lib/Target/Cpu0/Cpu0InstrFormats.td
    :end-before: // Cpu0 Pseudo Instructions Format
.. literalinclude:: ../../../lib/Target/Cpu0/Cpu0InstrFormats.td
    :start-after: // lbd document - mark - class Cpu0AsmPseudoInst


ADDiu is class ArithLogicI inherited from FL, can be expanded and get member 
value as follows,

.. code-block:: c++

  def ADDiu   : ArithLogicI<0x09, "addiu", add, simm16, immSExt16, CPURegs>;
  
  /// Arithmetic and logical instructions with 2 register operands.
  class ArithLogicI<bits<8> op, string instr_asm, SDNode OpNode,
            Operand Od, PatLeaf imm_type, RegisterClass RC> :
    FL<op, (outs GPROut:$ra), (ins RC:$rb, Od:$imm16),
     !strconcat(instr_asm, "\t$ra, $rb, $imm16"),
     [(set GPROut:$ra, (OpNode RC:$rb, imm_type:$imm16))], IIAlu> {
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

   :  FL<op, (outs GPROut:$ra), (ins RC:$rb, Od:$imm16),
     !strconcat(instr_asm, "\t$ra, $rb, $imm16"), 
     [(set GPROut:$ra, (OpNode RC:$rb, imm_type:$imm16))], IIAlu>
  
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

outs = GPROut:$ra

ins = CPURegs:$rb,simm16:$imm16

asmstr = "addiu\t$ra, $rb, $imm16"

pattern = [(set GPROut:$ra, (add RC:$rb, immSExt16:$imm16))]

itin = IIAlu

Members are,

ra = GPROut:$ra

rb = CPURegs:$rb

imm16 = simm16:$imm16

Opcode = 0x09;

Inst{23-20} = GPROut:$ra; 

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

pattern = [(set GPROut:$ra, (add RC:$rb, immSExt16:$imm16))]

itin = IIAlu

f =  FrmL

Members are,

Inst{31-24} = 0x09; 

OutOperandList = GPROut:$ra 

InOperandList  = CPURegs:$rb,simm16:$imm16

AsmString = "addiu\t$ra, $rb, $imm16"

Pattern = [(set GPROut:$ra, (add RC:$rb, immSExt16:$imm16))]

Itinerary = IIAlu
  
Summary with all members are, 

// Inherited from parent like Instruction

Namespace = "Cpu0";

DecoderNamespace = "Cpu0";

Inst{31-24} = 0x08; 

Inst{23-20} = GPROut:$ra; 

Inst{19-16} = CPURegs:$rb; 

Inst{15-0}  = simm16:$imm16; 

OutOperandList = CPURegs:$ra 

InOperandList  = CPURegs:$rb,simm16:$imm16

AsmString = "addiu\t$ra, $rb, $imm16"

Pattern = [(set GPROut:$ra, (add RC:$rb, immSExt16:$imm16))]

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
[(set GPROut:$ra, (add RC:$rb, immSExt16:$imm16))] which include keyword 
**“add”**. 
We will use it in DAG transformations later. 

File Cpu0Schedule.td include the function units and pipeline stages information
as follows,

.. rubric:: lbdex/Chapter2/Cpu0Schedule.td
.. literalinclude:: ../../../lib/Target/Cpu0/Cpu0Schedule.td
    :end-before: def IMULDIV : FuncUnit;
.. literalinclude:: ../../../lib/Target/Cpu0/Cpu0Schedule.td
    :start-after: def IMULDIV : FuncUnit;
    :end-before: def IIHiLo
.. literalinclude:: ../../../lib/Target/Cpu0/Cpu0Schedule.td
    :start-after: def IIIdiv
    :end-before: InstrItinData<IIHiLo
.. literalinclude:: ../../../lib/Target/Cpu0/Cpu0Schedule.td
    :start-after: InstrItinData<IIIdiv


Write cmake file
----------------

Target/Cpu0 directory has two files CMakeLists.txt and LLVMBuild.txt, 
contents as follows,

.. rubric:: lbdex/Chapter2/CMakeLists.txt
.. literalinclude:: ../../../lib/Target/Cpu0/CMakeLists.txt
    :end-before: tablegen(LLVM Cpu0GenDisassemblerTables.inc -gen-disassembler)
.. literalinclude:: ../../../lib/Target/Cpu0/CMakeLists.txt
    :start-after: tablegen(LLVM Cpu0GenCallingConv.inc -gen-callingconv)
    :end-before: tablegen(LLVM Cpu0GenAsmMatcher.inc -gen-asm-matcher)
.. literalinclude:: ../../../lib/Target/Cpu0/CMakeLists.txt
    :start-after: tablegen(LLVM Cpu0GenAsmMatcher.inc -gen-asm-matcher)
    :end-before: Cpu0AnalyzeImmediate.cpp
.. literalinclude:: ../../../lib/Target/Cpu0/CMakeLists.txt
    :start-after: Cpu0Subtarget.cpp
    :end-before: Cpu0TargetObjectFile.cpp
.. literalinclude:: ../../../lib/Target/Cpu0/CMakeLists.txt
    :start-after: Cpu0SelectionDAGInfo.cpp
    :end-before: add_subdirectory(InstPrinter)
.. literalinclude:: ../../../lib/Target/Cpu0/CMakeLists.txt
    :start-after: add_subdirectory(Disassembler)
    :end-before: add_subdirectory(AsmParser)


.. rubric:: lbdex/Chapter2/LLVMBuild.txt
.. literalinclude:: ../../../lib/Target/Cpu0/LLVMBuild.txt
    :end-before: AsmParser
.. literalinclude:: ../../../lib/Target/Cpu0/LLVMBuild.txt
    :start-after: InstPrinter
    :end-before: # Whether this target defines an assembly parser, 
.. literalinclude:: ../../../lib/Target/Cpu0/LLVMBuild.txt
    :start-after: has_disassembler = 1
    :end-before: AsmPrinter
.. literalinclude:: ../../../lib/Target/Cpu0/LLVMBuild.txt
    :start-after: AsmPrinter
    :end-before: Cpu0AsmPrinter
.. literalinclude:: ../../../lib/Target/Cpu0/LLVMBuild.txt
    :start-after: Cpu0AsmPrinter


CMakeLists.txt is the make information for cmake and # is comment.
LLVMBuild.txt files are written in a simple variant of the INI or configuration 
file format. 
Comments are prefixed by ``#`` in both files. 
We explain the setting for these 2 files in comments. 
Please spend a little time to read it. 
This book has broken the backend function code, add code chapter by chapter and 
even in section by section.
Don't try to understand everything in the text of book, the code added in each 
chapter is a reading material too. 
To understand the computer related knowledge in concept can ignore source code 
but implementation based on an existed open software cannot. 
In programming, documentation cannot replace the source code totally. 
Reading source code is a big opportunity in the open source development. 

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

.. rubric:: lbdex/Chapter2/Cpu0.h
.. literalinclude:: ../../../lib/Target/Cpu0/Cpu0.h
    :end-before: FunctionPass *createCpu0EmitGPRestorePass(Cpu0TargetMachine &TM);
.. literalinclude:: ../../../lib/Target/Cpu0/Cpu0.h
    :start-after: FunctionPass *createCpu0DelJmpPass(Cpu0TargetMachine &TM);

.. rubric:: lbdex/Chapter2/TargetInfo/Cpu0TargetInfo.cpp
.. literalinclude:: ../../../lib/Target/Cpu0/TargetInfo/Cpu0TargetInfo.cpp

.. rubric:: lbdex/Chapter2/TargetInfo/CMakeLists.txt
.. literalinclude:: ../../../lib/Target/Cpu0/TargetInfo/CMakeLists.txt

.. rubric:: lbdex/Chapter2/TargetInfo/LLVMBuild.txt
.. literalinclude:: ../../../lib/Target/Cpu0/TargetInfo/LLVMBuild.txt

Files Cpu0TargetMachine.cpp and MCTargetDesc/Cpu0MCTargetDesc.cpp just define 
the empty initialize function since we register nothing in them for this moment.

.. rubric:: lbdex/Chapter2/Cpu0TargetMachine.cpp
.. literalinclude:: ../../../lib/Target/Cpu0/Cpu0TargetMachine.cpp
    :end-before: #include "Cpu0TargetMachine.h"
.. literalinclude:: ../../../lib/Target/Cpu0/Cpu0TargetMachine.cpp
    :start-after: #include "llvm/CodeGen/Passes.h"
    :end-before: // Register the target.
.. literalinclude:: ../../../lib/Target/Cpu0/Cpu0TargetMachine.cpp
    :start-after: RegisterTargetMachine<Cpu0elTargetMachine> Y(TheCpu0elTarget);
    :end-before: // DataLayout --> Big-endian, 32-bit pointer/ABI/alignment

.. rubric:: lbdex/Chapter2/MCTargetDesc/Cpu0MCTargetDesc.h
.. literalinclude:: ../../../lib/Target/Cpu0/MCTargetDesc/Cpu0MCTargetDesc.h
    :end-before: #include "llvm/Support/DataTypes.h"
.. literalinclude:: ../../../lib/Target/Cpu0/MCTargetDesc/Cpu0MCTargetDesc.h
    :start-after: #include "llvm/Support/DataTypes.h"
    :end-before: class MCAsmBackend;
.. literalinclude:: ../../../lib/Target/Cpu0/MCTargetDesc/Cpu0MCTargetDesc.h
    :start-after: class StringRef;
    :end-before: class raw_ostream;
.. literalinclude:: ../../../lib/Target/Cpu0/MCTargetDesc/Cpu0MCTargetDesc.h
    :start-after: class raw_ostream;
    :end-before: MCCodeEmitter *createCpu0MCCodeEmitterEB
.. literalinclude:: ../../../lib/Target/Cpu0/MCTargetDesc/Cpu0MCTargetDesc.h
    :start-after: bool IsLittleEndian);

.. rubric:: lbdex/Chapter2/MCTargetDesc/Cpu0MCTargetDesc.cpp
.. literalinclude:: ../../../lib/Target/Cpu0/MCTargetDesc/Cpu0MCTargetDesc.cpp
    :end-before: #include "Cpu0MCAsmInfo.h"
.. literalinclude:: ../../../lib/Target/Cpu0/MCTargetDesc/Cpu0MCTargetDesc.cpp
    :start-after: #include "Cpu0MCAsmInfo.h"
    :end-before: #include "InstPrinter/Cpu0InstPrinter.h"
.. literalinclude:: ../../../lib/Target/Cpu0/MCTargetDesc/Cpu0MCTargetDesc.cpp
    :start-after: #include "InstPrinter/Cpu0InstPrinter.h"
    :end-before: static std::string ParseCpu0Triple
.. literalinclude:: ../../../lib/Target/Cpu0/MCTargetDesc/Cpu0MCTargetDesc.cpp
    :start-after: // lbd document - mark - createMCStreamer
    :end-before: // Register the MC asm info.
.. literalinclude:: ../../../lib/Target/Cpu0/MCTargetDesc/Cpu0MCTargetDesc.cpp
    :start-after: // lbd document - mark - RegisterMCInstPrinter


.. rubric:: lbdex/Chapter2/MCTargetDesc/CMakeLists.txt
.. literalinclude:: ../../../lib/Target/Cpu0/MCTargetDesc/CMakeLists.txt
    :end-before: Cpu0AsmBackend.cpp
.. literalinclude:: ../../../lib/Target/Cpu0/MCTargetDesc/CMakeLists.txt
    :start-after: Cpu0MCCodeEmitter.cpp
    :end-before: Cpu0ELFObjectWriter.cpp
.. literalinclude:: ../../../lib/Target/Cpu0/MCTargetDesc/CMakeLists.txt
    :start-after: Cpu0ELFObjectWriter.cpp

.. rubric:: lbdex/Chapter2/MCTargetDesc/LLVMBuild.txt
.. literalinclude:: ../../../lib/Target/Cpu0/MCTargetDesc/LLVMBuild.txt
    :end-before: Cpu0AsmPrinter
.. literalinclude:: ../../../lib/Target/Cpu0/MCTargetDesc/LLVMBuild.txt
    :start-after: Cpu0AsmPrinter


Please see "Target Registration" [#target-reg]_ for reference.


Build libraries and td
----------------------

The llvm source code is put in /Users/Jonathan/llvm/release/src and have llvm 
release-build in /Users/Jonathan/llvm/release/cmake_release_build. 
About how to build llvm, please refer [#clang]_. 
In appendix A, we made a copy from /Users/Jonathan/llvm/release/src to 
/Users/Jonathan/llvm/test/src for working with my Cpu0 target back end.
Sub-directories src is for source code and cmake_debug_build is for debug 
build directory.

Except directory src/lib/Target/Cpu0, there are a couple of files modified to 
support cpu0 new Target. 
Please check files in src_files_modify/src_files_modified/src/. 

You can update your llvm working copy and find the modified files by 
command,

.. code-block:: bash

  cp -rf lbdex/src_files_modified/src_files_modified/src/* 
  yourllvm/workingcopy/sourcedir/.
  
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

  118-165-78-230:cmake_debug_build Jonathan$ cmake -DCMAKE_CXX_COMPILER=clang++ 
  -DCMAKE_C_COMPILER=clang -DCMAKE_BUILD_TYPE=Debug  -G "Unix Makefiles" ../src/
  
  -- Targeting Cpu0 
  ...
  -- Targeting XCore 
  -- Configuring done 
  -- Generating done 
  -- Build files have been written to: /Users/Jonathan/llvm/test/cmake_debug
  _build 
  
  118-165-78-230:cmake_debug_build Jonathan$

After build, you can type command ``llc –version`` to find the cpu0 backend,

.. code-block:: bash

  118-165-78-230:cmake_debug_build Jonathan$ /Users/Jonathan/llvm/test/
  cmake_debug_build/bin/Debug/llc --version
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
the code from file TargetInfo/Cpu0TargetInfo.cpp what in 
"section Target Registration" [#asadasd]_ we made. 

Let's build lbdex/Chapter2 code as follows,

.. code-block:: bash

  118-165-75-57:lbdex Jonathan$ pwd
  /Users/Jonathan/llvm/test/src/lib/Target/Cpu0/lbdex
  118-165-75-57:lbdex Jonathan$ sh removecpu0.sh 
  118-165-75-57:lbdex Jonathan$ cp -rf lbdex/Chapter2/
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

Now try to run  command ``llc`` to compile input file ch3.cpp as follows,

.. rubric:: lbdex/InputFiles/ch3.cpp
.. literalinclude:: ../lbdex/InputFiles/ch3.cpp
    :start-after: /// start


First step, compile it with clang and get output ch3.bc as follows,

.. code-block:: bash

  118-165-78-230:InputFiles Jonathan$ pwd
  /Users/Jonathan/llvm/test/src/lib/Target/Cpu0/lbdex/InputFiles
  118-165-78-230:InputFiles Jonathan$ clang -target mips-unknown-linux-gnu -c 
  ch3.cpp -emit-llvm -o ch3.bc

As above, compile C to .bc by ``clang -target mips-unknown-linux-gnu`` because
Cpu0 borrow the ABI from Mips.
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
  ...
  ... Assertion `target.get() && "Could not allocate target machine!"' failed 
  ...

At this point, we finish the Target Registration for Cpu0 backend. 
The backend compiler command llc can recognize cpu0 backend now. 
Currently we just define target td files (Cpu0.td, Cpu0Other.td, 
Cpu0RegisterInfo.td, ...). 
According to LLVM structure, we need to define our target machine and include 
those td related files. 
The error message say we didn't define our target machine.
This book is a step-by-step backend delvelopment. 
You can review the houndreds lines of Chapter2 example code to see how to do 
the Target Registration. 


.. [#cpu0-chinese] Original Cpu0 architecture and ISA details (Chinese). http://ccckmit.wikidot.com/ocs:cpu0

.. [#cpu0-english] English translation of Cpu0 description. http://translate.google.com.tw/translate?js=n&prev=_t&hl=zh-TW&ie=UTF-8&layout=2&eotf=1&sl=zh-CN&tl=en&u=http://ccckmit.wikidot.com/ocs:cpu0

.. [#lb-note] The difference between LB and LBu is signed and unsigned byte value expand to a word size. For example, After LB Ra, [Rb+Cx], Ra is 0xffffff80(= -128) if byte [Rb+Cx] is 0x80; Ra is 0x0000007f(= 127) if byte [Rb+Cx] is 0x7f. After LBu Ra, [Rb+Cx], Ra is 0x00000080(= 128) if byte [Rb+Cx] is 0x80; Ra is 0x0000007f(= 127) if byte [Rb+Cx] is 0x7f. Difference between LH and LHu is similar.

.. [#cond-note] Conditions include the following comparisons: >, >=, ==, !=, <=, <. SW is actually set by the subtraction of the two register operands, and the flags indicate which conditions are present.

.. [#sra-note] Rb '>> Cx, Rb '>> Rc: Shift with signed bit remain. It's equal to ((Rb&'h80000000)|Rb>>Cx) or ((Rb&'h80000000)|Rb>>Rc).

.. [#call-note] jsub cx is direct call for 24 bits value of cx while jalr $rb is indirect call for 32 bits value of register $rb.

.. [#aosa-book] Chris Lattner, **LLVM**. Published in The Architecture of Open Source Applications. http://www.aosabook.org/en/llvm.html

.. [#tablegen] http://llvm.org/docs/TableGenFundamentals.html

.. [#cmake] http://llvm.org/docs/CMake.html

.. [#llvmbuild] http://llvm.org/docs/LLVMBuild.html

.. [#target-reg] http://llvm.org/docs/WritingAnLLVMBackend.html#target-registration

.. [#clang] http://clang.llvm.org/get_started.html

.. [#asadasd] http://jonathan2251.github.io/lbd/llvmstructure.html#target-registration
