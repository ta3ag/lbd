// LLVM_INSTALL=~/test/polly/llvm_build
// alias pollycc="${LLVM_INSTALL}/bin/clang++ -Xclang -load -Xclang ${LLVM_INSTALL}/lib/LLVMPolly.so"
// pollycc -O3 ex1.cpp -o ex1-clangvec
// pollycc -O3 -fno-vectorize ex1.cpp -o ex1-novec
// pollycc -O3 -fno-vectorize -mllvm -polly -mllvm -polly-vectorizer=polly -mllvm -polly-ignore-aliasing ex1.cpp -o ex1-polly-vec

#define LOOPA 10000000
#define LOOPB 2048
// sudo time ./ex1-O3: 0.28 second
// sudo time ./ex1-O3-novec: 1.08

// #define LOOPA 100000000
// sudo time ./ex1-O3: 2.80 second
// sudo time ./ex1-O3-novec: 10.84

// clang++ -O3 -S ex1.cpp -emit-llvm -o ex1-O3.ll

#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <iostream>
#include <vector>
#include <numeric>


/// This test contains some of the loops from the GCC vectrorizer example page [1].
/// Dorit Nuzman who developed the gcc vectorizer said that we can use them in our test suite.
///
/// [1] - http://gcc.gnu.org/projects/tree-ssa/vectorization.html

#define N 1024
#define ALIGNED16 __attribute__((aligned(16)))

int a[N*2] ALIGNED16;
int b[N*2] ALIGNED16;
int c[N*2] ALIGNED16;

__attribute__((noinline))
void example1 () {
  int i;

  for (i=0; i<LOOPB; i++){
    a[i] = b[i] + c[i];
  }
}

void init_memory(void *start, void* end) {
  unsigned char state = 1;
  while (start != end) {
    state *= 7; state ^= 0x27; state += 1;
    *((unsigned char*)start) = state;
    start = ((char*)start) + 1;
  }
}

int main(int argc,char* argv[]){
  init_memory(&a[0], &a[N*2]);
  init_memory(&b[0], &b[N*2]);
  init_memory(&c[0], &c[N*2]);
  for (int i=0; i < LOOPA; ++i) 
    example1();

  return 0;
}



