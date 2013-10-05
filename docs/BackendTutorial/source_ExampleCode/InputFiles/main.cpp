// /usr/local/llvm/release/cmake_debug_build/bin/clang -c -fPIC foobar.cpp
// /usr/local/llvm/release/cmake_debug_build/bin/clang -shared -o libfoobar.so foobar.o
// /home/Gamma/test/lld/cmake_debug_build/bin/lld -flavor gnu -target x86_64-unknown-linux-gnu -shared -o libfoobar.so foobar.o
// /usr/local/llvm/release/cmake_debug_build/bin/clang main.cpp libfoobar.so
// 
// /usr/local/llvm/release/cmake_debug_build/bin/clang -target x86_64-unknown-linux-gnu -c x86_64_start.ll -o x86_64_start.o
// /usr/local/llvm/release/cmake_debug_build/bin/clang -target x86_64-unknown-linux-gnu -c main.cpp -o main.o
// /home/Gamma/test/lld/cmake_debug_build/bin/lld -flavor gnu -target x86_64-unknown-linux-gnu x86_64_start.o main.o libfoobar.so


// /usr/local/llvm/release/cmake_debug_build/bin/clang -target mips-unknown-linux-gnu -c foobar.cpp -emit-llvm -o foobar.bc
// /home/Gamma/test/lld/cmake_debug_build/bin/llc -march=cpu0 -relocation-model=pic -filetype=obj foobar.bc -o foobar.o
// /home/Gamma/test/lld/cmake_debug_build/bin/lld -flavor gnu -target cpu0-unknown-linux-gnu -shared -o libfoobar.so foobar.o
// /home/Gamma/test/lld/cmake_debug_build/bin/llc -march=cpu0 -relocation-model=pic -filetype=obj start.ll -o start.o
// /usr/local/llvm/release/cmake_debug_build/bin/clang -target mips-unknown-linux-gnu -c main.cpp -emit-llvm -o main.bc
// /home/Gamma/test/lld/cmake_debug_build/bin/llc -march=cpu0 -relocation-model=pic -filetype=obj main.bc -o main.o
// /home/Gamma/test/lld/cmake_debug_build/bin/lld -flavor gnu -target cpu0-unknown-linux-gnu start.o main.o libfoobar.so

/// start
extern int foo(int x1, int x2);
extern int bar();

//#include <stdio.h>

int main()
{
  int a = foo(1, 2);
//  printf("foo(1, 2) = %d\n", a); 
  a += foo(3, 4);
  a += bar();
  
  return a;
}
