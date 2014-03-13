#!/bin/sh -a

rm matmul matmul.polly matmul.result matmul.polly.result
LLVM_INSTALL=~/test/polly/llvm_build
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

${LLVM_INSTALL}/bin/clang -O3 -DPOLYBENCH_DUMP_ARRAYS matmul.c -o matmul
pollycc -mllvm -polly -mllvm -polly-ignore-aliasing -O3 -DPOLYBENCH_DUMP_ARRAYS matmul.c -o matmul.polly

echo "\nWith print result"
echo "time ./matmul"
time -f "%E real, %U user, %S sys" ./matmul > matmul.result
echo "time ./matmul.polly"
time -f "%E real, %U user, %S sys" ./matmul.polly > matmul.polly.result

${LLVM_INSTALL}/bin/clang -O3 matmul.c -o matmul
pollycc -mllvm -polly -mllvm -polly-ignore-aliasing -O3 matmul.c -o matmul.polly

echo "\n\nNo print result"
echo "time ./matmul"
time -f "%E real, %U user, %S sys" ./matmul
echo "time ./matmul.polly"
time -f "%E real, %U user, %S sys" ./matmul.polly

echo "do diff matmul.result matmul.polly.result to ensure the output is same"
diff matmul.result matmul.polly.result


