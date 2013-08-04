// clang -target `llvm-config --host-target` -c ch_hello.cpp -emit-llvm -o ch_hello.bc
// /usr/local/llvm/test/cmake_debug_build/bin/llc -march=cpu0 -relocation-model=static -filetype=obj ch_hello.bc -o ch_hello.cpu0.o
// /usr/local/llvm/test/cmake_debug_build/bin/llvm-objdump -d ch_hello.cpu0.o | tail -n +6| awk '{print "/* " $1 " */\t" $2 " " $3 " " $4 " " $5 "\t/* " $6"\t" $7" " $8" " $9" " $10 "\t*/"}' > ../cpu0_verilog/raw/cpu0s.hex

#include <stdarg.h>

void print_char(char c);
void print_string(char *str);
char *itoa(int i);

#define OUT_MEM 0x7000 // 28672

// boot:
  asm("jmp 12"); // RESET: jmp RESET_START;
  asm("jmp 4");  // ERROR: jmp ERR_HANDLE;
  asm("jmp 4");  // IRQ: jmp IRQ_HANDLE;
  asm("jmp -4"); // ERR_HANDLE: jmp ERR_HANDLE; (loop forever)

  // RESET_START:
  #include "InitRegs.h"
  
  asm("addiu $sp, $zero, 0x6ffc");
  asm("addiu $3, $ZERO, 0x50");
  asm("iret $3");

int main()
{
  int a = 0;

//  char str[81] = "Hello world";
  print_string((char*)&("Hello world"));
//  print_string(str);
/*  print_char('H');
  print_char('e');
  print_char('l');
  print_char('l');
  print_char('o');
  print_char(' ');
  print_char('w');
  print_char('o');
  print_char('r');
//  print_char('l');
//  print_char('d');
  print_char('\0');*/

  return a;
}

// For memory IO
void print_char(char c)
{
  char *p = (char*)OUT_MEM;
  *p = c;

  return;
}

void print_string(char *str)
{
  char *p;

  for (p = str; *p != '\0'; p++)
    print_char(*p);
  print_char(*p);

  return;
}

#include "itoa.cpp"

