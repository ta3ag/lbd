// clang -target mips-unknown-linux-gnu -c ch8_1_6.cpp -emit-llvm -o ch8_1_6.bc
// /Users/Jonathan/llvm/test/cmake_debug_build/bin/Debug/llc -march=cpu0 -relocation-model=pic -filetype=asm ch8_1_6.bc -o -

/// start
int main()
{
  int a = 3;
  
  if (a != 0)
    a++;
  goto L1;
  a++;
L1:
  a--;
    
  return a;
}
