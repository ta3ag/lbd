// ~/llvm/release/cmake_debug_build/bin/Debug/clang -target mips-unknown-linux-gnu -c ch_run_sum_i.cpp -emit-llvm -o ch_run_sum_i.bc
// /Users/Jonathan/llvm/test/cmake_debug_build/bin/Debug/llc -march=cpu0 -relocation-model=static -filetype=obj ch_run_sum_i.bc -o ch_run_sum_i.cpu0.o
// /Users/Jonathan/llvm/test/cmake_debug_build/bin/Debug/llvm-objdump -d ch_run_sum_i.cpu0.o | tail -n +6| awk '{print "/* " $1 " */\t" $2 " " $3 " " $4 " " $5 "\t/* " $6"\t" $7" " $8" " $9" " $10 "\t*/"}' > ../cpu0_verilog/raw/cpu0s.hex

// /Users/Jonathan/llvm/test/cmake_debug_build/bin/Debug/llvm-objdump -d ch_run_sum_i.cpu0.o | tail -n +6| awk '{print "/* " $1 " */\t" $2 " " $3 " " $4 " " $5 "\t/* " $6"\t" $7" " $8" " $9" " $10 "\t*/"}' > ../cpu0_verilog/redesign/cpu0s.hex

#include <stdarg.h>

void print_integer(int x);
int sum_i(int amount, ...);

#define OUT_MEM 0x7000 // 28672

// boot:
  asm("jmp 12"); // RESET: jmp RESET_START;
  asm("jmp 4");  // ERROR: jmp ERR_HANDLE;
  asm("jmp 4");  // IRQ: jmp IRQ_HANDLE;
  asm("jmp -4"); // ERR_HANDLE: jmp ERR_HANDLE; (loop forever)

  // RESET_START:
  #include "InitRegs.h"
  
  asm("addiu $sp, $zero, 0x6ffc");
  asm("addiu $3, $ZERO, 0x50");
  asm("iret $3");

int main()
{
  int a = 0;
  a = sum_i(6, 0, 1, 2, 3, 4, 5);
  print_integer(a);    // a = 15

  return a;
}

// For memory IO
void print_integer(int x)
{
  int *p = (int*)OUT_MEM;
  *p = x;
 return;
}

int sum_i(int amount, ...)
{
  int i = 0;
  int val = 0;
  int sum = 0;
	
  va_list vl;
  va_start(vl, amount);
  for (i = 0; i < amount; i++)
  {
    val = va_arg(vl, int);
    sum += val;
  }
  va_end(vl);
  
  return sum; 
}
