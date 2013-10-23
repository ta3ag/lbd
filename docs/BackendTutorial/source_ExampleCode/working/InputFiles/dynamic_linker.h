#ifndef _DYNAMIC_LINKER_H_
#define _DYNAMIC_LINKER_H_

#define DYNLINKER_INFO_ADDR  0x70000
#define DYNENT_SIZE          8
#define DYNPROGSTART         0x40000
#define FLASHADDR            0xA0000
#define GPADDR               0x7FFF0

struct ProgAddr {
  int memAddr;
  int size;
};

extern void setGotPltSection();
extern void dynamic_linker();

#endif
