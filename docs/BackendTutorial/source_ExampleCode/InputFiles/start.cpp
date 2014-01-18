
/// start

#include "print.h"
#include "dynamic_linker.h"

extern int main();

#define initRegs() \
  asm("addiu $1,  $ZERO, 0"); \
  asm("addiu $2,  $ZERO, 0"); \
  asm("addiu $3,  $ZERO, 0"); \
  asm("addiu $4,  $ZERO, 0"); \
  asm("addiu $5,  $ZERO, 0"); \
  asm("addiu $6,  $ZERO, 0"); \
  asm("addiu $7,  $ZERO, 0"); \
  asm("addiu $8,  $ZERO, 0"); \
  asm("addiu $9,  $ZERO, 0"); \
  asm("addiu $10, $ZERO, 0"); \
  asm("addiu $fp, $ZERO, 0");


// Real entry (first instruction) is from cpu0BootAtomContent of 
// Cpu0RelocationPass.cpp jump to asm("start:") of start.cpp.
void start() {
  asm("start:");
  
  asm("lui $sp, 0x6");
  asm("addiu $sp, $sp, 0xfffc");
  int *gpaddr;
  gpaddr = (int*)GPADDR;
  __asm__ __volatile__("ld  $gp, %0"
                       : // no output register, specify output register to $gp
                       :"m"(*gpaddr)
                       );
  initRegs();
  main();
  asm("addiu $lr, $ZERO, -1");
  asm("ret $lr");
}

// For memory IO
extern "C" int putchar(int c)
{
  char *p = (char*)OUT_MEM;
  *p = c;

  return 0;
}

