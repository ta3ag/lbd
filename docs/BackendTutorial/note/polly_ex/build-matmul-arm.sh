#!/bin/sh -a

rm matmul matmul.polly
LLVM_INSTALL=~/test/polly/llvm_arm_build
alias pollycc="${LLVM_INSTALL}/bin/clang -Xclang -load -Xclang ${LLVM_INSTALL}/lib/LLVMPolly.so"
alias opt="opt -load ${LLVM_INSTALL}/lib/LLVMPolly.so"

${LLVM_INSTALL}/bin/clang -O3 -S matmul.c -emit-llvm -o matmul.ll
pollycc -O3 -S matmul.c -emit-llvm -o matmul.no-polly.ll
pollycc -mllvm -polly -O3 -S matmul.c -emit-llvm -o matmul.polly.ll
echo "--> 3. Prepare the LLVM-IR for Polly"
opt -S -mem2reg -loop-simplify -polly-indvars matmul.no-polly.ll > matmul.preopt.ll
echo "--> 4. Show the SCoPs detected by Polly"
opt -basicaa -polly-cloog -analyze -q matmul.preopt.ll
echo "--> 3. Prepare the LLVM-IR for Polly"
opt -S -mem2reg -loop-simplify -polly-indvars matmul.polly.ll > matmul.preopt.ll
echo "--> 4. Show the SCoPs detected by Polly"
opt -basicaa -polly-cloog -analyze -q matmul.preopt.ll

${LLVM_INSTALL}/bin/clang -O3 -DTEST -mcpu=cortex-a9 -mfloat-abi=hard matmul.c -o pmatmul
pollycc -mllvm -polly -O3 -DTEST -mcpu=cortex-a9 -mfloat-abi=hard matmul.c -o pmatmul.polly

${LLVM_INSTALL}/bin/clang -O3 -mcpu=cortex-a9 -mfloat-abi=hard matmul.c -o matmul
pollycc -mllvm -polly -O3 -mcpu=cortex-a9 -mfloat-abi=hard matmul.c -o matmul.polly

scp pmatmul pmatmul.polly matmul matmul.polly root@10.19.132.196:/root/cschen/
