// /usr/local/llvm/release/cmake_debug_build/bin/clang -target mips-unknown-linux-gnu -c dynamic_linker.cpp -emit-llvm -o dynamic_linker.bc
// /usr/local/llvm/release/cmake_debug_build/bin/llvm-dis dynamic_linker.bc -o dynamic_linker.ll

/// start

extern "C" int printf(const char *format, ...);

volatile void dynamic_linker()
{
  printf("dynamic_linker()\n");

  return;
}
