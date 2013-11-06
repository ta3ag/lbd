#!/usr/bin/env bash
#TOOLDIR=/home/Gamma/test/lld/cmake_debug_build/bin
TOOLDIR=/home/cschen/test/lld/cmake_debug_build/bin

cpu=cpu032I

/usr/local/llvm/release/cmake_debug_build/bin/clang -target mips-unknown-linux-gnu -c start.cpp -emit-llvm -o start.bc
/usr/local/llvm/release/cmake_debug_build/bin/clang -target mips-unknown-linux-gnu -c dynamic_linker.cpp -emit-llvm -o dynamic_linker.cpu0.bc
/usr/local/llvm/release/cmake_debug_build/bin/clang -target mips-unknown-linux-gnu -c printf-stdarg.c -emit-llvm -o printf-stdarg.bc
/usr/local/llvm/release/cmake_debug_build/bin/clang -target mips-unknown-linux-gnu -c foobar.cpp -emit-llvm -o foobar.cpu0.bc
${TOOLDIR}/llc -march=cpu0 -mcpu=${cpu} -relocation-model=static -filetype=obj -cpu0-reserve-gp=true dynamic_linker.cpu0.bc -o dynamic_linker.cpu0.o
${TOOLDIR}/llc -march=cpu0 -mcpu=${cpu} -relocation-model=static -filetype=obj -cpu0-reserve-gp=true printf-stdarg.bc -o printf-stdarg.cpu0.o
${TOOLDIR}/llc -march=cpu0 -mcpu=${cpu} -relocation-model=pic -filetype=obj -cpu0-reserve-gp=true -cpu0-no-cpload=true foobar.cpu0.bc -o foobar.cpu0.o
${TOOLDIR}/lld -flavor gnu -target cpu0-unknown-linux-gnu -shared -o libfoobar.cpu0.so foobar.cpu0.o
${TOOLDIR}/llc -mcpu=${cpu} -march=cpu0 -relocation-model=static -filetype=obj -cpu0-reserve-gp=true start.bc -o start.cpu0.o
/usr/local/llvm/release/cmake_debug_build/bin/clang -target mips-unknown-linux-gnu -c ch_dynamiclinker.cpp -emit-llvm -o ch_dynamiclinker.cpu0.bc
${TOOLDIR}/llc -march=cpu0 -mcpu=${cpu} -relocation-model=static -filetype=obj -cpu0-reserve-gp=true ch_dynamiclinker.cpu0.bc -o ch_dynamiclinker.cpu0.o
${TOOLDIR}/lld -flavor gnu -target cpu0-unknown-linux-gnu start.cpu0.o printf-stdarg.cpu0.o dynamic_linker.cpu0.o ch_dynamiclinker.cpu0.o libfoobar.cpu0.so
${TOOLDIR}/llvm-objdump -elf2hex -cpu0dumpso libfoobar.cpu0.so > ../cpu0_verilog/libso.hex
${TOOLDIR}/llvm-objdump -elf2hex -cpu0linkso a.out > ../cpu0_verilog/cpu0.hex
cp dynstr dynsym so_func_offset global_offset ../cpu0_verilog/.

