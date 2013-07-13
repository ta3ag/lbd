// clang -target `llvm-config --host-target` -c ch11_2.cpp -emit-llvm -o ch11_2.bc
// /Users/Jonathan/llvm/test/cmake_debug_build/bin/Debug/llc -march=cpu0 -relocation-model=static -filetype=asm -enable-cpu0-del-useless-jmp=false ch11_2.bc -o ch11_2.cpu0.s
// /Users/Jonathan/llvm/test/cmake_debug_build/bin/Debug/llc -march=cpu0 -relocation-model=static -filetype=asm -stats ch11_2.bc -o ch11_2.cpu0.s

// /Users/Jonathan/llvm/test/cmake_debug_build/bin/Debug/llc -march=cpu0 -relocation-model=static -filetype=obj -stats ch11_2.bc -o ch11_2.cpu0.o
// /Users/Jonathan/llvm/test/cmake_debug_build/bin/Debug/llvm-objdump -d ch11_2.cpu0.o | tail -n +6| awk '{print "/* " $1 " */\t" $2 " " $3 " " $4 " " $5 "\t/* " $6"\t" $7" " $8" " $9" " $10 "\t*/"}' > ../cpu0_verilog/redesign/cpu0s.hex

#include "InitRegs.h"

#define OUT_MEM 0x7000 // 28672

asm("addiu $sp, $zero, 0x6ffc");

void test_operators_asm();

int main()
{
  test_operators_asm();

  return 0;
}

void test_operators_asm()
{
  asm("addiu $sp, $sp, -12");
  asm("st $2, 8($sp)");
  asm("st $3, 4($sp)");
  asm("st $4, 0($sp)");
  asm("lui $2, 0x7fff");
  asm("lui $3, 0x1000");
  asm("addu $4, $2, $3");
  asm("lui $2, 0x7fff");
  asm("lui $3, 0x1000");
  asm("add $4, $2, $3");	// overflow
  asm("lui $2, 0x8fff");
  asm("lui $3, 0x7000");
  asm("sub $4, $2, $3");	// overflow
  asm("lui $2, 0x0");
  asm("addiu $3, $0, -1");
  asm("sub $4, $2, $3");	// $4=1, no overflow
  asm("lui $2, -1");
  asm("ori $2, $2, 0xffff");	// $2=0xffffffff=-1
  asm("andi $2, $2, 0xffff");	// $2=0x0000ffff
  asm("shl $2, $2, 16");	// $2=0xffff0000
  asm("xori $2, $2, 0xffff");	// $2=0xffffffff=-1
  asm("addiu $3, $0, -1");	// $3=0xffffffff=-1
  asm("divu $2, $3");		// HI=0, LO=1
  asm("div $2, $3");		// HI=0, LO=1, overflow
  asm("xori $2, $2, 1");	// $2 = 0xffffffffe
  asm("rol $4, $2, 4");		// $2 = 0xfffffffef
  asm("ror $4, $2, 8");		// $2 = 0xfefffffff
  asm("ld $2, 8($sp)");
  asm("ld $3, 4($sp)");
  asm("ld $4, 0($sp)");
  asm("addiu $sp, $sp, 12");
}
