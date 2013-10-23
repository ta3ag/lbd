// /usr/local/llvm/release/cmake_debug_build/bin/clang -target mips-unknown-linux-gnu -c dynamic_linker.cpp -emit-llvm -o dynamic_linker.bc
// /usr/local/llvm/release/cmake_debug_build/bin/llvm-dis dynamic_linker.bc -o dynamic_linker.ll

/// start

#include "dynamic_linker.h"

#define DEBUG_DLINKER
#define PLT0ADDR 0x10

#define STOP \
  asm("lui $t9, 0xffff"); \
  asm("addiu $t9, $zero, 0xffff"); \
  asm("ret $t9");

#define DEBUG \
  asm("ori $sw, $sw, 0x0004");

extern "C" int printf(const char *format, ...);

int got_plt_fill[0x20] = {
1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1
};

int progCounter; // program counter, init to 0 in main()

ProgAddr prog[10];

void setGotPltSection()
{
  progCounter = 0;
  int numDynEntry = 0;
  int gp = *(int*)GPADDR;
  numDynEntry = *((int*)(DYNLINKER_INFO_ADDR));
//  for (int i = 1; i < numDynEntry; i++) {
  for (int i = 1; i <= numDynEntry; i++) {
    // Skip the first .got.plt entry which is for cpu0Plt0AtomContent.
    // The offset 0x20, 0x30 of section .got.plt. which is for 
    // cpu0PltAtomContent is set to 0x10
    *(int*)(gp+0x10+i*0x10) = PLT0ADDR;
  }
}

void dynamic_linker()
{
//  static ProgAddr prog[10]; // has side effect (ProgAddr cannot be written in 
// Virtual Box on iMac).
  int nextFreeAddr;
  int *src, *dest, *end;
  int numDynEntry = 0;
  int dynsym_idx = 0;
  int dynsym = 0;
  char *dynstr = 0;
  int libOffset = 0;
  int nextFunLibOffset = 0;
  volatile int memAddr = 0;
  numDynEntry = *((int*)(DYNLINKER_INFO_ADDR));
  int gp = *(int*)GPADDR;
#ifdef DEBUG_DLINKER
  printf("gp = %d\n", gp);
#endif
  dynsym_idx = *(int*)gp;
#ifdef DEBUG_DLINKER
  printf("numDynEntry = %d, dynsym_idx = %d\n", numDynEntry, dynsym_idx);
#endif
  dynsym = *(int*)((DYNLINKER_INFO_ADDR+8)+(dynsym_idx*DYNENT_SIZE));
  dynstr = (char*)(DYNLINKER_INFO_ADDR+8+numDynEntry*8+numDynEntry*52+dynsym);
  libOffset = *((int*)(DYNLINKER_INFO_ADDR+8+numDynEntry*8+(dynsym_idx-1)*52));
  nextFunLibOffset = *((int*)(DYNLINKER_INFO_ADDR+8+numDynEntry*8+dynsym_idx*52));
#ifdef DEBUG_DLINKER
  printf("dstr = %x, dynsym = %d, *dstr = %s\n", 
         (int)dynstr, dynsym, dynstr);
  printf("libOffset = %d, nextFunLibOffset = %d, progCounter = %d\n", 
         libOffset, nextFunLibOffset, progCounter);
#endif
  if (progCounter == 0)
     nextFreeAddr = DYNPROGSTART;
  else
     nextFreeAddr = prog[progCounter-1].memAddr+prog[progCounter-1].size;
  prog[progCounter].memAddr = nextFreeAddr;
  prog[progCounter].size = (nextFunLibOffset - libOffset);

#ifdef DEBUG_DLINKER
  printf("prog[progCounter].memAddr = %d, prog[progCounter].size = %d\n", 
  prog[progCounter].memAddr, (unsigned int)(prog[progCounter].size));
#endif
  // Load program from (FLASHADDR+libOffset..FLASHADDR+nextFunLibOffset-1) to
  // (nextFreeAddr..nextFreeAddr+prog[progCounter].size-1)
  src = (int*)(FLASHADDR+libOffset);
  end = (int*)(src+prog[progCounter].size/4);
#ifdef DEBUG_DLINKER
  printf("end = %x, src = %x, nextFreeAddr = %x\n", 
         (unsigned int)end, (unsigned int)src, (unsigned int)nextFreeAddr);
  printf("*src = %x\n", (unsigned int)(*src));
#endif
  for (dest = (int*)(prog[progCounter].memAddr); src < end; src++, dest++) {
    *dest = *src;
#ifdef DEBUG_DLINKER
    printf("*dest = %08x\n", (unsigned int)(*dest));
#endif
  }
  progCounter++;

#ifdef DEBUG_DLINKER
  printf("progCounter-1 = %x, prog[progCounter-1].memAddr = %x, \
         *prog[progCounter-1].memAddr = %x\n", 
    (unsigned int)(progCounter-1), (unsigned int)(prog[progCounter-1].memAddr), 
    *(unsigned int*)(prog[progCounter-1].memAddr));
#endif
  // Change .got.plt for "ld	$t9, idx($gp)"
  *((int*)(gp+0x10+dynsym_idx*0x10)) = prog[progCounter-1].memAddr;
  *(int*)(0x7FFE0) = prog[progCounter-1].memAddr;
#ifdef DEBUG_DLINKER
  printf("*((int*)(gp+0x10+dynsym_idx*0x10)) = %x, *(int*)(0x7FFE0) = %x\n", 
         *((int*)(gp+0x10+dynsym_idx*0x10)), (unsigned int)(*(int*)(0x7FFE0)));
#endif
//  STOP;
  DEBUG;
  // restore $lr. The next instruction of foo() of main.cpp for the main.cpp
  // call foo() first time example.
  // The $lr, $fp and $sp saved in cpu0Plt0AtomContent of Cpu0LinkingContext.cpp.
  asm("ld $lr, 4($gp)"); // restore $lr
  asm("ld $fp, 8($gp)"); // restore $fp
  asm("ld $sp, 12($gp)"); // restore $sp
  // jmp to the dynamic linked function. It's foo() for the main.cpp call foo() 
  // first time example.
  asm("lui $t9, 0x7");
  asm("ori $t9, $t9, 0xFFE0");
  asm("ld $t9, 0($t9)");
  asm("ret $t9");

  return;
}

