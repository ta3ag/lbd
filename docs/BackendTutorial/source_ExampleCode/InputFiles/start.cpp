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
// /usr/local/llvm/release/cmake_debug_build/bin/clang -target mips-unknown-linux-gnu -c start.cpp -emit-llvm -o start.bc
// /usr/local/llvm/release/cmake_debug_build/bin/llvm-dis start.bc -o start.ll

/// start

#include "dynamic_linker.h"

extern int main();

#define initRegs() \
  asm("addiu $1,	$ZERO, 0"); \
  asm("addiu $2,	$ZERO, 0"); \
  asm("addiu $3,	$ZERO, 0"); \
  asm("addiu $4,	$ZERO, 0"); \
  asm("addiu $5,	$ZERO, 0"); \
  asm("addiu $6,	$ZERO, 0"); \
  asm("addiu $7,	$ZERO, 0"); \
  asm("addiu $8,	$ZERO, 0"); \
  asm("addiu $9,	$ZERO, 0"); \
  asm("addiu $10,	$ZERO, 0"); \
  asm("addiu $fp, $ZERO, 0");

void start() {
//  asm("boot:");
  asm("lui   $1,  0x7");
  asm("ori   $1,  $1, 0xfff0");
  asm("ld    $gp, 0($1)"); // load $gp($11) value from 0x7fff0
  initRegs();
  asm("addiu $sp, $zero, 0x6ffc");
  main();
  asm("addiu $lr, $ZERO, -1");
  asm("ret $lr");
}
