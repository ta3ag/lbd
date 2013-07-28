// ~/llvm/release/cmake_debug_build/bin/Debug/clang -target mips-unknown-linux-gnu -c test_str.cpp -emit-llvm -o test_str.bc
// /Users/Jonathan/llvm/test/cmake_debug_build/bin/Debug/llc -march=cpu0 -relocation-model=static -filetype=obj test_str.bc -o test_str.o
// /Users/Jonathan/llvm/test/cmake_debug_build/bin/Debug/llvm-objdump -d test_str.o | tail -n +6| awk '{print "/* " $1 " */\t" $2 " " $3 " " $4 " " $5 "\t/* " $6"\t" $7" " $8" " $9" " $10 "\t*/"}' > ../cpu0_verilog/raw/cpu0s.hex

#include "boot.cpp"

#include "print.h"

int main()
{
  char str[15] = "Hello world";
  print_string(str);
//  print_integer(15);

  return 0;
}

#include "print.cpp"
