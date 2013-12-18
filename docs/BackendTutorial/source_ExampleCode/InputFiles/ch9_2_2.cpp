// clang -target mips-unknown-linux-gnu -c ch9_2_2.cpp -emit-llvm -o ch9_2_2.bc
// /Users/Jonathan/llvm/test/cmake_debug_build/bin/Debug/llc -march=cpu0 -relocation-model=pic -filetype=asm ch9_2_2.bc -o -

/// start
extern "C" int printf(const char *format, ...);

class Date_9_2_2
{
public:
  int year;
  int month;
  int day;
  int hour;
  int minute;
  int second;
public:
  Date_9_2_2(int year, int month, int day, int hour, int minute, int second);
  Date_9_2_2 getDate();
};

Date_9_2_2::Date_9_2_2(int year, int month, int day, int hour, int minute, int second)
{
  this->year = year;
  this->month = month;
  this->day = day;
  this->hour = hour;
  this->minute = minute;
  this->second = second;
}

Date_9_2_2 Date_9_2_2::getDate()
{ 
  return *this;
}

int test_contructor()
{
  Date_9_2_2 date1 = Date_9_2_2(2013, 1, 26, 12, 21, 10);
  Date_9_2_2 date2 = date1.getDate();
#ifdef PRINT_TEST
  printf("date1 = %d %d %d %d %d %d\n", date1.year, date1.month, date1.day, \
    date1.hour, date1.minute, date1.second); // date1 = 2013 1 26 12 21 10
  printf("date2 = %d %d %d %d %d %d\n", date2.year, date2.month, date2.day, \
    date2.hour, date2.minute, date2.second); // date2 = 2013 1 26 12 21 10
#endif

  return 0;
}
