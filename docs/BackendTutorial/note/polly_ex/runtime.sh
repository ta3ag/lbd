#!/bin/sh -a

rm matmul matmul.polly
LLVM_INSTALL=~/polly/llvm_build
alias pollycc="${LLVM_INSTALL}/bin/clang -Xclang -load -Xclang ${LLVM_INSTALL}/lib/LLVMPolly.so"

${LLVM_INSTALL}/bin/clang -O3 matmul.c -o matmul
pollycc -mllvm -polly -O3 matmul.c -o matmul.polly

echo "time ./matmul"
time -f "%E real, %U user, %S sys" ./matmul
echo "time ./matmul.polly"
time -f "%E real, %U user, %S sys" ./matmul.polly

