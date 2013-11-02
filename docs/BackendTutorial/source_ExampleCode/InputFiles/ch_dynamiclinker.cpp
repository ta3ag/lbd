// /usr/local/llvm/release/cmake_debug_build/bin/clang -c -fPIC foobar.cpp
// /home/Gamma/test/lld/cmake_debug_build/bin/lld -flavor gnu -target x86_64-unknown-linux-gnu -shared -o libfoobar.so foobar.o
// /usr/local/llvm/release/cmake_debug_build/bin/clang -target x86_64-unknown-linux-gnu -c x86_64_start.ll -o x86_64_start.o
// /usr/local/llvm/release/cmake_debug_build/bin/clang -target x86_64-unknown-linux-gnu -c main.cpp -o main.o
// /home/Gamma/test/lld/cmake_debug_build/bin/lld -flavor gnu -target x86_64-unknown-linux-gnu x86_64_start.o main.o libfoobar.so

/// start
#include "dynamic_linker.h"
#include "print.h"

extern "C" int printf(const char *format, ...);

// For memory IO
extern "C" int putchar(const char c)
{
  char *p = (char*)OUT_MEM;
  *p = c;

  return 0;
}

extern int la(int x1, int x2);
extern int foo(int x1, int x2);
extern int bar();
extern int h(int x1, int x2);

int main()
{
//  ENABLE_TRACE;
  int a = 0;

  dynamic_linker_init();
#if 0
  a = la(1, 2);
  printf("la(1, 2) = %d\n", a);
#endif
#if 0
  a = foo(1, 2);
  printf("foo(1, 2) = %d\n", a);
#endif
#if 1
  a = bar();
  printf("bar() = %d\n", a);
#endif
  
  return 0;
}
