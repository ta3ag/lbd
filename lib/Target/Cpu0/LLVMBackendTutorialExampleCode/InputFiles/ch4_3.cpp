// clang -c ch4_1_2.cpp -emit-llvm -o ch4_1_2.bc
// /usr/local/llvm/test/cmake_debug_build/bin/llc -march=cpu0 -relocation-model=pic -filetype=asm ch4_1_2.bc -o ch4_1_2.cpu0.s
// /usr/local/llvm/test/cmake_debug_build/bin/llc -march=cpu0 -relocation-model=pic -filetype=obj ch4_1_2.bc -o ch4_1_2.cpu0.o

// /Users/Jonathan/llvm/test/cmake_debug_build/bin/Debug/llc -march=cpu0 -relocation-model=pic -filetype=asm ch4_1_2.bc -o ch4_1_2.cpu0.s
// /Users/Jonathan/llvm/test/cmake_debug_build/bin/Debug/llc -march=cpu0 -relocation-model=static -filetype=asm ch4_1_2.bc -o ch4_1_2.cpu0.static.s
// /Users/Jonathan/llvm/test/cmake_debug_build/bin/Debug/llc -march=cpu0 -relocation-model=pic -filetype=obj ch4_1_2.bc -o ch4_1_2.cpu0.o
// /Users/Jonathan/llvm/test/cmake_debug_build/bin/Debug/llc -march=cpu0el -relocation-model=pic -filetype=obj ch4_1_2.bc -o ch4_1_2.cpu0el.o

/// start
int main()
{
  int a = 5;
  int b = 2;
  int d = 0;
  unsigned int a1 = -5, d1 = 0;

  d = a / b;
  d1 = a1 / b;

  return d;
}

