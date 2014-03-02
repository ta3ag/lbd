// clang -target mips-unknown-linux-gnu -O2 -c ch12_2.cpp -emit-llvm -o ch12_2.bc
// /Users/Jonathan/llvm/test/cmake_debug_build/bin/Debug/llc -march=cpu0 -mcpu=cpu032I -relocation-model=static -filetype=asm ch12_2.bc -o -
// /Users/Jonathan/llvm/test/cmake_debug_build/bin/Debug/llc -march=cpu0 -mcpu=cpu032II -relocation-model=static -filetype=asm ch12_2.bc -o -


/// start
int test_OptSlt(int a, int b, int c)
{
  int d = 0, e = 0, f = 0;

  d = (a < 1);
  e = (b < 2);
  f = d + e;

  return (f);
}

