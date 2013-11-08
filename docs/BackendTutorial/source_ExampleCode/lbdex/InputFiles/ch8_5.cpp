// clang -target mips-unknown-linux-gnu -c ch8_5.cpp -emit-llvm -o ch8_5.bc
// /Users/Jonathan/llvm/test/cmake_debug_build/bin/Debug/llc -march=cpu0 -relocation-model=pic -filetype=asm ch8_5.bc -o -

/// start
int test_over64bits()
{
  long long a = 0x3080000000;
  long long b = 0x2080000000;
  long long c = a / b;
  
  return 0;
}

int test_softfloat()
{
  long long a = 0x3080000000;
  long long b = 0x2080000000;
  double b2 = 20008000.3;
  
  float e = a * b;
  double d2 = a * b2;
  
  return 0;
}
