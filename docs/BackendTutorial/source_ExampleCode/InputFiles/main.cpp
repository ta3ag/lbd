// gcc/clang -c -fPIC foobar.cpp
// gcc/clang -shared -o libfoobar.so foobar.o
// /home/Gamma/test/lld/cmake_debug_build/bin/lld -flavor gnu -target x86_64-unknown-linux-gnu -shared -o libfoobar.so foobar.o
// gcc/clang main.cpp libfoobar.so

/// start
extern int foo(int x1, int x2);
extern int bar();

int main()
{
  int a = foo(1, 2);
  a += foo(3, 4);
  a += bar();
  
  return a;
}
