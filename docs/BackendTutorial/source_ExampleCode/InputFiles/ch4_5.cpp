// clang -c ch4_5.cpp -emit-llvm -o ch4_5.bc
// /Users/Jonathan/llvm/test/cmake_debug_build/bin/Debug/llc -march=cpu0 -relocation-model=pic -filetype=asm ch4_5.bc -o -

int test_andorxornot()
{
  int a = 5;
  int b = 3;
  int c = 0, d = 0, e = 0;

  c = (a & b);
  d = (a | b);
  e = (a ^ b);
  b = !a;
  
  return b;
}

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

