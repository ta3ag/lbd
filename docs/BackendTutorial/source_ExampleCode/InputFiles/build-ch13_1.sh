#!/usr/bin/env bash
#TOOLDIR=/home/Gamma/test/lld/cmake_debug_build/bin
TOOLDIR=/home/cschen/test/lld/cmake_debug_build/bin

cpu=cpu032I

/usr/local/llvm/release/cmake_debug_build/bin/clang -target mips-unknown-linux-gnu -c start.cpp -emit-llvm -o start.bc
/usr/local/llvm/release/cmake_debug_build/bin/clang -target mips-unknown-linux-gnu -c ch13_1.c -emit-llvm -o ch13_1.bc
/usr/local/llvm/release/cmake_debug_build/bin/clang -target mips-unknown-linux-gnu -c a.c -emit-llvm -o a.bc
${TOOLDIR}/llc -march=cpu0 -mcpu=${cpu} -relocation-model=static -filetype=obj  start.bc -o start.cpu0.o
${TOOLDIR}/llc -march=cpu0 -mcpu=${cpu} -relocation-model=static -filetype=obj ch13_1.bc -o ch13_1.cpu0.o
${TOOLDIR}/llc -march=cpu0 -mcpu=${cpu} -relocation-model=static -filetype=obj a.bc -o a.cpu0.o
${TOOLDIR}/lld -flavor gnu -target cpu0-unknown-linux-gnu start.cpu0.o ch13_1.cpu0.o a.cpu0.o -o a.out

