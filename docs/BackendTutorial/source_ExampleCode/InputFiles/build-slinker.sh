#!/usr/bin/env bash
OS=`uname -s`
echo "OS =" ${OS}

if [ "$OS" == "Linux" ]; then
  TOOLDIR=/usr/local/llvm/test/cmake_debug_build/bin
else
  TOOLDIR=~/llvm/test/cmake_debug_build/bin/Debug
fi

if [ "$1" != "" ] && [ $1 != cpu032I ] && [ $1 != cpu032II ]; then
  echo "1st argument is cpu032I (default) or cpu032II"
  exit 1
fi
if [ "$1" == "" ]; then
  CPU=cpu032I
else
  CPU=$1
fi
echo "CPU =" "${CPU}"

bash rminput.sh

clang -target mips-unknown-linux-gnu -c start.cpp -emit-llvm -o start.bc
clang -target mips-unknown-linux-gnu -c printf-stdarg.c -emit-llvm \
-o printf-stdarg.bc
clang -c ch9_4.cpp -emit-llvm -o ch9_4.bc
clang -target mips-unknown-linux-gnu -c ch_slinker.cpp -emit-llvm \
-o ch_slinker.bc
${TOOLDIR}/llc -march=cpu0 -mcpu=${CPU} -relocation-model=static -filetype=obj \
start.bc -o start.cpu0.o
${TOOLDIR}/llc -march=cpu0 -mcpu=${CPU} -relocation-model=static -filetype=obj \
printf-stdarg.bc -o printf-stdarg.cpu0.o
${TOOLDIR}/llc -march=cpu0 -mcpu=${CPU} -relocation-model=static -filetype=obj \
ch9_4.bc -o ch9_4.cpu0.o
${TOOLDIR}/llc -march=cpu0 -mcpu=${CPU} -relocation-model=static -filetype=obj \
ch_slinker.bc -o ch_slinker.cpu0.o
${TOOLDIR}/llc -march=cpu0 -mcpu=${CPU} -relocation-model=static -filetype=obj \
lib_cpu0.ll -o lib_cpu0.o
${TOOLDIR}/lld -flavor gnu -target cpu0-unknown-linux-gnu start.cpu0.o \
printf-stdarg.cpu0.o ch9_4.cpu0.o ch_slinker.cpu0.o lib_cpu0.o -o a.out
${TOOLDIR}/llvm-objdump -elf2hex a.out > ../cpu0_verilog/cpu0.hex
