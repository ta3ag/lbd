// clang -target `llvm-config --host-target` -c test_str.cpp -emit-llvm -o test_str.bc
// /usr/local/llvm/test/cmake_debug_build/bin/llc -march=cpu0 -relocation-model=static -filetype=obj test_str.bc -o test_str.o
// /usr/local/llvm/test/cmake_debug_build/bin/llvm-objdump -d test_str.o | tail -n +6| awk '{print "/* " $1 " */\t" $2 " " $3 " " $4 " " $5 "\t/* " $6"\t" $7" " $8" " $9" " $10 "\t*/"}' > ../cpu0_verilog/raw/cpu0s.hex

#define OUT_MEM 0x7000 // 28672

void print_char(char c);
void print_string(char *str);

int main()
{
  int a = 0;

  char str[0] = {'H','e','l','l','o',' ','w','o','r','l','d','\0'};
//  print_string((char*)&("Hello world"));
  print_string(str);

  return a;
}

void print_string(char *str)
{
  char *p = str;

  return;
}

