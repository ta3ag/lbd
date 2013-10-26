
/// start

#include "dynamic_linker.h"

int foo(int x1, int x2)
{
  int sum = x1 + x2;
  
  return sum; 
}

int bar()
{
  int a;
  ENABLE_TRACE;
  a = foo(2, 2);

  return a;
}

