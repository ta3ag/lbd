#ifndef _DEBUG_H_
#define _DEBUG_H_

#define STOP \
  asm("lui $t9, 0xffff"); \
  asm("addiu $t9, $zero, 0xffff"); \
  asm("ret $t9");

#define ENABLE_TRACE \
  asm("mfsw $at"); \
  asm("ori $at, $at, 0x0020"); \
  asm("mtsw $at");

#define DISABLE_TRACE \
  asm("mfsw $at"); \
  asm("andi $at, $at, 0xffdf"); \
  asm("mtsw $at");

#endif

