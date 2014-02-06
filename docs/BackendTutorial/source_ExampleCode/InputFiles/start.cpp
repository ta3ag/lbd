
/// start

#include "Cpu0Config.h"
#include "print.h"
#include "dynamic_linker.h"
#include "start.h"

extern int main();

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

