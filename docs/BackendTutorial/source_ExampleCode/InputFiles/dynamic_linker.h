#ifndef _DYNAMIC_LINKER_H_
#define _DYNAMIC_LINKER_H_

#define DYNLINKER_INFO_ADDR  0x70000
#define DYNENT_SIZE          8
#define DYNPROGSTART         0x40000
#define FLASHADDR            0xA0000
#define GPADDR               0x7FFF0

#define STOP \
  asm("lui $t9, 0xffff"); \
  asm("addiu $t9, $zero, 0xffff"); \
  asm("ret $t9");

#define ENABLE_TRACE \
  asm("ori $sw, $sw, 0x0004");

#define DISABLE_TRACE \
  asm("andi $sw, $sw, 0xfffb");

struct ProgAddr {
  int memAddr;
  int size;
};

extern void setGotPltSection();
extern void dynamic_linker();

#endif
