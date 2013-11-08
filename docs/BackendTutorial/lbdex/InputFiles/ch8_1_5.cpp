// C operator ==, !=, &&, || test

// clang -target mips-unknown-linux-gnu -c ch8_1_5.cpp -emit-llvm -o ch8_1_5.bc
// /Users/Jonathan/llvm/test/cmake_debug_build/bin/Debug/llc -march=cpu0 -relocation-model=pic -filetype=asm ch8_1_5.bc -o -

/// start
int main()
{
  unsigned int a = 0;
  int b = 1;
  int c = 2;
  
  if ((a == 0 && b == 2) || (c != 2)) {
    a++;
  }
  
  return 0;
}
