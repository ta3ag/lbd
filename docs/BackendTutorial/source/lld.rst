.. _sec-lld:

LLD for Cpu0
==============

This chapter add Cpu0 backend in lld. With this lld Cpu0 for ELF linker support,
the program with global variable can be allocated in ELF file format layout. 
Meaning the relocation records of global variable can be solved. In addition, 
llvm-objdump driver is modified for support generate Hex file from ELF.
With these two tool supported, the program with global variables exist in section
.data and .rodata can be accessed and transfered to Hex file which feed to 
Verilog Cpu0 machine and run on your PC/Laptop.


Install lld
-------------

LLD project is underdevelopment and can be compiled with c++11 standard (C++
2011 year announced standard). Currently, we only know how to build lld with 
llvm on Linux platform or Linux VM. Please let us know if you know how to build
it on iMac with Xcode. So, if you got iMac only, please install VM (such as 
Virtual Box). We porting lld Cpu0 at 2013/08/16, so please checkout the last
commit of 2013/08/15 as follows,



