// clang -target mips-unknown-linux-gnu -c ch8_1_4.cpp -emit-llvm -o ch8_1_4.bc
// /Users/Jonathan/llvm/test/cmake_debug_build/bin/Debug/llc -march=cpu0 -relocation-model=pic -filetype=asm ch8_1_4.bc -o -

/// start
int main()
{
  int a[3]={0, 1, 2};
    
  return 0;
}
