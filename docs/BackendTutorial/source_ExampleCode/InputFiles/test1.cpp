// /home/cschen/test/lld_20130816/cmake_debug_build/bin/clang -target mips-unknown-linux-gnu -c test1.cpp -emit-llvm -o test1.bc
// /home/cschen/test/lld_20130816/cmake_debug_build/bin/llc -march=cpu0 -relocation-model=static -filetype=obj test1.bc -o test1.cpu0.o
// /home/cschen/test/lld_20130816/cmake_debug_build/bin/lld -flavor gnu -target cpu0-unknown-linux-gnu test1.cpu0.o -o a.out
// /home/cschen/test/lld_20130816/cmake_debug_build/bin/llvm-objdump -elf2hex a.out > ../cpu0_verilog/raw/cpu0s.hex
/*
#include "boot.cpp"

#include "print.h"
*/
#if 0
#include "boot.cpp"

#include "print.h"


int test();

int main()
{
  int a = 0;
  a = test1();
  print_integer(a);    // a = 2

  return a;
}

#include "print.cpp"

#define PAD_RIGHT 0x00000001
#define PAD_ZERO 0x00000002

int test1()
{
  int sum = 0;
  int pad = (PAD_ZERO | PAD_RIGHT);
  if (!(pad & PAD_RIGHT)) {
  // (pad & PAD_RIGHT) == 0
    sum = 1;
  }
  else {
    sum = 2;
  }
  return sum; 
}

int test2(const char *format)
{
	register int width;

	for ( ; (*format >= '0' && *format <= '9'); ++format) {
		width *= 10;
		width += *format - '0';
	}

  return 0; 
}
#endif

int test1(int bs)
{
//	unsigned int bs = sizeof(int)*8;
	int mi;
	mi = (1 << (bs-1)) + 1;
//	mi = (bs << 14) + 1;
  return mi;
}
