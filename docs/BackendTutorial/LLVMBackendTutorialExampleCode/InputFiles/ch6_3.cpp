// clang -c ch6_3.cpp -emit-llvm -o ch6_3.bc
// /Users/Jonathan/llvm/test/cmake_debug_build/bin/Debug/llc -march=cpu0 -relocation-model=pic -filetype=asm ch6_3.bc -o ch6_3.cpu0.s
// /Users/Jonathan/llvm/test/cmake_debug_build/bin/Debug/llc -march=cpu0 -relocation-model=static -filetype=asm ch6_3.bc -o ch6_3.cpu0.static.s

struct Date
{
  short year;
  char month;
  char day;
  char hour;
  char minute;
  char second;
};

unsigned char b[4] = {'a', 'b', 'c', '\0'};

int main()
{
  unsigned char a = b[1];
  char c = (char)b[1];
  Date date1 = {2012, (char)11, (char)25, (char)9, (char)40, (char)15};
  char m = date1.month;
  char s = date1.second;

  return 0;
}

