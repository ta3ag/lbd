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

clang -O1 -target mips-unknown-linux-gnu -c ch_optimize.cpp -emit-llvm -o \
ch_optimize.bc
${TOOLDIR}/llc -march=cpu0 -mcpu=${cpu} -relocation-model=static -filetype=obj \
ch_optimize.bc -o ch_optimize.cpu0.o
${TOOLDIR}/llvm-objdump -d ch_optimize.cpu0.o | tail -n +6| awk \
'{print "/* " $1 " */\t" $2 " " $3 " " $4 " " $5 "\t/* " $6"\t" $7" " $8" \
" $9" " $10 "\t*/"}' > ../cpu0_verilog/cpu0.hex
