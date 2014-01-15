// clang -target mips-unknown-linux-gnu -c ch_inlineasm.cpp -emit-llvm -o ch_inlineasm.bc
// /Users/Jonathan/llvm/test/cmake_debug_build/bin/Debug/llc -march=cpu0 -relocation-model=pic -filetype=asm ch_inlineasm.bc -o -

/// start
int test_inlineasm()
{
  int a = 2;
  int b = 1;
  int c;

  call i32 asm "addu $0, $1, $2", "=r,r,r"(i32 %0, i32 %1) nounwind
#if 0
asm ("add $0, $1, $2;"
             :"=r"(c)        /* output */
             :"r"(a)         /* input */
             :"r"(b)         /* input */
             );       
#endif
  return 0;
}

