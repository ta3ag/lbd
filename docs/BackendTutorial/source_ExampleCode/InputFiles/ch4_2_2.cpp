// clang -c ch4_2_2.cpp -emit-llvm -o ch4_2_2.bc
// /usr/local/llvm/test/cmake_debug_build/bin/llc -march=cpu0 -relocation-model=pic -filetype=asm ch4_2_2.bc -o -

int test_setxx()
{
  int a = 5;
  int b = 3;
  
  b = (a == b); // seq
  b = (a != b); // sne
  b = (a < b);  // slt
  b = (a <= b); // sle
  b = (a > b);  // sgt
  b = (a >= b); // sge
  
  return b;
}

