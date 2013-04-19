// clang -target `llvm-config --host-target` -c ch11_2.cpp -emit-llvm -o ch11_2.bc
// /Users/Jonathan/llvm/test/cmake_debug_build/bin/Debug/llc -march=cpu0 -relocation-model=static -filetype=asm -enable-cpu0-del-useless-jmp=false ch11_2.bc -o ch11_2.cpu0.s
// /Users/Jonathan/llvm/test/cmake_debug_build/bin/Debug/llc -march=cpu0 -relocation-model=static -filetype=asm -stats ch11_2.bc -o ch11_2.cpu0.s

// /Users/Jonathan/llvm/test/cmake_debug_build/bin/Debug/llc -march=cpu0 -relocation-model=static -filetype=obj -stats ch11_2.bc -o ch11_2.cpu0.o
// /Users/Jonathan/llvm/test/cmake_debug_build/bin/Debug/llvm-objdump -d ch11_2.cpu0.o | tail -n +6| awk '{print "/* " $1 " */\t" $2 " " $3 " " $4 " " $5 "\t/* " $6"\t" $7" " $8" " $9" " $10 "\t*/"}' > ../cpu0_verilog/redesign/cpu0s.hex

#include "InitRegs.h"

#define OUT_MEM 0x7000 // 28672

asm("addiu $sp, $zero, 0x6ffc");

void print_integer(int x);
int test_operators();
void test_operators_asm();
int test_control();

int main()
{
  int a = 0;
  a = test_operators(); // a = 13
  print_integer(a);
  a += test_control();	// a = 31
  print_integer(a);
  test_operators_asm();

  return a;
}

// For memory mapped I/O
void print_integer(int x)
{
  int *p = (int*)OUT_MEM;
  *p = x;
 return;
}

int test_operators()
{
  int a = 11;
  int b = 2;
  int c = 0;
  int d = 0;
  int e, f, g, h, i, j, k, l = 0;
  unsigned int a1 = -5, k1 = 0;
  unsigned int b1 = 0xf0000001;
  unsigned int c1 = 0x000fffff;

  a1 = b1 + c1;
  c = a + b;
//  c = 0x7fff0000 + 0x10000000;
  d = a - b;
  e = a * b;
  f = a / b;
  b = (a+1)%12;
  g = (a & b);
  h = (a | b);
  i = (a ^ b);
  j = (a << 2);
  k = (a >> 2);
  print_integer(k);
  k1 = (a1 >> 2);
  print_integer((int)k1);

  b = !a;
  int* p = &b;
  
  return c; // 13
}

void test_operators_asm()
{
  asm("addiu $sp, $sp, -12");
  asm("st $2, 8($sp)");
  asm("st $3, 4($sp)");
  asm("st $4, 0($sp)");
  asm("lui $2, 0x7fff");
  asm("lui $3, 0x1000");
  asm("addu $4, $2, $3");
  asm("lui $2, 0x7fff");
  asm("lui $3, 0x1000");
  asm("add $4, $2, $3");	// overflow
  asm("lui $2, 0x8fff");
  asm("lui $3, 0x7000");
  asm("sub $4, $2, $3");	// overflow
  asm("lui $2, 0x0");
  asm("addiu $3, $0, -1");
  asm("sub $4, $2, $3");	// $4=1, no overflow
  asm("lui $2, -1");
  asm("ori $2, $2, 0xffff");	// $2=0xffffffff=-1
  asm("andi $2, $2, 0xffff");	// $2=0x0000ffff
  asm("shl $2, $2, 16");	// $2=0xffff0000
  asm("xori $2, $2, 0xffff");	// $2=0xffffffff=-1
  asm("addiu $3, $0, -1");	// $3=0xffffffff=-1
  asm("divu $2, $3");		// HI=0, LO=1
  asm("div $2, $3");		// HI=0, LO=1, overflow
  asm("xori $2, $2, 1");	// $2 = 0xffffffffe
  asm("rol $4, $2, 4");		// $2 = 0xfffffffef
  asm("ror $4, $2, 8");		// $2 = 0xfefffffff
  asm("ld $2, 8($sp)");
  asm("ld $3, 4($sp)");
  asm("ld $4, 0($sp)");
  asm("addiu $sp, $sp, 12");
}

int test_control()
{
  int b = 1;
  int c = 2;
  int d = 3;
  int e = 4;
  int f = 5;
  
  if (b != 0) {
    b++;
  }
  if (c > 0) {
    c++;
  }
  if (d >= 0) {
    d++;
  }
  if (e < 0) {
    e++;
  }
  if (f <= 0) {
    f++;
  }
  
  return (b+c+d+e+f); // (2+3+4+4+5)=18
}

