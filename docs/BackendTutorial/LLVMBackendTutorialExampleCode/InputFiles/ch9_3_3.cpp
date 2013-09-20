// clang -target mips-unknown-linux-gnu -c ch9_3_3.cpp -emit-llvm -I/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.8.sdk/usr/include/ -o ch9_3_3.bc
// /Users/Jonathan/llvm/test/cmake_debug_build/bin/Debug/llc ch9_3_3.bc -o ch9_3_3.s
// clang++ ch9_3_3.s -o ch9_3_3.native
// ./ch9_3_3.native
// lldb -- ch9_3_3.native
// b main
// s
// ...
// print $rsp		; print %rsp, choose $ instead of % in assembly code

// mips-linux-gnu-g++ -g ch9_3_3.cpp -o ch9_3_3 -static
// qemu-mips ch9_3_3
// mips-linux-gnu-g++ -S ch9_3_3.cpp
// cat ch9_3_3.s

/// start
#include <stdio.h>
#include <stdarg.h>

int sum_i(int amount, ...)
{
//  int amount = 0;
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

int main()
{
  int a = sum_i(6, 1, 2, 3, 4, 5, 6);
  printf("a = %d\n", a);
	
  return a;
}
