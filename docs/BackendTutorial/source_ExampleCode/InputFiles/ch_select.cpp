// clang -O2 -c ch_select.cpp -emit-llvm -o ch_select.bc
// /usr/local/llvm/test/cmake_debug_build/bin/llc -march=cpu0 -relocation-model=pic -filetype=asm ch_select.bc -o -

int select(int a, int b)
{
  int c = 0;

  if (a < b)
    return 1;
  else
    return 2;
}

