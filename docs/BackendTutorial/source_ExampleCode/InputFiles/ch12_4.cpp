// clang -O1 -target mips-unknown-linux-gnu -c ch12_4.cpp -emit-llvm -o ch12_4.bc
// ~/llvm/test/cmake_debug_build/bin/Debug/llc -march=cpu0 -mcpu=cpu032I -relocation-model=static -filetype=asm ch12_4.bc -o -
// /usr/local/llvm/test/cmake_debug_build/bin/llc -march=cpu0 -mcpu=cpu032I -relocation-model=static -filetype=asm ch12_4.bc -o -

/// start
int select_1(int a, int b)
{
  int c = 0;

  if (a < b)
    return 1;
  else
    return 2;
}

int select_2(int a)
{
  int c = 0;

  if (a)
    c = 1;
  else
    c = 3;

  return c;
}

int test_select(int a, int b)
{
  int c = 0;
  
  c = select_1(a, b);
  c += select_2(a);
  
  return c;
}

