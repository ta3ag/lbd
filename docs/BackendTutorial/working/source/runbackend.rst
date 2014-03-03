.. _sec-runbackend:

Run backend
===========

This chapter will implement Cpu0 instruction by Verilog language.
We can write a C++ main 
function as well as the boot code by assembly hand code, and translate this 
main()+bootcode() into obj file.
Combined with llvm-objdump support in last chapter, 
this main()+bootcode() elf can be translated into hex file format which 
include the disassemble code as comment. 
Furthermore, we can design the Cpu0 with Verilog language tool and run the Cpu0 
backend on PC by feed the hex file and see the Cpu0 instructions execution 
result.


Verilog of CPU0
------------------

Verilog language is an IEEE standard in IC design. There are a lot of book and 
documents for this language. Web site [#]_ has a pdf [#]_ in this. 
Example code lbdex/cpu0_verilog/cpu0.v is the 
cpu0 design in Verilog. In Appendix A, we have downloaded and installed Icarus 
Verilog tool both on iMac and Linux. The cpu0.v and cpu0Is.v is a simple design 
with only few hundreds lines of code. 
Alough it has not the pipeline features, we can assume the 
cpu0 backend code run on the pipeline machine because the pipeline version  
use the same machine instructions. Verilog is C like language in syntex and 
this book is a compiler book, so we list the cpu0.v as well as the building 
command directly as below. We expect 
readers can understand the Verilog code just with a little patient and no need 
further explanation. According computer architecture, there are two type of I/O. 
One is memory mapped I/O, the other is instruction I/O. 
Cpu0 use memory mapped I/O where memory address 0x80000 as the output port. 
When meet the instruction **"st $ra, cx($rb)"**, where cx($rb) is 
0x80000, Cpu0 display the content as follows,

.. code-block:: c++

      ST : begin
        ...
        if (R[b]+c16 == `IOADDR) begin
          outw(R[a]);

.. rubric:: lbdex/cpu0_verilog/cpu0.v
.. literalinclude:: ../lbdex/cpu0_verilog/cpu0.v

.. rubric:: lbdex/cpu0_verilog/cpu0Is.v
.. literalinclude:: ../lbdex/cpu0_verilog/cpu0Is.v


.. code-block:: bash

  JonathantekiiMac:raw Jonathan$ pwd
  /Users/Jonathan/test/2/lbd/lbdex/cpu0_verilog/raw
  JonathantekiiMac:raw Jonathan$ iverilog -o cpu0Is cpu0Is.v 


Run program on CPU0 machine
---------------------------

Now let's compile ch_run_backend.cpp as below. Since code size grows up from 
low to high address and stack grows up from high to low address. We set $sp 
at 0x6ffc because assuming cpu0.v use 0x7000 bytes of memory.

.. rubric:: lbdex/InputFiles/start.h
.. literalinclude:: ../lbdex/InputFiles/start.h
    :start-after: /// start

.. rubric:: lbdex/InputFiles/print.h
.. literalinclude:: ../lbdex/InputFiles/print.h
    :start-after: /// start

.. rubric:: lbdex/InputFiles/print.cpp
.. literalinclude:: ../lbdex/InputFiles/print.cpp
    :start-after: /// start

.. rubric:: lbdex/InputFiles/ch_nolld.h
.. literalinclude:: ../lbdex/InputFiles/ch_nolld.h
    :start-after: /// start

.. rubric:: lbdex/InputFiles/ch_nolld.cpp
.. literalinclude:: ../lbdex/InputFiles/ch_nolld.cpp
    :start-after: /// start

.. rubric:: lbdex/InputFiles/ch_run_backend.cpp
.. literalinclude:: ../lbdex/InputFiles/ch_run_backend.cpp
    :start-after: /// start

.. rubric:: lbdex/InputFiles/build-run_backend.sh
.. literalinclude:: ../lbdex/InputFiles/build-run_backend.sh

Let's run Chapter11_2/ with ``llvm-objdump -d`` for input files 
ch_run_backend.cpp to generate the hex file of inputing to cpu0Is Verilog 
simulator to get the output result as below. 
Remind ch_run_backend.cpp have to be compiled with option 
``clang -target mips-unknown-linux-gnu`` and use the clang of
your build instead of download from Xcode on iMac. The ~/llvm/release/
cmake_debug_build/bin/Debug/ is my build clang from source code.


.. code-block:: bash

  JonathantekiiMac:InputFiles Jonathan$ bash build-run_backend.sh
  JonathantekiiMac:InputFiles Jonathan$ cd ../cpu0_verilog
  JonathantekiiMac:cpu0_verilog Jonathan$ ./cpu0Is
  WARNING: cpu0Is.v:386: $readmemh(cpu0.hex): Not enough words in the file for the 
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
  -21474836487
  7
  15
  2
  1
  RET to PC < 0, finished!


.. code-block:: bash

  JonathantekiiMac:raw Jonathan$ cd ../../InputFiles/
  JonathantekiiMac:InputFiles Jonathan$ /Users/Jonathan/llvm/test/cmake_debug_build
  /bin/Debug/llvm-objdump -d ch_run_backend.cpu0.o 
  ...
  _Z13print_integeri:
       ................                                   addiu $sp, $sp, -24
       ................                                   st  $lr, 20($sp)
       ................                                   st  $fp, 16($sp)
       ................                                   add $fp, $sp, $zero
       ................                                   ld  $2, 24($fp)
       ................                                   st  $2, 12($fp)
       ................                                   st  $2, 0($sp)
       ................                                   jsub  716
       ................                                   st  $2, 8($fp)
       ................                                   st  $2, 0($sp)
       ................                                   jsub  1084
       ................                                   add $sp, $fp, $zero
       ................                                   ld  $fp, 16($sp)
       ................                                   ld  $lr, 20($sp)
       ................                                   addiu $sp, $sp, 24
       ................                                   ret $lr
  ...
  _Z14print1_integeri:
       ................                                   addiu $sp, $sp, -8
       ................                                   st  $fp, 4($sp)
       ................                                   add $fp, $sp, $zero
       ................                                   ld  $2, 8($fp)
       ................                                   st  $2, 0($fp)
       ................                                   ld  $1, 8($sp)
       ................                                   st  $1, 28672($zero)
       ................                                   add $sp, $fp, $zero
       ................                                   ld  $fp, 4($sp)
       ................                                   addiu $sp, $sp, 8
       ................                                   ret $lr


You can trace the memory binary code and destination
register changed at every instruction execution by the following change and
get the result as below,

.. rubric:: lbdex/cpu0_verilog/cpu0Is.v

.. code-block:: c++

  `define TRACE 

.. rubric:: lbdex/cpu0_verilog/cpu0.v

.. code-block:: c++

      ...
      `D = 1; // Trace register content at beginning

.. code-block:: bash

  JonathantekiiMac:raw Jonathan$ ./cpu0Is
  WARNING: cpu0.v:386: $readmemh(cpu0.hex): Not enough words in the file for the 
  requested range [0:28671].
  00000000: 2600000c
  00000004: 26000004
  00000008: 26000004
  0000000c: 26fffffc
  00000010: 09100000
  00000014: 09200000
  ...
  taskInterrupt(001)
  1530ns 00000054 : 02ed002c m[28620+44  ]=-1          SW=00000000
  1610ns 00000058 : 02bd0028 m[28620+40  ]=0           SW=00000000
  ...                     
  RET to PC < 0, finished!


As above result, cpu0.v dump the memory first after read input cpu0.hex. 
Next, it run instructions from address 0 and print each destination 
register value in the fourth column. 
The first column is the nano seconds of timing. The second 
is instruction address. The third is instruction content. 
We have checked many example code is correct by print the variable with 
print_integer().

This chapter shows Verilog PC output by display the I/O memory mapped address but
didn't implementing the output hardware interface or port. 
The real output hardware 
interface/port is hardware output device dependent, such as RS232, speaker, 
LED, .... You should implement the I/O interface/port when you want to program 
FPGA and wire I/O device to the I/O port.


.. [#] http://www.ece.umd.edu/courses/enee359a/

.. [#] http://www.ece.umd.edu/courses/enee359a/verilog_tutorial.pdf
