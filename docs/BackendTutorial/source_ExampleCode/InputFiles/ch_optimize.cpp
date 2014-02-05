// bash build-optimize.sh

/// start

#include "debug.h"
#include "boot.cpp"

#include "print.h"

int test_select(int a, int b);

int main()
{
  int a = test_select(1, 2); // 4
  print_integer(a);  // a = 4

  return 0;
}

#include "print.cpp"

#include "ch12_4.cpp"

