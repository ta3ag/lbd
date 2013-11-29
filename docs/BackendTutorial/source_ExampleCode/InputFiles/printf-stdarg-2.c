
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

