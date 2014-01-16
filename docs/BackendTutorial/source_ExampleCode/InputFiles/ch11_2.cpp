// clang -target mips-unknown-linux-gnu -c ch11_2.cpp -emit-llvm -o ch11_2.bc
// ~/llvm/test/cmake_debug_build/bin/Debug/llc -march=cpu0 -relocation-model=static -filetype=asm ch11_2.bc -o -
// /usr/local/llvm/test/cmake_debug_build/bin/llc -march=cpu0 -relocation-model=static -filetype=asm ch11_2.bc -o -

/// start
extern "C" int printf(const char *format, ...);
int inlineasm_addu(void)
{
  int foo = 10;
  const int bar = 15;

//  %3 = call i32 asm sideeffect "addu $0,$1,$2", "=r,r,r"(i32 %1, i32 %2) #1, !srcloc !1
  __asm__ __volatile__("addu %0,%1,%2"
                       :"=r"(foo) // 5
                       :"r"(foo), "r"(bar)
                       );

  return foo;
}

int inlineasm_addiu(void)
{
  int foo = 10;
  const int n_5 = -5;
  const int n5 = 5;
  const int n0 = 0;
  const unsigned int un5 = 5;
  const int n65536 = 0x10000;
  const int n_65531 = -65531;

//   %2 = call i32 asm sideeffect "addu $0,$1,$2", "=r,r,I"(i32 %1, i32 15) #1, !srcloc !2
  __asm__ __volatile__("addiu %0,%1,%2"
                       :"=r"(foo) // 15
                       :"r"(foo), "I"(n_5)
                       );

  __asm__ __volatile__("addiu %0,%1,%2"
                       :"=r"(foo) // 15
                       :"r"(foo), "J"(n0)
                       );

  __asm__ __volatile__("addiu %0,%1,%2"
                       :"=r"(foo) // 10
                       :"r"(foo), "K"(n5)
                       );

  __asm__ __volatile__("ori %0,%1,%2"
                       :"=r"(foo) // 65546
                       :"r"(foo), "L"(n65536) // 0x10000 = 65536
                       );

  __asm__ __volatile__("addiu %0,%1,%2"
                       :"=r"(foo) // 15
                       :"r"(foo), "N"(n_65531)
                       );

  __asm__ __volatile__("addiu %0,%1,%2"
                       :"=r"(foo) // 10
                       :"r"(foo), "O"(n_5)
                       );

  __asm__ __volatile__("addiu %0,%1,%2"
                       :"=r"(foo) // 15
                       :"r"(foo), "P"(un5)
                       );

  __asm__ __volatile__("addiu %0,%1,%2"
                       :"=r"(foo) // 10
                       :"r"(foo), "n"(n_5)
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
  int c, d;
  __asm__ __volatile__("ld %0,%1"
                       :"=r"(c) // c=3
                       :"m"(g[2])
                       );

  __asm__ __volatile__("addiu %0,%1,1"
                       :"=r"(d) // d=4
                       :"r"(c)
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
  int a, b, c, d, e, f;

  a = inlineasm_addu(); // 25
  printf("a=%d\n", a);
  b = inlineasm_addiu(); // -5
  printf("b=%d\n", b);
  c = inlineasm_arg(1, 10); // -9
  printf("c=%d\n", c);
  d = inlineasm_arg(6, 3); // 3
  printf("d=%d\n", d);
  __asm__ __volatile__("addiu %0,%1,1"
                       :"=r"(e) // e=4
                       :"r"(d)
                       );
  printf("e=%d\n", e);
  f = inlineasm_global(); // 4
  printf("f=%d\n", f);
  return (a+b+c+d+e+f); // 25+10-9+3+4+4=37
}

