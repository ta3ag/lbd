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

extern int main();
extern void (*p_dynamic_linker)();
// The start address of reset
// boot:
/*asm("boot:");
asm("jmp _start"); // RESET: jmp RESET_START;
asm("jmp 4");  // ERROR: jmp ERR_HANDLE;
asm("jmp 4");  // IRQ: jmp IRQ_HANDLE;
asm("jmp -4"); // ERR_HANDLE: jmp ERR_HANDLE; (loop forever)
*/
void start() {
  asm("lui   $1,  0x7");
  asm("ori   $1,  $1, 0xfff0");
  asm("ld    $gp, 0($1)"); // load $gp($10) value from 0x7fff0
  asm("addiu $1,  $ZERO, 0x10");
  asm("st    $1,  0($gp)"); // 0($gp) = 0x10 (= 2nd .PTL0)
  asm("addiu $1,	$ZERO, 0");
  asm("addiu $2,	$ZERO, 0");
  asm("addiu $3,	$ZERO, 0");
  asm("addiu $4,	$ZERO, 0");
  asm("addiu $5,	$ZERO, 0");
  asm("addiu $6,	$ZERO, 0");
  asm("addiu $7,	$ZERO, 0");
  asm("addiu $8,	$ZERO, 0");
  asm("addiu $9,	$ZERO, 0");
  asm("addiu $11, $ZERO, 0");
  asm("addiu $12, $ZERO, 0");
  
  asm("addiu $sp, $zero, 0x6ffc");
  main();
  asm("addiu $14, $ZERO, -1");
  asm("ret $14");
}
