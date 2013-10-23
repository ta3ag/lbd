// /usr/local/llvm/release/cmake_debug_build/bin/clang -target mips-unknown-linux-gnu -c printf-stdarg.c -emit-llvm -o printf-stdarg.bc
// /usr/local/llvm/release/cmake_debug_build/bin/clang -target mips-unknown-linux-gnu -c printf-stdarg-2.c -emit-llvm -o printf-stdarg-2.bc
// /home/cschen/test/lld_20130816/cmake_debug_build/bin/llc -march=cpu0 -relocation-model=static -filetype=obj printf-stdarg-2.bc -o printf-stdarg-2.cpu0.o
// /home/cschen/test/lld_20130816/cmake_debug_build/bin/llc -march=cpu0 -relocation-model=static -filetype=obj start.ll -o start.cpu0.o
// /home/cschen/test/lld_20130816/cmake_debug_build/bin/llc -march=cpu0 -relocation-model=static -filetype=obj printf-stdarg.bc -o printf-stdarg.cpu0.o
// /home/cschen/test/lld_20130816/cmake_debug_build/bin/lld -flavor gnu -target cpu0-unknown-linux-gnu start.cpu0.o printf-stdarg.cpu0.o printf-stdarg-2.cpu0.o -o a.out
// /home/cschen/test/lld_20130816/cmake_debug_build/bin/llvm-objdump -elf2hex a.out > ../cpu0_verilog/raw/cpu0s.hex

// /usr/local/llvm/release/cmake_debug_build/bin/clang -target mips-unknown-linux-gnu -c printf-stdarg.c -emit-llvm -o printf-stdarg.bc
// /usr/local/llvm/release/cmake_debug_build/bin/clang -target mips-unknown-linux-gnu -c printf-stdarg-2.c -emit-llvm -o printf-stdarg-2.bc
// /home/Gamma/test/lld/cmake_debug_build/bin/llc -march=cpu0 -relocation-model=static -filetype=obj start.ll -o start.cpu0.o
// /home/Gamma/test/lld/cmake_debug_build/bin/llc -march=cpu0 -relocation-model=static -filetype=obj printf-stdarg.bc -o printf-stdarg.cpu0.o
// /home/Gamma/test/lld/cmake_debug_build/bin/llc -march=cpu0 -relocation-model=static -filetype=obj printf-stdarg-2.bc -o printf-stdarg-2.cpu0.o
// /home/Gamma/test/lld/cmake_debug_build/bin/lld -flavor gnu -target cpu0-unknown-linux-gnu start.cpu0.o printf-stdarg.cpu0.o printf-stdarg-2.cpu0.o -o a.out
// /home/Gamma/test/lld/cmake_debug_build/bin/llvm-objdump -elf2hex a.out > ../cpu0_verilog/raw/cpu0s.hex
// /home/Gamma/test/lld/cmake_debug_build/bin/llvm-objdump -elf2hex a.out > ../cpu0_verilog/redesign/cpu0s.hex

// hexdump -s 0x0ef0 -n 368  -v -e '4/1 "%02x " "\n"' a.out

// objdump -s -j .rodata a.out | tail -n +5| awk '{print "/* " $1 " */\t" $2 " " $3 " " $4 " " $5 "\t/* " $6 " " $7 " " $8 " " $9 " " $10 " " $11 " " $12 " " $13 " " $14 " " $15 " " $16 " " $17 " " $18 " " $19 " " $20 "\t*/"}' >> ../cpu0_verilog/raw/cpu0s.hex

/// start

#include "print.h"

extern int printf(const char *format, ...);
extern int sprintf(char *out, const char *format, ...);

struct Time
{
  int hour;
  int minute;
  int second;
};

struct Date
{
  int year;
  int month;
  int day;
  int hour;
  int minute;
  int second;
};

int gI = 100;
struct Date gDate = {2012, 10, 12, 1, 2, 3};

int test_global()
{
  return gI;
}

struct Date copyDate(struct Date date)
{ 
  return date;
}

struct Time copyTime(struct Time time)
{ 
  return time;
}

// For memory IO
int putchar(const char c)
{
  char *p = (char*)OUT_MEM;
  *p = c;

  return 0;
}

int main(void)
{
  char *ptr = "Hello world!";
  char *np = 0;
  int i = 5;
  unsigned int bs = sizeof(int)*8;
  int mi;
  char buf[80];

  int a = 0;
  struct Time time1 = {1, 10, 12};
  struct Time time2;
  struct Date date;

  a = test_global();  // gI = 100
  printf("global variable gI = %d\n", a);
  printf("time1 = %d %d %d\n", time1.hour, time1.minute, time1.second);
  date = copyDate(gDate);
  printf("date = %d %d %d %d %d %d\n", date.year, date.month, date.day, date.hour, date.minute, date.second);
  time2 = copyTime(time1); // test return V0, V1, A0
  printf("time2 = %d %d %d\n", time2.hour, time2.minute, time2.second);

  mi = (1 << (bs-1)) + 1;
  printf("%s\n", ptr);
  printf("printf test\n");
  printf("%s is null pointer\n", np);
  printf("%d = 5\n", i);
  printf("%d = - max int\n", mi);
  printf("char %c = 'a'\n", 'a');
  printf("hex %x = ff\n", 0xff);
  printf("hex %02x = 00\n", 0);
  printf("signed %d = unsigned %u = hex %x\n", -3, -3, -3);
  printf("%d %s(s)%", 0, "message");
  printf("\n");
  printf("%d %s(s) with %%\n", 0, "message");
  sprintf(buf, "justif: \"%-10s\"\n", "left"); printf("%s", buf);
  sprintf(buf, "justif: \"%10s\"\n", "right"); printf("%s", buf);
  sprintf(buf, " 3: %04d zero padded\n", 3); printf("%s", buf);
  sprintf(buf, " 3: %-4d left justif.\n", 3); printf("%s", buf);
  sprintf(buf, " 3: %4d right justif.\n", 3); printf("%s", buf);
  sprintf(buf, "-3: %04d zero padded\n", -3); printf("%s", buf);
  sprintf(buf, "-3: %-4d left justif.\n", -3); printf("%s", buf);
  sprintf(buf, "-3: %4d right justif.\n", -3); printf("%s", buf);

  return 0;
}

