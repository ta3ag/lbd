// clang -target mips-unknown-linux-gnu -c ch7_5.cpp -emit-llvm -o ch7_5.bc
// /Users/Jonathan/llvm/test/cmake_debug_build/bin/Debug/llc -march=cpu0 -relocation-model=static -filetype=asm ch7_5.bc -o -

/// start
int a[3] = {2012, 10, 12};

int test_struct()
{
  int i = a[1];

  return i; // 10
}

