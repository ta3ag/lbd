#ifndef _DYNAMIC_LINKER_H_
#define _DYNAMIC_LINKER_H_

#define DYNLINKER_INFO_ADDR  0x70000
#define DYNENT_SIZE          4
#define DYNPROGSTART         0x40000
#define FLASHADDR            0xA0000
#define GPADDR               0x7FFF0

#define STOP \
  asm("lui $t9, 0xffff"); \
  asm("addiu $t9, $zero, 0xffff"); \
  asm("ret $t9");

#define ENABLE_TRACE \
  asm("mfsw $at"); \
  asm("ori $at, $at, 0x0004"); \
  asm("mtsw $at");

#define DISABLE_TRACE \
  asm("mfsw $at"); \
  asm("andi $at, $at, 0xfffb"); \
  asm("mtsw $at");

struct ProgAddr {
  int memAddr;
  int size;
};

extern void dynamic_linker_init();
extern void dynamic_linker();

#endif
