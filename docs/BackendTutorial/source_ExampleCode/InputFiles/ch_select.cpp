// clang -O2 -c ch_select.cpp -emit-llvm -o ch_select.bc
// ~/llvm/test/cmake_debug_build/bin/Debug/llc -march=cpu0 -relocation-model=pic -filetype=asm ch_select.bc -o -
// /usr/local/llvm/test/cmake_debug_build/bin/llc -march=cpu0 -relocation-model=pic -filetype=asm ch_select.bc -o -

/// start
int select_1(int a, int b)
{
  int c = 0;

  if (a < b)
    return 1;
  else
    return 2;
}

int select_2(int a, int b)
{
  int c = 0;

  if (a < b)
    c = 1;

  return c;
}

int test_select(int a, int b)
{
  int c = 0;
  
  c = select_1(a, b);
  c += select_2(a, b);
  
  return c;
}
