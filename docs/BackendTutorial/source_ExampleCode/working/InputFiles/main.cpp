// /usr/local/llvm/release/cmake_debug_build/bin/clang -c -fPIC foobar.cpp
// /home/Gamma/test/lld/cmake_debug_build/bin/lld -flavor gnu -target x86_64-unknown-linux-gnu -shared -o libfoobar.so foobar.o
// /usr/local/llvm/release/cmake_debug_build/bin/clang -target x86_64-unknown-linux-gnu -c x86_64_start.ll -o x86_64_start.o
// /usr/local/llvm/release/cmake_debug_build/bin/clang -target x86_64-unknown-linux-gnu -c main.cpp -o main.o
// /home/Gamma/test/lld/cmake_debug_build/bin/lld -flavor gnu -target x86_64-unknown-linux-gnu x86_64_start.o main.o libfoobar.so

// /usr/local/llvm/release/cmake_debug_build/bin/clang -target mips-unknown-linux-gnu -c dynamic_linker.cpp -emit-llvm -o dynamic_linker.cpu0.bc
// /usr/local/llvm/release/cmake_debug_build/bin/clang -target mips-unknown-linux-gnu -c printf-stdarg.c -emit-llvm -o printf-stdarg.bc
// /usr/local/llvm/release/cmake_debug_build/bin/clang -target mips-unknown-linux-gnu -c foobar.cpp -emit-llvm -o foobar.cpu0.bc
// /home/cschen/test/lld/cmake_debug_build/bin/llc -march=cpu0 -relocation-model=static -filetype=obj -cpu0-fix-global-base-register=true dynamic_linker.cpu0.bc -o dynamic_linker.cpu0.o
// /home/cschen/test/lld/cmake_debug_build/bin/llc -march=cpu0 -relocation-model=static -filetype=obj -cpu0-fix-global-base-register=true printf-stdarg.bc -o printf-stdarg.cpu0.o
// /home/cschen/test/lld/cmake_debug_build/bin/llc -march=cpu0 -relocation-model=pic -filetype=obj -cpu0-fix-global-base-register=true foobar.cpu0.bc -o foobar.cpu0.o
// /home/cschen/test/lld/cmake_debug_build/bin/lld -flavor gnu -target cpu0-unknown-linux-gnu -shared -o libfoobar.cpu0.so foobar.cpu0.o
// /home/cschen/test/lld/cmake_debug_build/bin/llc -march=cpu0 -relocation-model=static -filetype=obj -cpu0-fix-global-base-register=true start.ll -o start.cpu0.o
// /usr/local/llvm/release/cmake_debug_build/bin/clang -target mips-unknown-linux-gnu -c main.cpp -emit-llvm -o main.cpu0.bc
// /home/cschen/test/lld/cmake_debug_build/bin/llc -march=cpu0 -relocation-model=static -filetype=obj -cpu0-fix-global-base-register=true main.cpu0.bc -o main.cpu0.o
// /home/cschen/test/lld/cmake_debug_build/bin/lld -flavor gnu -target cpu0-unknown-linux-gnu start.cpu0.o printf-stdarg.cpu0.o dynamic_linker.cpu0.o main.cpu0.o libfoobar.cpu0.so
// /home/cschen/test/lld/cmake_debug_build/bin/llvm-objdump -elf2hex a.out > ../cpu0_verilog/raw/cpu0s.hex
// /home/cschen/test/lld/cmake_debug_build/bin/llvm-objdump -elf2hex -dumpso libfoobar.cpu0.so > ../cpu0_verilog/raw/libso.hex
// cp dynstr dynsym so_func_offset global_offset ../cpu0_verilog/raw/.

// /usr/local/llvm/release/cmake_debug_build/bin/clang -target mips-unknown-linux-gnu -c dynamic_linker.cpp -emit-llvm -o dynamic_linker.cpu0.bc
// /usr/local/llvm/release/cmake_debug_build/bin/clang -target mips-unknown-linux-gnu -c printf-stdarg.c -emit-llvm -o printf-stdarg.bc
// /usr/local/llvm/release/cmake_debug_build/bin/clang -target mips-unknown-linux-gnu -c foobar.cpp -emit-llvm -o foobar.cpu0.bc
// /home/Gamma/test/lld/cmake_debug_build/bin/llc -march=cpu0 -relocation-model=static -filetype=obj -cpu0-fix-global-base-register=true dynamic_linker.cpu0.bc -o dynamic_linker.cpu0.o
// /home/Gamma/test/lld/cmake_debug_build/bin/llc -march=cpu0 -relocation-model=static -filetype=obj -cpu0-fix-global-base-register=true printf-stdarg.bc -o printf-stdarg.cpu0.o
// /home/Gamma/test/lld/cmake_debug_build/bin/llc -march=cpu0 -relocation-model=pic -filetype=obj -cpu0-fix-global-base-register=true foobar.cpu0.bc -o foobar.cpu0.o
// /home/Gamma/test/lld/cmake_debug_build/bin/lld -flavor gnu -target cpu0-unknown-linux-gnu -shared -o libfoobar.cpu0.so foobar.cpu0.o
// /home/Gamma/test/lld/cmake_debug_build/bin/llc -march=cpu0 -relocation-model=static -filetype=obj -cpu0-fix-global-base-register=true start.ll -o start.cpu0.o
// /usr/local/llvm/release/cmake_debug_build/bin/clang -target mips-unknown-linux-gnu -c main.cpp -emit-llvm -o main.cpu0.bc
// /home/Gamma/test/lld/cmake_debug_build/bin/llc -march=cpu0 -relocation-model=static -filetype=obj -cpu0-fix-global-base-register=true main.cpu0.bc -o main.cpu0.o
// /home/Gamma/test/lld/cmake_debug_build/bin/lld -flavor gnu -target cpu0-unknown-linux-gnu start.cpu0.o printf-stdarg.cpu0.o dynamic_linker.cpu0.o main.cpu0.o libfoobar.cpu0.so
// /home/Gamma/test/lld/cmake_debug_build/bin/llvm-objdump -elf2hex a.out > ../cpu0_verilog/raw/cpu0s.hex
// /home/Gamma/test/lld/cmake_debug_build/bin/llvm-objdump -elf2hex -dumpso libfoobar.cpu0.so > ../cpu0_verilog/raw/libso.hex
// cp dynstr dynsym num_dyn_entry so_func_offset global_offset ../cpu0_verilog/raw/.

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

extern int foo(int x1, int x2);
extern int bar();

int gI = 100;

int main()
{
  int a = 0;
  setGotPltSection();
  a = foo(1, 2);
  printf("foo(1, 2) = %d\n", a); 
/*  a = foo(3, 4);
  printf("foo(3, 4) = %d\n", a); */
  a = bar();
  printf("bar() = %d\n", a);
  
  return 0;
}
