
/// start

#include "print.h"

// For memory IO
int putchar(int c)
{
  char *p = (char*)OUT_MEM;
  *p = c;

  return 0;
}

