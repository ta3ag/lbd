// clang -I/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.8.sdk/usr/include/ -c ch8_10.cpp -emit-llvm -o ch8_10.bc
// /Users/Jonathan/llvm/test/cmake_debug_build/bin/Debug/llc -march=cpu0 -relocation-model=pic -filetype=asm ch8_10.bc -o ch8_10.cpu0.s

#include <alloca.h>

int sum(int x1, int x2, int x3, int x4, int x5, int x6)
{
  int sum = x1 + x2 + x3 + x4 + x5 + x6;
  
  return sum; 
}

int weight_sum(int x1, int x2, int x3, int x4, int x5, int x6)
{
  int *b = (int*)alloca(sizeof(int) * x1);
  *b = 1111;
  int weight = sum(6*x1, x2, x3, x4, 2*x5, x6);
  
  return weight; 
}

int main()
{
  int a = weight_sum(1, 2, 3, 4, 5, 6);
  
  return a;
}
