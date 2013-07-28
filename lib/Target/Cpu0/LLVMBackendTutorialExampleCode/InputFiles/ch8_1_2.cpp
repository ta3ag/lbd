// clang -target mips-unknown-linux-gnu -c ch8_1_2.cpp -emit-llvm -o ch8_1_2.bc
// /Users/Jonathan/llvm/test/cmake_debug_build/bin/Debug/llc -march=cpu0 -relocation-model=static -filetype=asm ch8_1_2.bc -o -

int main()
{
  char str[81] = "Hello world";
  char s[6] = "Hello";
  
  return 0;
}
