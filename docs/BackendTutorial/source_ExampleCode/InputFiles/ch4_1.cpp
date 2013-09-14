// clang -c ch4_1.cpp -emit-llvm -o ch4_1.bc
// /usr/local/llvm/test/cmake_debug_build/bin/llc -march=cpu0 -relocation-model=pic -filetype=asm ch4_1.bc -o ch4_1.cpu0.s
// /usr/local/llvm/test/cmake_debug_build/bin/llc -march=cpu0 -relocation-model=pic -filetype=obj ch4_1.bc -o ch4_1.cpu0.o

// /Users/Jonathan/llvm/test/cmake_debug_build/bin/Debug/llc -march=cpu0 -relocation-model=pic -filetype=asm ch4_1.bc -o ch4_1.cpu0.s
// /Users/Jonathan/llvm/test/cmake_debug_build/bin/Debug/llc -march=cpu0 -relocation-model=static -filetype=asm ch4_1.bc -o ch4_1.cpu0.static.s
// /Users/Jonathan/llvm/test/cmake_debug_build/bin/Debug/llc -march=cpu0 -relocation-model=pic -filetype=obj ch4_1.bc -o ch4_1.cpu0.o
// /Users/Jonathan/llvm/test/cmake_debug_build/bin/Debug/llc -march=cpu0el -relocation-model=pic -filetype=obj ch4_1.bc -o ch4_1.cpu0el.o

/// start
int test_math()
{
  int a = 5;
  int b = 2;
  int c = 0;
  int d = 0;
  int e, f = 0;
  unsigned int a1 = -5, e1 = 0, f1 = 0;

  c = a + b;
  d = a - b;
  d = a * b;
  e = (a << 2);
  e1 = (a1 << 2);
  f = (a >> 2);
  f1 = (a1 >> 2);
  e = (1 << a);
  e1 = (1 << a1);
  f = (0x80000000 >> a);
  f1 = (b >> a);

  return c;
}

