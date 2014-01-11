#!/usr/bin/env bash
#TOOLDIR=/usr/local/llvm/test/cmake_debug_build/bin
TOOLDIR=~/llvm/test/cmake_debug_build/bin/Debug

cpu=cpu032I

clang -target mips-unknown-linux-gnu -c ch_run_backend.cpp -emit-llvm -o \
ch_run_backend.bc
${TOOLDIR}/llc -march=cpu0 -mcpu=${cpu} -relocation-model=static -filetype=obj \
ch_run_backend.bc -o ch_run_backend.cpu0.o
${TOOLDIR}/llvm-objdump -d ch_run_backend.cpu0.o | tail -n +6| awk \
'{print "/* " $1 " */\t" $2 " " $3 " " $4 " " $5 "\t/* " $6"\t" $7" " $8" \
" $9" " $10 "\t*/"}' > ../cpu0_verilog/cpu0.hex
