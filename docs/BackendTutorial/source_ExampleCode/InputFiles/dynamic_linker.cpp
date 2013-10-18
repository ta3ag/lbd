// /usr/local/llvm/release/cmake_debug_build/bin/clang -target mips-unknown-linux-gnu -c dynamic_linker.cpp -emit-llvm -o dynamic_linker.bc
// /usr/local/llvm/release/cmake_debug_build/bin/llvm-dis dynamic_linker.bc -o dynamic_linker.ll

/// start

#include "dynamic_linker.h"

extern "C" int printf(const char *format, ...);

int progCounter; // program counter, init to 0 in main()

void dynamic_linker()
{
#if 0
  printf("dynamic_linker()\n");
#else
  static ProgAddr prog[10];
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
  libOffset = *((int*)(DYNLINKER_INFO_ADDR+8+numDynEntry*8+dynsym*52));
  nextFunLibOffset = *((int*)(DYNLINKER_INFO_ADDR+8+numDynEntry*8+(dynsym+1)*52));
  printf("Number of numDynEntry = %d, dstr = %x, dynsym = %d, *dstr = %s\n", numDynEntry, (int)dynstr, dynsym, dynstr);
  printf("libOffset = %d, nextFunLibOffset = %d, progCounter = %d\n", 
         libOffset, nextFunLibOffset, progCounter);
  if (progCounter == 0)
     nextFreeAddr = DYNPROGSTART;
  else
     nextFreeAddr = prog[progCounter-1].memAddr+prog[progCounter-1].size;
  prog[progCounter].memAddr = nextFreeAddr;
  prog[progCounter].size = (nextFunLibOffset - libOffset);

  // Load program from (FLASHADDR+libOffset..FLASHADDR+nextFunLibOffset-1) to
  // (nextFreeAddr..nextFreeAddr+prog[progCounter].size-1)
  end = (int*)(prog[progCounter].memAddr+prog[progCounter].size);
  src = (int*)(FLASHADDR+libOffset);
  printf("end = %x, src = %x, nextFreeAddr = %x\n", (unsigned int)end, (unsigned int)src, (unsigned int)nextFreeAddr);
  for (dest = (int*)(prog[progCounter].memAddr); src < end; src++, dest++)
    *dest = *src;
  progCounter++;

  printf("progCounter-1 = %x, prog[progCounter-1].memAddr = %x, *prog[progCounter-1].memAddr = %x\n", (unsigned int)(progCounter-1), (unsigned int)(prog[progCounter-1].memAddr), *(unsigned int*)(prog[progCounter-1].memAddr));
  // Change .got.plt for "ld	$t9, idx($gp)"
  *((int*)(GPADDR+0x18+dynsym*0x10)) = prog[progCounter].memAddr;
  memAddr = prog[progCounter].memAddr;
  asm("ld $t9, 44($fp)"); // 36($fp) is address of variable memAddr
  asm("addiu $sp, $sp, 104"); // 36($sp) is address for variable memAddr
  asm("ret $t9");
#endif
  return;
}

