
#include "itoa.cpp"

// For memory IO
void print_char(const char c)
{
  char *p = (char*)OUT_MEM;
  *p = c;

  return;
}
#if 0
void dump_mem(char *str, int n)
{
  int i = 0;
  const char *p;

  for (i = 0, p = str; i < n; i++) {
    char x = (char)(*p >> 4);
    if (x <= 9)
      print_char(x+'0');
    else
      print_char(x+'A');
  }
  print_char('\n');

  return;
}
#endif
void print_string(const char *str)
{
  const char *p;

  for (p = str; *p != '\0'; p++)
    print_char(*p);
  print_char(*p);
  print_char('\n');

  return;
}

// For memory IO
void print_integer(int x)
{
#if 0
  int *p = (int*)OUT_MEM;
  *p = x;
#endif

  char str[INT_DIGITS + 2];
  itoa(str, x);
  print_string(str);

  return;
}

