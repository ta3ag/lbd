// clang -target mips-unknown-linux-gnu -c ch7_4.cpp -emit-llvm -o ch7_4.bc
// /Users/Jonathan/llvm/test/cmake_debug_build/bin/Debug/llc -march=cpu0 -relocation-model=pic -filetype=asm ch7_4.bc -o -

/// start
int test_local_pointer()
{
  int b = 3;
  
  int* p = &b;

  return *p;
}

int test_longlong()
{
  long long a = 0x3080000000;
  long long b = 0x2080000000;
  int a1 = 0x30008000;
  int b1 = 0x20008000;
  
  long long c = a + b;
  c = a - b;
  long long d = a * b;
  d = a1 * b1;
  
  return 0;
}

