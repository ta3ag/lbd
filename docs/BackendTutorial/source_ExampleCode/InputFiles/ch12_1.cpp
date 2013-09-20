// clang -target mips-unknown-linux-gnu -c ch12_1.cpp -emit-llvm -o ch12_1.bc
// /Users/Jonathan/llvm/test/cmake_debug_build/bin/Debug/llc -march=cpu0 -relocation-model=static -filetype=asm -enable-cpu0-del-useless-jmp=false ch12_1.bc -o -
// /Users/Jonathan/llvm/test/cmake_debug_build/bin/Debug/llc -march=cpu0 -relocation-model=static -filetype=asm -stats ch12_1.bc -o -

/// start
int main()
{
  int a = 0;
  int b = 1;
  int c = 2;
  
  if (a == 0) {
    a++;
  }
  if (b == 0) {
    a = a + b;
  } else if (b < 0) {
    a = a--;
  }
  if (c > 0) {
    c++;
  }
  
  return a;
}

