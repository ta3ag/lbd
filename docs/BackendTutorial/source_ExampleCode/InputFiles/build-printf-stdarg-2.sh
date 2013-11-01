#!/usr/bin/env bash
#TOOLDIR=/home/Gamma/test/lld/cmake_debug_build/bin
TOOLDIR=/home/Gamma/test/lld_1030/cmake_debug_build/bin
#TOOLDIR=/home/cschen/test/lld/cmake_debug_build/bin

/usr/local/llvm/release/cmake_debug_build/bin/clang -target mips-unknown-linux-gnu -c start.cpp -emit-llvm -o start.bc
/usr/local/llvm/release/cmake_debug_build/bin/llvm-dis start.bc -o start.ll
/usr/local/llvm/release/cmake_debug_build/bin/clang -target mips-unknown-linux-gnu -c printf-stdarg.c -emit-llvm -o printf-stdarg.bc
/usr/local/llvm/release/cmake_debug_build/bin/clang -target mips-unknown-linux-gnu -c printf-stdarg-2.c -emit-llvm -o printf-stdarg-2.bc
${TOOLDIR}/llc -march=cpu0 -relocation-model=static -filetype=obj start.ll -o start.cpu0.o
${TOOLDIR}/llc -march=cpu0 -relocation-model=static -filetype=obj printf-stdarg.bc -o printf-stdarg.cpu0.o
${TOOLDIR}/llc -march=cpu0 -relocation-model=static -filetype=obj printf-stdarg-2.bc -o printf-stdarg-2.cpu0.o
${TOOLDIR}/lld -flavor gnu -target cpu0-unknown-linux-gnu start.cpu0.o printf-stdarg.cpu0.o printf-stdarg-2.cpu0.o -o a.out
${TOOLDIR}/llvm-objdump -elf2hex a.out > ../cpu0_verilog/raw/cpu0s.hex
${TOOLDIR}/llvm-objdump -elf2hex a.out > ../cpu0_verilog/redesign/cpu0s.hex

