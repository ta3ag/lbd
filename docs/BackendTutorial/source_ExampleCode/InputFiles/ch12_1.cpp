// clang -c ch12_1.cpp -emit-llvm -o ch12_1.bc
// /usr/local/llvm/test/cmake_debug_build/bin/llc -march=mips -filetype=asm ch12_1.bc -o ch12_1.mips.s

#include <alloca.h>

int sum_i(int x1, int x2, int x3, int x4, int x5, int x6)
{
  int *b = (int*)alloca(sizeof(int) * x1);
  int sum = x1 + x2 + x3 + x4 + x5 + x6;
  
  return sum; 
}

int main()
{
  int a = sum_i(1, 2, 3, 4, 5, 6);
  
  return a;
}
