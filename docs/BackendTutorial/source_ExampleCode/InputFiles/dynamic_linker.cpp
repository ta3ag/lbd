// /usr/local/llvm/release/cmake_debug_build/bin/clang -target mips-unknown-linux-gnu -c dynamic_linker.cpp -emit-llvm -o dynamic_linker.bc
// /usr/local/llvm/release/cmake_debug_build/bin/llvm-dis dynamic_linker.bc -o dynamic_linker.ll

/// start

#define DYNLINKER_INFO_ADDR  0x70000
#define DYNENT_SIZE          8
#define FLASHADDR            0xA0000
#define DYNPROGSTART         0x40000
#define PLT_1ST_ADDR         0x20

extern "C" int printf(const char *format, ...);

struct ProgAddr {
  int id;
  int memAddr;
  int size;
};

int progCounter; // program counter, init to 0 in main()

void dynamic_linker()
{
  asm("st $gp, 40($fp)"); // store $gp to rgp, rgp point to the start address 
                          // of section .got.plt
  asm("st $7, 36($fp)"); // store $7 to r7, r7 is the caller next instruction 
                         // address
  asm("st $8, 32($fp)"); // store $8 to r8, r8 is dynamic entry offset

  static ProgAddr prog[10];
  int nextFreeAddr;
  int *src, *dest, *end;
  int numDynEntry = 0;
  int dynsym = 0;
  char *dynstr = 0;
  int libOffset = 0;
  int nextFunLibOffset = 0;
  volatile int memAddr = 0;
  volatile int rgp, r7, r8;

  printf("r7 = %d, r8 = %d\n", r7, r8);
  numDynEntry = *((int*)(DYNLINKER_INFO_ADDR));
  dynsym = *((int*)((DYNLINKER_INFO_ADDR+8)+(r8/16)*DYNENT_SIZE));
  dynstr = (char*)(DYNLINKER_INFO_ADDR+8+numDynEntry*8+numDynEntry*52+dynsym);
  libOffset = *((int*)(DYNLINKER_INFO_ADDR+8+numDynEntry*8+dynsym*52));
  nextFunLibOffset = *((int*)(DYNLINKER_INFO_ADDR+8+numDynEntry*8+(dynsym+1)*52));
  printf("Number of numDynEntry = %d, dynsym = %d, dstr = %s, libOffset = %d, \
         nextFunLibOffset = %d, progCounter = %d\n", numDynEntry, dynsym, dynstr, 
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

  // Change .got.plt for "ld	$t9, idx($gp)"
  *((int*)(rgp+r8)) = prog[progCounter].memAddr;
  memAddr = prog[progCounter].memAddr;
  asm("ld $t9, 44($fp)"); // 36($fp) is address of variable memAddr
  asm("addiu $sp, $sp, 104"); // 36($sp) is address for variable memAddr
  asm("ret $t9");

  return;
}

