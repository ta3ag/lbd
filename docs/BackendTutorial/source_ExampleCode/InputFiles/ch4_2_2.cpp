// clang -c ch4_2_2.cpp  -emit-llvm -o ch4_2_2.bc
// /Users/Jonathan/llvm/test/cmake_debug_build/bin/Debug/llc -march=cpu0 -relocation-model=pic -filetype=asm ch4_2_2.bc -o -

// test_mod()
int test_mod(int c)
{
  int b = 11;
  
  b = (b+1)%c;
  
  return b;
}
