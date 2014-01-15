// clang -target mips-unknown-linux-gnu -c ch11_2.cpp -emit-llvm -o ch11_2.bc
// ~/llvm/test/cmake_debug_build/bin/Debug/llc -march=cpu0 -relocation-model=static -filetype=asm ch11_2.bc -o -
// /usr/local/llvm/test/cmake_debug_build/bin/llc -march=cpu0 -relocation-model=static -filetype=asm ch11_2.bc -o -

/// start
int inlineasm_addu(void)
{
  int foo = 10, bar = 15;

  __asm__ __volatile__("addu %0,%1,%2"
                       :"=r"(foo)
                       :"r"(foo), "r"(bar)
                       );

  return foo;
}

int inlineasm_arg(int u, int v)
{
  int w;

  __asm__ __volatile__("subu %0,%1,%2"
                       :"=r"(w)
                       :"r"(u), "r"(v)
                       );

  return w;
}

int g[3] = {1,2,3};

int inlineasm_global()
{
  int d;
  __asm__ __volatile__("addiu %0,%1,1"
                       :"=r"(d) // d=4
                       :"r"(g[2])
                       );

  return d;
}

#ifdef TESTSOFTFLOATLIB
// test_float() will call soft float library
int inlineasm_float()
{
  float a = 2.2;
  float b = 3.3;
  
  int c = (int)(a + b);

  int d;
  __asm__ __volatile__("addiu %0,%1,1"
                       :"=r"(d)
                       :"r"(c)
                       );

  return d;
}
#endif

int test_inlineasm()
{
  int a, b, c, d, e;

  a = inlineasm_addu(); // 25
  b = inlineasm_arg(1, 10); // -9
  c = inlineasm_arg(6, 3); // 3
  __asm__ __volatile__("addiu %0,%1,1"
                       :"=r"(d) // d=4
                       :"r"(c)
                       );
  e = inlineasm_global(); // 4

  return (a+b+c+d+e); // 25-9+3+4+4=27
}

