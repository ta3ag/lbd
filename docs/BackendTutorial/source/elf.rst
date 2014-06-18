.. _sec-elf:

ELF Support
===========

Cpu0 backend generated the ELF format of obj. 
The ELF (Executable and Linkable Format) is a common standard file format for 
executables, object code, shared libraries and core dumps. 
First published in the System V Application Binary Interface specification, 
and later in the Tool Interface Standard, it was quickly accepted among 
different vendors of Unixsystems. 
In 1999 it was chosen as the standard binary file format for Unix and 
Unix-like systems on x86 by the x86open project. 
Please reference [#]_.

The binary encode of Cpu0 instruction set in obj has been checked in the 
previous chapters. 
But we didn't dig into the ELF file format like elf header and relocation 
record at that time. 
This chapter will use the binutils which has been installed in 
"sub-section Install other tools on iMac" of Appendix A: “Installing LLVM” 
[#]_ to analysis cpu0 ELF file. 
You will learn the objdump, readelf, ..., tools and understand the ELF file 
format itself through using these tools to analyze the cpu0 generated obj in 
this chapter. 
LLVM has the llvm-objdump tool which like objdump. We will make cpu0 support 
llvm-objdump tool further in this chapter. 
The binutils is a cross compiler tool chains include a couple of CPU ELF dump 
function support. 
Linux platform has binutils already and no need to install it further.
We use Linux binutils in this chapter just because iMac will display Chinese 
text. 
The iMac corresponding binutils have no problem except it add g in command name. 
For example, use gobjdump instead of objdump, and display with your area 
language instead of pure English on iMac.

The binutils tool we use is not a part of llvm tools, but it's a powerful tool 
in ELF analysis. 
This chapter introduce the tool to readers since we think it is a valuable 
knowledge in this popular ELF format and the ELF binutils analysis tool. 
An LLVM compiler engineer has the responsibility to analyze the ELF since 
the obj is need to be handled by linker or loader later. 
With this tool, you can verify your generated ELF format.
 
The cpu0 author has published a “System Software” book which introduce the 
topics 
of assembler, linker, loader, compiler and OS in concept, and at same time 
demonstrates how to use binutils and gcc to analysis ELF through the example 
code in his book. 
It's a Chinese book of “System Software” in concept and practice. 
This book does the real analysis through binutils. 
The “System Software” [#]_ written by Beck is a famous book in concept of 
telling readers what is the compiler output, what is the linker output, 
what is the loader output, and how they work together. 
But it covers the concept only. 
You can reference it to understand how the **“Relocation Record”** works if you 
need to refresh or learning this knowledge for this chapter.

[#]_, [#]_, [#]_ are the Chinese documents available from the cpu0 author on 
web site.


ELF format
-----------

ELF is a format used both in obj and executable file. 
So, there are two views in it as :num:`Figure #elf-f1`.

.. _elf-f1:
.. figure:: ../Fig/elf/1.png
    :height: 320 px
    :width: 213 px
    :scale: 100 %
    :align: center

    ELF file format overview

As :num:`Figure #elf-f1`, the “Section header table” include sections .text, .rodata, 
..., .data which are sections layout for code, read only data, ..., and 
read/write data. 
“Program header table” include segments for run time code and data. 
The definition of segments is run time layout for code and data while sections 
is link time layout for code and data.

ELF header and Section header table
------------------------------------

Let's run Chapter9_4/ with ch6_1.cpp, and dump ELF header information by 
``readelf -h`` to see what information the ELF header contains.

.. code-block:: bash

  [Gamma@localhost InputFiles]$ /usr/local/llvm/test/cmake_debug_build/
  bin/llc -march=cpu0 -relocation-model=pic -filetype=obj ch6_1.bc -o ch6_1.cpu0.o
  
  [Gamma@localhost InputFiles]$ readelf -h ch6_1.cpu0.o 
    Magic:   7f 45 4c 46 01 02 01 03 00 00 00 00 00 00 00 00 
    Class:                             ELF32
    Data:                              2's complement, big endian
    Version:                           1 (current)
    OS/ABI:                            UNIX - GNU
    ABI Version:                       0
    Type:                              REL (Relocatable file)
    Machine:                           <unknown>: 0xc9
    Version:                           0x1
    Entry point address:               0x0
    Start of program headers:          0 (bytes into file)
    Start of section headers:          176 (bytes into file)
    Flags:                             0x0
    Size of this header:               52 (bytes)
    Size of program headers:           0 (bytes)
    Number of program headers:         0
    Size of section headers:           40 (bytes)
    Number of section headers:         8
    Section header string table index: 5
  [Gamma@localhost InputFiles]$ 

  [Gamma@localhost InputFiles]$ /usr/local/llvm/test/cmake_debug_build/
  bin/llc -march=mips -relocation-model=pic -filetype=obj ch6_1.bc -o ch6_1.mips.o
  
  [Gamma@localhost InputFiles]$ readelf -h ch6_1.mips.o 
  ELF Header:
    Magic:   7f 45 4c 46 01 02 01 03 00 00 00 00 00 00 00 00 
    Class:                             ELF32
    Data:                              2's complement, big endian
    Version:                           1 (current)
    OS/ABI:                            UNIX - GNU
    ABI Version:                       0
    Type:                              REL (Relocatable file)
    Machine:                           MIPS R3000
    Version:                           0x1
    Entry point address:               0x0
    Start of program headers:          0 (bytes into file)
    Start of section headers:          200 (bytes into file)
    Flags:                             0x50001007, noreorder, pic, cpic, o32, mips32
    Size of this header:               52 (bytes)
    Size of program headers:           0 (bytes)
    Number of program headers:         0
    Size of section headers:           40 (bytes)
    Number of section headers:         9
    Section header string table index: 6
  [Gamma@localhost InputFiles]$ 


As above ELF header display, it contains information of magic number, version, 
ABI, ..., . The Machine field of cpu0 is unknown while mips is MIPSR3000. 
It is because cpu0 is not a popular CPU recognized by utility readelf. 
Let's check ELF segments information as follows,

.. code-block:: bash

  [Gamma@localhost InputFiles]$ readelf -l ch6_1.cpu0.o 
  
  There are no program headers in this file.
  [Gamma@localhost InputFiles]$ 


The result is in expectation because cpu0 obj is for link only, not for 
execution. 
So, the segments is empty. 
Check ELF sections information as follows. 
Every section contain offset and size information.

.. code-block:: bash

  [Gamma@localhost InputFiles]$ readelf -S ch6_1.cpu0.o 
  There are 10 section headers, starting at offset 0xd4:
  
  Section Headers:
    [Nr] Name              Type            Addr     Off    Size   ES Flg Lk Inf Al
    [ 0]                   NULL            00000000 000000 000000 00      0   0  0
    [ 1] .text             PROGBITS        00000000 000034 000034 00  AX  0   0  4
    [ 2] .rel.text         REL             00000000 000310 000018 08      8   1  4
    [ 3] .data             PROGBITS        00000000 000068 000004 00  WA  0   0  4
    [ 4] .bss              NOBITS          00000000 00006c 000000 00  WA  0   0  4
    [ 5] .eh_frame         PROGBITS        00000000 00006c 000028 00   A  0   0  4
    [ 6] .rel.eh_frame     REL             00000000 000328 000008 08      8   5  4
    [ 7] .shstrtab         STRTAB          00000000 000094 00003e 00      0   0  1
    [ 8] .symtab           SYMTAB          00000000 000264 000090 10      9   6  4
    [ 9] .strtab           STRTAB          00000000 0002f4 00001b 00      0   0  1
  Key to Flags:
    W (write), A (alloc), X (execute), M (merge), S (strings)
    I (info), L (link order), G (group), T (TLS), E (exclude), x (unknown)
    O (extra OS processing required) o (OS specific), p (processor specific)
  [Gamma@localhost InputFiles]$ 



Relocation Record
------------------

The cpu0 backend translate global variable as follows,

.. code-block:: bash

  [Gamma@localhost InputFiles]$ clang -target mips-unknown-linux-gnu -c ch6_1.cpp 
  -emit-llvm -o ch6_1.bc
  [Gamma@localhost InputFiles]$ /usr/local/llvm/test/cmake_debug_build/
  bin/llc -march=cpu0 -relocation-model=pic -filetype=asm ch6_1.bc -o ch6_1.cpu0.s
  [Gamma@localhost InputFiles]$ cat ch6_1.cpu0.s 
    .section .mdebug.abi32
    .previous
    .file "ch6_1.bc"
    .text
    ...
    .cfi_startproc
    .frame  $sp,8,$lr
    .mask   0x00000000,0
    .set  noreorder
    .cpload $t9
    ...
    lui $2, %got_hi(gI)
    addu $2, $2, $gp
    ld $2, %got_lo(gI)($2)
    ...
    .type gI,@object              # @gI
    .data
    .globl  gI
    .align  2
  gI:
    .4byte  100                     # 0x64
    .size gI, 4
  
  
  [Gamma@localhost InputFiles]$ /usr/local/llvm/test/cmake_debug_build/
  bin/llc -march=cpu0 -relocation-model=pic -filetype=obj ch6_1.bc -o ch6_1.cpu0.o
  [Gamma@localhost InputFiles]$ objdump -s ch6_1.cpu0.o
  
  ch6_1.cpu0.o:     file format elf32-big
  
  Contents of section .text:
  // .cpload machine instruction
   0000 0fa00000 09aa0000 13aa6000 ........  ..............`.
   ...
   0020 002a0000 00220000 012d0000 09dd0008  .*..."...-......
   ...
  [Gamma@localhost InputFiles]$ Jonathan$ 
  
  [Gamma@localhost InputFiles]$ readelf -tr ch6_1.cpu0.o 
  There are 8 section headers, starting at offset 0xb0:

  Section Headers:
    [Nr] Name
         Type            Addr     Off    Size   ES   Lk Inf Al
         Flags
    [ 0] 
         NULL            00000000 000000 000000 00   0   0  0
         [00000000]: 
    [ 1] .text
         PROGBITS        00000000 000034 000044 00   0   0  4
         [00000006]: ALLOC, EXEC
    [ 2] .rel.text
         REL             00000000 0002a8 000020 08   6   1  4
         [00000000]: 
    [ 3] .data
         PROGBITS        00000000 000078 000008 00   0   0  4
         [00000003]: WRITE, ALLOC
    [ 4] .bss
         NOBITS          00000000 000080 000000 00   0   0  4
         [00000003]: WRITE, ALLOC
    [ 5] .shstrtab
         STRTAB          00000000 000080 000030 00   0   0  1
         [00000000]: 
    [ 6] .symtab
         SYMTAB          00000000 0001f0 000090 10   7   5  4
         [00000000]: 
    [ 7] .strtab
         STRTAB          00000000 000280 000025 00   0   0  1
         [00000000]: 

  Relocation section '.rel.text' at offset 0x2a8 contains 4 entries:
   Offset     Info    Type            Sym.Value  Sym. Name
  00000000  00000805 unrecognized: 5       00000000   _gp_disp
  00000004  00000806 unrecognized: 6       00000000   _gp_disp
  00000020  00000616 unrecognized: 16      00000004   gI
  00000028  00000617 unrecognized: 17      00000004   gI

  
  [Gamma@localhost InputFiles]$ readelf -tr ch6_1.mips.o 
  There are 9 section headers, starting at offset 0xc8:

  Section Headers:
    [Nr] Name
         Type            Addr     Off    Size   ES   Lk Inf Al
         Flags
    [ 0] 
         NULL            00000000 000000 000000 00   0   0  0
         [00000000]: 
    [ 1] .text
         PROGBITS        00000000 000034 000038 00   0   0  4
         [00000006]: ALLOC, EXEC
    [ 2] .rel.text
         REL             00000000 0002f8 000018 08   7   1  4
         [00000000]: 
    [ 3] .data
         PROGBITS        00000000 00006c 000008 00   0   0  4
         [00000003]: WRITE, ALLOC
    [ 4] .bss
         NOBITS          00000000 000074 000000 00   0   0  4
         [00000003]: WRITE, ALLOC
    [ 5] .reginfo
         MIPS_REGINFO    00000000 000074 000018 00   0   0  1
         [00000002]: ALLOC
    [ 6] .shstrtab
         STRTAB          00000000 00008c 000039 00   0   0  1
         [00000000]: 
    [ 7] .symtab
         SYMTAB          00000000 000230 0000a0 10   8   6  4
         [00000000]: 
    [ 8] .strtab
         STRTAB          00000000 0002d0 000025 00   0   0  1
         [00000000]: 

  Relocation section '.rel.text' at offset 0x2f8 contains 3 entries:
   Offset     Info    Type            Sym.Value  Sym. Name
  00000000  00000905 R_MIPS_HI16       00000000   _gp_disp
  00000004  00000906 R_MIPS_LO16       00000000   _gp_disp
  0000001c  00000709 R_MIPS_GOT16      00000004   gI


As depicted in `section Handle $gp register in PIC addressing mode`_, it 
translate **“.cpload %reg”** into the following.

.. code-block:: c++

  // Lower ".cpload $reg" to
  //  "lui   $gp, %hi(_gp_disp)"
  //  "addiu $gp, $gp, %lo(_gp_disp)"
  //  "addu  $gp, $gp, $t9"

The _gp_disp value is determined by loader. So, it's undefined in obj. 
You can find the Relocation Records for offset 0 and 4 of .text section 
referred to _gp_disp value. 
The offset 0 and 4 of .text section are instructions "lui $gp, %hi(_gp_disp)"
and "addiu $gp, $gp, %lo(_gp_disp)" which their corresponding obj 
encode are 0fa00000 and  09aa0000. 
The obj translate the %hi(_gp_disp) and %lo(_gp_disp) into 0 since when loader 
load this obj into memory, loader will know the _gp_disp value at run time and 
will update these two offset relocation records into the correct offset value. 
You can check if the cpu0 of %hi(_gp_disp) and %lo(_gp_disp) are correct by 
above mips Relocation Records of R_MIPS_HI(_gp_disp) and  R_MIPS_LO(_gp_disp) 
even though the cpu0 is not a CPU recognized by readelf utilitly. 
The instruction **“ld $2, %got(gI)($gp)”** is same since we don't know what the 
address of .data section variable will load to. 
So, translate the address to 0 and made a relocation record on 0x00000020 of 
.text section. Linker or Loader will change this address when this program is 
linked or loaded depend on the program is static link or dynamic link.


Cpu0 ELF related files
-----------------------

Files Cpu0ELFObjectWrite.cpp and Cpu0MC*.cpp are the files take care the obj 
format. 
Most obj code translation are defined by Cpu0InstrInfo.td and 
Cpu0RegisterInfo.td. 
With these td description, LLVM translate Cpu0 instruction into obj format 
automatically.


llvm-objdump
-------------

llvm-objdump -t -r
~~~~~~~~~~~~~~~~~~

In iMac, ``gobjdump -tr`` can display the information of relocation records 
like ``readelf -tr``. LLVM tool llvm-objdump is the same tool as objdump. 
Let's run gobjdump and llvm-objdump commands as follows to see the differences. 

.. code-block:: bash

  118-165-83-12:InputFiles Jonathan$ clang -target mips-unknown-linux-gnu -c 
  ch9_3.cpp -emit-llvm -o ch9_3.bc
  118-165-83-10:InputFiles Jonathan$ /Users/Jonathan/llvm/test/cmake_debug_build/
  bin/Debug/llc -march=cpu0 -relocation-model=pic -filetype=obj ch9_3.bc -o 
  ch9_3.cpu0.o

  118-165-78-12:InputFiles Jonathan$ gobjdump -t -r ch9_3.cpu0.o
  
  ch9_3.cpu0.o:     file format elf32-big

  SYMBOL TABLE:
  00000000 l    df *ABS*	00000000 ch9_3.bc
  00000000 l    d  .text	00000000 .text
  00000000 l    d  .data	00000000 .data
  00000000 l    d  .bss	00000000 .bss
  00000000 g     F .text	00000084 _Z5sum_iiz
  00000084 g     F .text	00000080 main
  00000000         *UND*	00000000 _gp_disp


  RELOCATION RECORDS FOR [.text]:
  OFFSET   TYPE              VALUE 
  00000084 UNKNOWN           _gp_disp
  00000088 UNKNOWN           _gp_disp
  000000e0 UNKNOWN           _Z5sum_iiz


  118-165-83-10:InputFiles Jonathan$ /Users/Jonathan/llvm/test/cmake_debug_build/
  bin/Debug/llvm-objdump -t -r ch9_3_3.cpu0.o
  
  ch9_3.cpu0.o:	file format ELF32-CPU0

  RELOCATION RECORDS FOR [.text]:
  132 R_CPU0_HI16 _gp_disp
  136 R_CPU0_LO16 _gp_disp
  224 R_CPU0_CALL16 _Z5sum_iiz

  SYMBOL TABLE:
  00000000 l    df *ABS*	00000000 ch9_3.bc
  00000000 l    d  .text	00000000 .text
  00000000 l    d  .data	00000000 .data
  00000000 l    d  .bss	00000000 .bss
  00000000 g     F .text	00000084 _Z5sum_iiz
  00000084 g     F .text	00000080 main
  00000000         *UND*	00000000 _gp_disp


The latter llvm-objdump can display the file format and relocation records 
information since we add the relocation records information in ELF.h as follows, 

.. rubric:: include/support/ELF.h
.. code-block:: c++

  // Machine architectures
  enum {
    ...
    EM_CPU0          = 201, // Document Write An LLVM Backend Tutorial For Cpu0
    ...
  }
  

.. rubric:: include/object/ELF.h
.. code-block:: c++

  ...
  template<support::endianness target_endianness, bool is64Bits>
  error_code ELFObjectFile<target_endianness, is64Bits>
              ::getRelocationTypeName(DataRefImpl Rel,
                        SmallVectorImpl<char> &Result) const {
    ...
    switch (Header->e_machine) {
    ...
    case ELF::EM_CPU0:	// llvm-objdump -t -r
      switch (Type) {
        LLVM_ELF_SWITCH_RELOC_TYPE_NAME(R_CPU0_NONE);
        LLVM_ELF_SWITCH_RELOC_TYPE_NAME(R_CPU0_24);
        LLVM_ELF_SWITCH_RELOC_TYPE_NAME(R_CPU0_32);
        LLVM_ELF_SWITCH_RELOC_TYPE_NAME(R_CPU0_HI16);
        LLVM_ELF_SWITCH_RELOC_TYPE_NAME(R_CPU0_LO16);
        LLVM_ELF_SWITCH_RELOC_TYPE_NAME(R_CPU0_GPREL16);
        LLVM_ELF_SWITCH_RELOC_TYPE_NAME(R_CPU0_LITERAL);
        LLVM_ELF_SWITCH_RELOC_TYPE_NAME(R_CPU0_GOT16);
        LLVM_ELF_SWITCH_RELOC_TYPE_NAME(R_CPU0_PC16);
        LLVM_ELF_SWITCH_RELOC_TYPE_NAME(R_CPU0_PC24);
        LLVM_ELF_SWITCH_RELOC_TYPE_NAME(R_CPU0_CALL16);
        LLVM_ELF_SWITCH_RELOC_TYPE_NAME(R_CPU0_GOT_HI16);
        LLVM_ELF_SWITCH_RELOC_TYPE_NAME(R_CPU0_GOT_LO16);
        LLVM_ELF_SWITCH_RELOC_TYPE_NAME(R_CPU0_RELGOT);
        LLVM_ELF_SWITCH_RELOC_TYPE_NAME(R_CPU0_TLS_TPREL32);
        LLVM_ELF_SWITCH_RELOC_TYPE_NAME(R_CPU0_GLOB_DAT);
        LLVM_ELF_SWITCH_RELOC_TYPE_NAME(R_CPU0_JUMP_SLOT);
      default: break;
      }
      break;
    ...
    }
  
  
  template<support::endianness target_endianness, bool is64Bits>
  error_code ELFObjectFile<target_endianness, is64Bits>
              ::getRelocationValueString(DataRefImpl Rel,
                        SmallVectorImpl<char> &Result) const {
    ...
    case ELF::EM_CPU0:  // llvm-objdump -t -r
    res = symname;
    break;
    ...
  }
  
  template<support::endianness target_endianness, bool is64Bits>
  StringRef ELFObjectFile<target_endianness, is64Bits>
               ::getFileFormatName() const {
    switch(Header->e_ident[ELF::EI_CLASS]) {
    case ELF::ELFCLASS32:
    switch(Header->e_machine) {
    ...
    case ELF::EM_CPU0:  // llvm-objdump -t -r
      return "ELF32-CPU0";
    ...
  }
  
  template<support::endianness target_endianness, bool is64Bits>
  unsigned ELFObjectFile<target_endianness, is64Bits>::getArch() const {
    switch(Header->e_machine) {
    ...
    case ELF::EM_CPU0:  // llvm-objdump -t -r
    return (target_endianness == support::little) ?
         Triple::cpu0el : Triple::cpu0;
    ...
  }


llvm-objdump -d
~~~~~~~~~~~~~~~~

Run the last Chapter example code with command ``llvm-objdump -d`` for dump 
file from elf to hex as follows, 

.. code-block:: bash

  JonathantekiiMac:InputFiles Jonathan$ clang -target mips-unknown-linux-gnu -c 
  ch8_1_1.cpp -emit-llvm -o ch8_1_1.bc
  JonathantekiiMac:InputFiles Jonathan$ /Users/Jonathan/llvm/test/cmake_debug_
  build/bin/Debug/llc -march=cpu0 -relocation-model=pic -filetype=obj ch8_1_1.bc 
  -o ch8_1_1.cpu0.o
  JonathantekiiMac:InputFiles Jonathan$ /Users/Jonathan/llvm/test/cmake_debug_
  build/bin/Debug/llvm-objdump -d ch8_1_1.cpu0.o
  
  ch8_1_1.cpu0.o: file format ELF32-unknown
  
  Disassembly of section .text:error: no disassembler for target cpu0-unknown-
  unknown

To support llvm-objdump, the following code added to Chapter10_1/ 
(the DecoderMethod for brtarget24 has been added in previous chapter).

.. rubric:: lbdex/Chapter10_1/CMakeLists.txt
.. code-block:: c++

  tablegen(LLVM Cpu0GenDisassemblerTables.inc -gen-disassembler)
  ...
  add_subdirectory(Disassembler)
  ...
  
.. rubric:: lbdex/Chapter10_1/LLVMBuild.txt
.. code-block:: c++

  [common]
  subdirectories = 
    Disassembler 
  ...
  has_disassembler = 1
  ...

.. rubric:: lbdex/Chapter10_1/Cpu0InstrInfo.td
.. code-block:: c++

  class CmpInstr<bits<8> op, string instr_asm, 
           InstrItinClass itin, RegisterClass RC, RegisterClass RD, 
           bit isComm = 0>:
    FA<op, (outs RD:$rc), (ins RC:$ra, RC:$rb),
     !strconcat(instr_asm, "\t$sw, $ra, $rb"), [], itin> {
    ...
    let DecoderMethod = "DecodeCMPInstruction";
  }
  ...
  let isBranch=1, isTerminator=1, isBarrier=1, imm16=0, hasDelaySlot = 1,
    isIndirectBranch = 1 in
  class JumpFR<bits<8> op, string instr_asm, RegisterClass RC>:
    FL<op, (outs), (ins RC:$ra),
     !strconcat(instr_asm, "\t$ra"), [(brind RC:$ra)], IIBranch> {
    let rb = 0;
    let imm16 = 0;
  }
  
  let isCall=1, hasDelaySlot=0 in {
    class JumpLink<bits<8> op, string instr_asm>:
    FJ<op, (outs), (ins calltarget:$target, variable_ops),
       !strconcat(instr_asm, "\t$target"), [(Cpu0JmpLink imm:$target)],
       IIBranch> {
       let DecoderMethod = "DecodeJumpAbsoluteTarget";
       }
  
  def JR      : JumpFR<0x2C, "ret", CPURegs>;
  
.. rubric:: lbdex/Chapter10_1/Disassembler/CMakeLists.txt
.. literalinclude:: ../../../lib/Target/Cpu0/Disassembler/CMakeLists.txt
  
.. rubric:: lbdex/Chapter10_1/Disassembler/LLVMBuild.txt
.. literalinclude:: ../../../lib/Target/Cpu0/Disassembler/LLVMBuild.txt

.. rubric:: lbdex/Chapter10_1/Disassembler/Cpu0Disassembler.cpp
.. literalinclude:: ../../../lib/Target/Cpu0/Disassembler/Cpu0Disassembler.cpp
  

As above code, it add directory Disassembler for handling the obj to assembly 
code reverse translation. So, add Disassembler/Cpu0Disassembler.cpp and modify 
the CMakeList.txt and LLVMBuild.txt to build with directory Disassembler and 
enable the disassembler table generated by "has_disassembler = 1". 
Most of code is handled by the table of \*.td files defined. 
Not every instruction in \*.td can be disassembled without trouble even though 
they can be translated into assembly and obj successfully. 
For those cannot be disassembled, LLVM supply the **"let DecoderMethod"** 
keyword to allow programmers implement their decode function. 
In Cpu0 example, we define function DecodeCMPInstruction(), DecodeBranch24Target()
and DecodeJumpAbsoluteTarget() in Cpu0Disassembler.cpp and tell the LLVM table 
driven system by write **"let DecoderMethod = ..."** in the corresponding 
instruction definitions or ISD node of Cpu0InstrInfo.td. 
LLVM will call these DecodeMethod when user use Disassembler job in tools, like 
``llvm-objdump -d``.
You can check the comments above these DecodeMethod functions to see how it 
work.
For the CMP instruction, according the definiton of CmpInstr<...> in 
Cpu0InstrInfo.td, the assembler will print as $sw, $ra, $rb. ($sw is a 
fixed name operand and won't exists in instruction), and encode as "10230000" 
in elf binary. Since $sw is a fixed operand, we choose assigning $rc to 0. 
You can define the CmpInstr as follows,

.. code-block:: c++

  class CmpInstr<bits<8> op, string instr_asm, 
                 InstrItinClass itin, RegisterClass RC, RegisterClass RD, 
                 bit isComm = 0>:
    FA<op, (outs RD:$ra), (ins RC:$rb, RC:$rc),
       !strconcat(instr_asm, "\t$ra, $rb, $rc"), [], itin> {
    let shamt = 0;
  }

Above definition will encode this CMP example instruction to "10a23000" and 
no need the DecoderMethod function. Just like other instruction the TablGen 
take care everthing on them. We choose not to encode $sw to show how to take 
care the implicit operand in binary form. The following code can set operand 
$sw as implicit operand (not shown) in assembly form as follows,

.. code-block:: c++

  class CmpInstr<bits<8> op, string instr_asm, 
                 InstrItinClass itin, RegisterClass RC, RegisterClass RD, 
                 bit isComm = 0>:
    FA<op, (outs RD:$rc), (ins RC:$ra, RC:$rb),
       !strconcat(instr_asm, "\t$ra, $rb"), [], itin> { // without display $sw
    ...
  }

It will display as "cmp $ra, $rb" in assembly form.

The RET (Cpu0ISD::Ret) and JR (ISD::BRIND) are both for "ret" instruction. 
The former is for instruction encode in assembly and obj while the latter is 
for decode in disassembler. 
The IR node Cpu0ISD::Ret is created in LowerReturn() which called at function 
exit point.

Finally cpu032II include all cpu032I instruction set and add some 
instrucitons. When ``llvm-objdump -d`` invoked, function ParseCpu0Triple() as 
the following will be called. Not like ``llc -mcpu=cpu032I`` can set mcpu type,
so the varaible CPU in ParseCpu0Triple() is empty when invoked by 
``llvm-objdump -d``. Set Cpu0ArchFeature to "+cpu032II" than it can disassemble 
all instructions.

.. rubric:: lbdex/Chapter10_1/MCTargetDesc/Cpu0MCTargetDesc.cpp
.. code-block:: c++

  static std::string ParseCpu0Triple(StringRef TT, StringRef CPU) {
    ...
    if (TheTriple == "cpu0" || TheTriple == "cpu0el") {
      if (CPU.empty() || CPU == "cpu032II") {
        Cpu0ArchFeature = "+cpu032II";
      } else if (CPU == "cpu032I") {
        Cpu0ArchFeature = "+cpu032I";
      }
    }
    return Cpu0ArchFeature;
  }

Now, run Chapter10_1/ with command ``llvm-objdump -d ch8_1_1.cpu0.o`` will get 
the following result.

.. code-block:: bash

  JonathantekiiMac:InputFiles Jonathan$ /Users/Jonathan/llvm/test/cmake_debug_
  build/bin/Debug/llc -march=cpu0 -relocation-model=pic -filetype=obj 
  ch8_1_1.bc -o ch8_1_1.cpu0.o
  JonathantekiiMac:InputFiles Jonathan$ /Users/Jonathan/llvm/test/cmake_debug_
  build/bin/Debug/llvm-objdump -d ch8_1_1.cpu0.o
  
  ch8_1_1.cpu0.o:	file format ELF32-CPU0

  Disassembly of section .text:
  _Z13test_control1v:
         0: 09 dd ff d8                                   addiu $sp, $sp, -40
         4: 09 30 00 00                                   addiu $3, $zero, 0
         8: 02 3d 00 24                                   st  $3, 36($sp)
         c: 09 20 00 01                                   addiu $2, $zero, 1
        10: 02 2d 00 20                                   st  $2, 32($sp)
        14: 09 40 00 02                                   addiu $4, $zero, 2
        18: 02 4d 00 1c                                   st  $4, 28($sp)
        1c: 09 40 00 03                                   addiu $4, $zero, 3
        20: 02 4d 00 18                                   st  $4, 24($sp)
        24: 09 40 00 04                                   addiu $4, $zero, 4
        28: 02 4d 00 14                                   st  $4, 20($sp)
        2c: 09 40 00 05                                   addiu $4, $zero, 5
        30: 02 4d 00 10                                   st  $4, 16($sp)
        34: 09 40 00 06                                   addiu $4, $zero, 6
        38: 02 4d 00 0c                                   st  $4, 12($sp)
        3c: 09 40 00 07                                   addiu $4, $zero, 7
        40: 02 4d 00 08                                   st  $4, 8($sp)
        44: 09 40 00 08                                   addiu $4, $zero, 8
        48: 02 4d 00 04                                   st  $4, 4($sp)
        4c: 09 40 00 09                                   addiu $4, $zero, 9
        50: 02 4d 00 00                                   st  $4, 0($sp)
        54: 01 4d 00 24                                   ld  $4, 36($sp)
        58: 10 43 00 00                                   cmp $sw, $4, $3
        5c: 31 00 00 10                                   jne $sw, 16
        60: 36 00 00 00                                   jmp 0
        64: 01 4d 00 24                                   ld  $4, 36($sp)
        68: 09 44 00 01                                   addiu $4, $4, 1
        6c: 02 4d 00 24                                   st  $4, 36($sp)
        70: 01 4d 00 20                                   ld  $4, 32($sp)
        74: 10 43 00 00                                   cmp $sw, $4, $3
        78: 30 00 00 10                                   jeq $sw, 16
        7c: 36 00 00 00                                   jmp 0
        80: 01 4d 00 20                                   ld  $4, 32($sp)
        84: 09 44 00 01                                   addiu $4, $4, 1
        88: 02 4d 00 20                                   st  $4, 32($sp)
        8c: 01 4d 00 1c                                   ld  $4, 28($sp)
        90: 10 42 00 00                                   cmp $sw, $4, $2
        94: 32 00 00 10                                   jlt $sw, 16
        98: 36 00 00 00                                   jmp 0
        9c: 01 4d 00 1c                                   ld  $4, 28($sp)
        a0: 09 44 00 01                                   addiu $4, $4, 1
        a4: 02 4d 00 1c                                   st  $4, 28($sp)
        a8: 01 4d 00 18                                   ld  $4, 24($sp)
        ac: 10 43 00 00                                   cmp $sw, $4, $3
        b0: 32 00 00 10                                   jlt $sw, 16
        b4: 36 00 00 00                                   jmp 0
        b8: 01 4d 00 18                                   ld  $4, 24($sp)
        bc: 09 44 00 01                                   addiu $4, $4, 1
        c0: 02 4d 00 18                                   st  $4, 24($sp)
        c4: 09 40 ff ff                                   addiu $4, $zero, -1
        c8: 01 5d 00 14                                   ld  $5, 20($sp)
        cc: 10 54 00 00                                   cmp $sw, $5, $4
        d0: 33 00 00 10                                   jgt $sw, 16
        d4: 36 00 00 00                                   jmp 0
        d8: 01 4d 00 14                                   ld  $4, 20($sp)
        dc: 09 44 00 01                                   addiu $4, $4, 1
        e0: 02 4d 00 14                                   st  $4, 20($sp)
        e4: 01 4d 00 10                                   ld  $4, 16($sp)
        e8: 10 43 00 00                                   cmp $sw, $4, $3
        ec: 33 00 00 10                                   jgt $sw, 16
        f0: 36 00 00 00                                   jmp 0
        f4: 01 3d 00 10                                   ld  $3, 16($sp)
        f8: 09 33 00 01                                   addiu $3, $3, 1
        fc: 02 3d 00 10                                   st  $3, 16($sp)
       100: 01 3d 00 0c                                   ld  $3, 12($sp)
       104: 10 32 00 00                                   cmp $sw, $3, $2
       108: 33 00 00 10                                   jgt $sw, 16
       10c: 36 00 00 00                                   jmp 0
       110: 01 3d 00 0c                                   ld  $3, 12($sp)
       114: 09 33 00 01                                   addiu $3, $3, 1
       118: 02 3d 00 0c                                   st  $3, 12($sp)
       11c: 01 3d 00 08                                   ld  $3, 8($sp)
       120: 10 32 00 00                                   cmp $sw, $3, $2
       124: 32 00 00 10                                   jlt $sw, 16
       128: 36 00 00 00                                   jmp 0
       12c: 01 2d 00 08                                   ld  $2, 8($sp)
       130: 09 22 00 01                                   addiu $2, $2, 1
       134: 02 2d 00 08                                   st  $2, 8($sp)
       138: 01 2d 00 08                                   ld  $2, 8($sp)
       13c: 01 3d 00 04                                   ld  $3, 4($sp)
       140: 10 32 00 00                                   cmp $sw, $3, $2
       144: 35 00 00 10                                   jge $sw, 16
       148: 36 00 00 00                                   jmp 0
       14c: 01 2d 00 04                                   ld  $2, 4($sp)
       150: 09 22 00 01                                   addiu $2, $2, 1
       154: 02 2d 00 04                                   st  $2, 4($sp)
       158: 01 2d 00 20                                   ld  $2, 32($sp)
       15c: 01 3d 00 24                                   ld  $3, 36($sp)
       160: 10 32 00 00                                   cmp $sw, $3, $2
       164: 30 00 00 10                                   jeq $sw, 16
       168: 36 00 00 00                                   jmp 0
       16c: 01 2d 00 00                                   ld  $2, 0($sp)
       170: 09 22 00 01                                   addiu $2, $2, 1
       174: 02 2d 00 00                                   st  $2, 0($sp)
       178: 01 2d 00 20                                   ld  $2, 32($sp)
       17c: 01 3d 00 24                                   ld  $3, 36($sp)
       180: 11 23 20 00                                   addu  $2, $3, $2
       184: 01 3d 00 1c                                   ld  $3, 28($sp)
       188: 11 22 30 00                                   addu  $2, $2, $3
       18c: 01 3d 00 18                                   ld  $3, 24($sp)
       190: 11 22 30 00                                   addu  $2, $2, $3
       194: 01 3d 00 14                                   ld  $3, 20($sp)
       198: 11 22 30 00                                   addu  $2, $2, $3
       19c: 01 3d 00 10                                   ld  $3, 16($sp)
       1a0: 11 22 30 00                                   addu  $2, $2, $3
       1a4: 01 3d 00 0c                                   ld  $3, 12($sp)
       1a8: 11 22 30 00                                   addu  $2, $2, $3
       1ac: 01 3d 00 08                                   ld  $3, 8($sp)
       1b0: 11 22 30 00                                   addu  $2, $2, $3
       1b4: 01 3d 00 04                                   ld  $3, 4($sp)
       1b8: 11 22 30 00                                   addu  $2, $2, $3
       1bc: 01 3d 00 00                                   ld  $3, 0($sp)
       1c0: 11 22 30 00                                   addu  $2, $2, $3
       1c4: 09 dd 00 28                                   addiu $sp, $sp, 40
       1c8: 3c e0 00 00                                   ret $lr



.. _section Handle $gp register in PIC addressing mode:
	http://jonathan2251.github.io/lbd/funccall.html#handle-gp-register-in-pic-addressing-mode


.. [#] http://en.wikipedia.org/wiki/Executable_and_Linkable_Format

.. [#] http://jonathan2251.github.io/lbd/install.html#install-other-tools-on-imac

.. [#] Leland Beck, System Software: An Introduction to Systems Programming. 

.. [#] http://ccckmit.wikidot.com/lk:aout

.. [#] http://ccckmit.wikidot.com/lk:objfile

.. [#] http://ccckmit.wikidot.com/lk:elf

