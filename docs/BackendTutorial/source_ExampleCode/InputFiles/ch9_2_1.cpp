// clang -target mips-unknown-linux-gnu -c ch9_2_1.cpp -emit-llvm -o ch9_2_1.bc
// /Users/Jonathan/llvm/test/cmake_debug_build/bin/Debug/llc -march=cpu0 -relocation-model=pic -filetype=asm ch9_2_1.bc -o -

/// start
extern "C" int printf(const char *format, ...);

struct Date
{
  int year;
  int month;
  int day;
  int hour;
  int minute;
  int second;
};
Date gDate = {2012, 10, 12, 1, 2, 3};

struct Time
{
  int hour;
  int minute;
  int second;
};
Time gTime = {2, 20, 30};

Date getDate()
{ 
  return gDate;
}

Date copyDate(Date date)
{ 
  return date;
}

Date copyDate(Date* date)
{ 
  return *date;
}

Time copyTime(Time time)
{ 
  return time;
}

Time copyTime(Time* time)
{ 
  return *time;
}

int test_func_arg_struct()
{
  Time time1 = {1, 10, 12};
  Date date1 = getDate();
  Date date2 = copyDate(date1);
  Date date3 = copyDate(&date1);
  Time time2 = copyTime(time1);
  Time time3 = copyTime(&time1);
#ifdef PRINT_TEST
  printf("date1 = %d %d %d %d %d %d\n", date1.year, date1.month, date1.day, \
    date1.hour, date1.minute, date1.second); // date1 = 2012 10 12 1 2 3
  printf("date2 = %d %d %d %d %d %d\n", date2.year, date2.month, date2.day, \ 
    date2.hour, date2.minute, date2.second); // date2 = 2012 10 12 1 2 3
  // time2 = 1 10 12
  printf("time2 = %d %d %d\n", time2.hour, time2.minute, time2.second);
  // time3 = 1 10 12
  printf("time3 = %d %d %d\n", time3.hour, time3.minute, time3.second);
#endif

  return 0;
}
