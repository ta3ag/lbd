// clang -target mips-unknown-linux-gnu -c ch12_3.cpp -emit-llvm -o ch12_3.bc
// /Users/Jonathan/llvm/test/cmake_debug_build/bin/Debug/llc -march=cpu0 -relocation-model=pic -filetype=obj ch12_3.bc -o ch12_3.cpu0.o

/// start
asm("ld	$2, 8($sp)");
asm("st	$0, 4($sp)");
asm("addiu $3,	$ZERO, 0");
asm("slti $4, $3, 1");
asm("sltiu $4, $3, 0");
asm("add $3, $1, $2");
asm("sub $3, $2, $3");
asm("mul $2, $1, $3");
asm("div $3, $2");
asm("divu $2, $3");
asm("and $2, $1, $3");
asm("or $3, $1, $2");
asm("xor $1, $2, $3");
asm("mult $4, $3");
asm("multu $3, $2");
asm("mfhi $3");
asm("mflo $2");
asm("mthi $2");
asm("mtlo $2");
asm("sra $2, $2, 2");
asm("rol $2, $1, 3");
asm("ror $3, $3, 4");
asm("shl $2, $2, 2");
asm("shr $2, $3, 5");
asm("beq $2, $4, 20");
asm("slt $4, $2, $3");
asm("sltu $2, $3, $1");
asm("bne $2, $3, -16");
asm("swi 0x00000400");
asm("jsub 0x000010000");
asm("ret $lr");
asm("jalr $t9");
asm("li $3, 0x00700000");
asm("la $3, 0x00800000($6)");
asm("la $3, 0x00900000");
