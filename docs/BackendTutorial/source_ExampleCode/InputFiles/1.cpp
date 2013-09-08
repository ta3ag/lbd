// ~/llvm/release/cmake_debug_build/bin/Debug/clang -target mips-unknown-linux-gnu -c ch_run_sum_i.cpp -emit-llvm -o ch_run_sum_i.bc
// /Users/Jonathan/llvm/test/cmake_debug_build/bin/Debug/llc -march=cpu0 -relocation-model=static -filetype=obj ch_run_sum_i.bc -o ch_run_sum_i.cpu0.o
// /Users/Jonathan/llvm/test/cmake_debug_build/bin/Debug/llvm-objdump -d ch_run_sum_i.cpu0.o | tail -n +6| awk '{print "/* " $1 " */\t" $2 " " $3 " " $4 " " $5 "\t/* " $6"\t" $7" " $8" " $9" " $10 "\t*/"}' > ../cpu0_verilog/raw/cpu0s.hex

// /Users/Jonathan/llvm/test/cmake_debug_build/bin/Debug/llvm-objdump -d ch_run_sum_i.cpu0.o | tail -n +6| awk '{print "/* " $1 " */\t" $2 " " $3 " " $4 " " $5 "\t/* " $6"\t" $7" " $8" " $9" " $10 "\t*/"}' > ../cpu0_verilog/redesign/cpu0s.hex

// /home/cschen/test/lld_20130816/cmake_debug_build/bin/clang -target mips-unknown-linux-gnu -c ch_run_sum_i.cpp -emit-llvm -o ch_run_sum_i.bc
// /home/cschen/test/lld_20130816/cmake_debug_build/bin/llc -march=cpu0 -relocation-model=static -filetype=obj ch_run_sum_i.bc -o ch_run_sum_i.cpu0.o
// /home/cschen/test/lld_20130816/cmake_debug_build/bin/lld -flavor gnu -target cpu0-unknown-linux-gnu ch_run_sum_i.cpu0.o -o a.out
// /home/cschen/test/lld_20130816/cmake_debug_build/bin/llvm-objdump -elf2hex a.out > ../cpu0_verilog/raw/cpu0s.hex

// /home/Gamma/test/lld_0816/cmake_debug_build/bin/clang -target mips-unknown-linux-gnu -c 1.cpp -emit-llvm -o 1.bc
// /home/Gamma/test/lld_0816/cmake_debug_build/bin/llc -march=cpu0 -relocation-model=static -filetype=obj 1.bc -o 1.cpu0.o
// /home/Gamma/test/lld_0816/cmake_debug_build/bin/lld -flavor gnu -target cpu0-unknown-linux-gnu 1.cpu0.o -o a.out
// /home/Gamma/test/lld_0816/cmake_debug_build/bin/llvm-objdump -elf2hex a.out > ../cpu0_verilog/raw/cpu0s.hex

#include "boot.cpp"

#include "print.h"
int sum_i(char* buf, int amount, ...);

int main()
{
  char buf[80];
  int a = 0;
  a = sum_i(buf, 6, 0, 1, 2, 3, 4, 5);
  print_integer(a); // a = 15

  return a;
}

#include "print.cpp"

#include <stdarg.h>
int sum_i(char* buf, int amount, ...)
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
