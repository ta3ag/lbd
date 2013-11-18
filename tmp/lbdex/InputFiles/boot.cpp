/*cschen@cschen-BM6835-BM6635-BP6335:~/test/lbd/docs/BackendTutorial/source_ExampleCode/InputFiles$ /home/cschen/test/lld_20130816/cmake_debug_build/bin/llvm-objdump -t a.out

// _start: will create 2 _start in elf as follows,
a.out:	file format ELF32-CPU0

SYMBOL TABLE:
00000000         *UND*	00000000 
00000114 l     F .text	00000000 _start
...
00000164 g     F .text	00000078 main
...
00000000         *UND*	00000000 _start
*/

/// start
// boot:
  asm("boot:");
//  asm("_start:");
  asm("jmp 12"); // RESET: jmp RESET_START;
  asm("jmp 4");  // ERROR: jmp ERR_HANDLE;
  asm("jmp 4");  // IRQ: jmp IRQ_HANDLE;
  asm("jmp -4"); // ERR_HANDLE: jmp ERR_HANDLE; (loop forever)

  // RESET_START:
  #include "InitRegs.cpp"
  
  asm("addiu $sp, $zero, 0x6ffc");
  asm("addiu $3, $ZERO, 0x50"); // Assume main() entry point is address 0x50
  asm("iret $3");
