// /usr/local/llvm/release/cmake_debug_build/bin/clang -c -fPIC foobar.cpp
// /home/Gamma/test/lld/cmake_debug_build/bin/lld -flavor gnu -target x86_64-unknown-linux-gnu -shared -o libfoobar.so foobar.o
// /usr/local/llvm/release/cmake_debug_build/bin/clang -target x86_64-unknown-linux-gnu -c x86_64_start.ll -o x86_64_start.o
// /usr/local/llvm/release/cmake_debug_build/bin/clang -target x86_64-unknown-linux-gnu -c main.cpp -o main.o
// /home/Gamma/test/lld/cmake_debug_build/bin/lld -flavor gnu -target x86_64-unknown-linux-gnu x86_64_start.o main.o libfoobar.so


// /usr/local/llvm/release/cmake_debug_build/bin/clang -target mips-unknown-linux-gnu -c foobar.cpp -emit-llvm -o foobar.cpu0.bc
// /home/Gamma/test/lld/cmake_debug_build/bin/llc -march=cpu0 -relocation-model=pic -filetype=obj foobar.cpu0.bc -o foobar.cpu0.o
// /home/Gamma/test/lld/cmake_debug_build/bin/lld -flavor gnu -target cpu0-unknown-linux-gnu -shared -o libfoobar.cpu0.so foobar.cpu0.o
// /home/Gamma/test/lld/cmake_debug_build/bin/llc -march=cpu0 -relocation-model=static -filetype=obj start.ll -o start.cpu0.o
// /usr/local/llvm/release/cmake_debug_build/bin/clang -target mips-unknown-linux-gnu -c main.cpp -emit-llvm -o main.cpu0.bc
// /home/Gamma/test/lld/cmake_debug_build/bin/llc -march=cpu0 -relocation-model=static -filetype=obj main.cpu0.bc -o main.cpu0.o
// /home/Gamma/test/lld/cmake_debug_build/bin/lld -flavor gnu -target cpu0-unknown-linux-gnu start.cpu0.o main.cpu0.o libfoobar.cpu0.so

/// start
extern int foo(int x1, int x2);
extern int bar();

//#include <stdio.h>

int gI = 100;

int main()
{
  int b = gI;
  int a = foo(1, 2);
//  printf("foo(1, 2) = %d\n", a); 
  a += foo(3, 4);
  a += bar();
  
  return a;
}
