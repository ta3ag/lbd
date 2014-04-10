#!/bin/sh -a

LLVM_INSTALL=~/test/polly/llvm_build
alias pollycc="${LLVM_INSTALL}/bin/clang++ -Xclang -load -Xclang ${LLVM_INSTALL}/lib/LLVMPolly.so"
alias opt="opt -load ${LLVM_INSTALL}/lib/LLVMPolly.so"

${LLVM_INSTALL}/bin/clang -O3 -S ex1.cpp -emit-llvm -o ex1.ll
pollycc -O3 -S ex1.cpp -emit-llvm -o ex1.no-polly.ll
pollycc -mllvm -polly -O3 -S ex1.cpp -emit-llvm -o ex1.polly.ll
echo "--> 3. Prepare the LLVM-IR for Polly"
opt -S -mem2reg -loop-simplify -polly-indvars ex1.no-polly.ll > ex1.preopt.ll
echo "--> 4. Show the SCoPs detected by Polly"
opt -basicaa -polly-cloog -analyze -q ex1.preopt.ll
echo "--> 3. Prepare the LLVM-IR for Polly"
opt -S -mem2reg -loop-simplify -polly-indvars ex1.polly.ll > ex1.preopt.ll
echo "--> 4. Show the SCoPs detected by Polly"
opt -basicaa -polly-cloog -analyze -q ex1.preopt.ll

