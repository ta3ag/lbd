// /usr/local/llvm/release/cmake_debug_build/bin/clang -target mips-unknown-linux-gnu -c hello.cpp -emit-llvm -o hello.bc
// /home/cschen/test/lld/cmake_debug_build/bin/llc -march=cpu0 -mcpu=cpu032I -relocation-model=static -filetype=obj ch_hello.bc -o ch_hello.cpu0.o

// start
extern int printf(const char *format, ...);

int main(void)
{
  char *ptr = "Hello world!";

  printf("%s\n", ptr);
}

