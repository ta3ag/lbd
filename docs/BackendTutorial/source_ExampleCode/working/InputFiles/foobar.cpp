
/// start

#include "dynamic_linker.h"

int foo(int x1, int x2)
{
  int sum = x1 + x2;
  
  return sum; 
}

int foo1(int x1, int x2)
{
  int sum = x1 + x2;
  
  return sum; 
}

int bar()
{
  int a;
  ENABLE_TRACE;
  asm("ori $fp, $fp, 0x0000");
  asm("ori $sp, $sp, 0x0000");
//  DISABLE_TRACE;
  a = foo(2, 2);
  ENABLE_TRACE;
  asm("ori $fp, $fp, 0x0000");
  asm("ori $sp, $sp, 0x0000");
  DISABLE_TRACE;
//  a = foo1(2, 2);

  return a;
}

