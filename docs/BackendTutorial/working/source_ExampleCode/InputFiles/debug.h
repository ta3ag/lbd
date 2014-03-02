#ifndef _DEBUG_H_
#define _DEBUG_H_

#include "Cpu0Config.h"

#define STOP \
  asm("lui $t9, 0xffff"); \
  asm("addiu $t9, $zero, 0xffff"); \
  asm("ret $t9");

#ifdef ASM_EASY_PORTING
  #define ENABLE_TRACE \
    asm("mfsw $at"); \
    asm("ori $at, $at, 0x0020"); \
    asm("mtsw $at");
#else
  #define ENABLE_TRACE \
    asm("ori $sw, $sw, 0x0020");
#endif

#ifdef ASM_EASY_PORTING
  #define DISABLE_TRACE \
    asm("mfsw $at"); \
    asm("andi $at, $at, 0xffdf"); \
    asm("mtsw $at");
#else
  #define DISABLE_TRACE \
    asm("andi $sw, $sw, 0xffdf");
#endif

#endif

