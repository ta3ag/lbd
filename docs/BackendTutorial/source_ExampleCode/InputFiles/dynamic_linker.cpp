// /usr/local/llvm/release/cmake_debug_build/bin/clang -target mips-unknown-linux-gnu -c dynamic_linker.cpp -emit-llvm -o dynamic_linker.bc
// /usr/local/llvm/release/cmake_debug_build/bin/llvm-dis dynamic_linker.bc -o dynamic_linker.ll

/// start

#include "dynamic_linker.h"

extern "C" int printf(const char *format, ...);

int progCounter; // program counter, init to 0 in main()

ProgAddr prog[10];

void dynamic_linker()
{
#if 0
  printf("dynamic_linker()\n");
#else
  asm("lui $at, 0x7");
  asm("ori $at, $at, 0xFFD0");
  asm("st $lr, 0($at)");
  printf("*((int*)(0x7FFD0) = %08x\n", (unsigned int)(*(int*)(0x7FFD0)));
//  static ProgAddr prog[10]; // has side effect (ProgAddr cannot be written in Virtual Box on iMac.
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
  printf("gp = %d\n", gp);
  dynsym_idx = *(int*)gp;
  printf("dynsym_idx = %d\n", dynsym_idx);
  dynsym = *(int*)((DYNLINKER_INFO_ADDR+8)+(dynsym_idx*DYNENT_SIZE));
  dynstr = (char*)(DYNLINKER_INFO_ADDR+8+numDynEntry*8+numDynEntry*52+dynsym);
  libOffset = *((int*)(DYNLINKER_INFO_ADDR+8+numDynEntry*8+(dynsym-1)*52));
  nextFunLibOffset = *((int*)(DYNLINKER_INFO_ADDR+8+numDynEntry*8+dynsym*52));
  printf("Number of numDynEntry = %d, dstr = %x, dynsym = %d, *dstr = %s\n", numDynEntry, (int)dynstr, dynsym, dynstr);
  printf("libOffset = %d, nextFunLibOffset = %d, progCounter = %d\n", 
         libOffset, nextFunLibOffset, progCounter);
  if (progCounter == 0)
     nextFreeAddr = DYNPROGSTART;
  else
     nextFreeAddr = prog[progCounter-1].memAddr+prog[progCounter-1].size;
  printf("libOffset = %d, nextFunLibOffset = %d, progCounter = %d\n", 
         libOffset, nextFunLibOffset, progCounter);
  prog[progCounter].memAddr = nextFreeAddr;
  prog[progCounter].size = (nextFunLibOffset - libOffset);

  printf("prog[progCounter].memAddr = %d, prog[progCounter].size = %d\n", prog[progCounter].memAddr, (unsigned int)(prog[progCounter].size));
  // Load program from (FLASHADDR+libOffset..FLASHADDR+nextFunLibOffset-1) to
  // (nextFreeAddr..nextFreeAddr+prog[progCounter].size-1)
  src = (int*)(FLASHADDR+libOffset);
  end = (int*)(src+prog[progCounter].size/4);
  printf("end = %x, src = %x, nextFreeAddr = %x\n", (unsigned int)end, (unsigned int)src, (unsigned int)nextFreeAddr);
  printf("*src = %x\n", (unsigned int)(*src));
  for (dest = (int*)(prog[progCounter].memAddr); src < end; src++, dest++) {
    *dest = *src;
    printf("*dest = %08x\n", (unsigned int)(*dest));
  }
  progCounter++;

  printf("progCounter-1 = %x, prog[progCounter-1].memAddr = %x, *prog[progCounter-1].memAddr = %x\n", (unsigned int)(progCounter-1), (unsigned int)(prog[progCounter-1].memAddr), *(unsigned int*)(prog[progCounter-1].memAddr));
  // Change .got.plt for "ld	$t9, idx($gp)"
  *((int*)(gp+0x10+dynsym*0x10)) = prog[progCounter-1].memAddr;
  *(int*)(0x7FFE0) = prog[progCounter-1].memAddr;
  printf("*((int*)(gp+0x10+dynsym*0x10)) = %x, *(int*)(0x7FFE0) = %x\n", *((int*)(gp+0x10+dynsym*0x10)), (unsigned int)(*(int*)(0x7FFE0)));
  printf("*((int*)(0x7FFD0) = %08x\n", (unsigned int)(*(int*)(0x7FFD0)));
  // restore $lr. The next instruction of foo() of main.cpp for the main.cpp 
  // call foo() first time example.
  asm("lui $at, 0x7");
  asm("ori $at, $at, 0xFFD0");
  asm("ld $lr, 0($at)");
  
  asm("ld $fp, 96($sp)"); // restore $fp
  asm("addiu $sp, $sp, 104"); // restore $sp
  // jmp to the dynamic linked function. It's foo() for the main.cpp call foo() 
  // first time example.
  asm("lui $at, 0x7");
  asm("ori $at, $at, 0xFFE0");
  asm("ld $t9, 0($at)");
  asm("ret $t9");
#endif
  return;
}

