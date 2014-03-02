// clang -c ch_c++_2.cpp -emit-llvm -o ch_c++_2.bc
// /Users/Jonathan/llvm/test/cmake_debug_build/bin/Debug/llvm-dis ch_c++_2.bc -o -
// /Users/Jonathan/llvm/test/cmake_debug_build/bin/Debug/llc -march=cpu0 -relocation-model=static -filetype=asm ch_c++_2.bc -o -

#include <string>
using namespace std;

int main() {
  try {
    string s("hello");
  }
  catch (...) {
  }
}