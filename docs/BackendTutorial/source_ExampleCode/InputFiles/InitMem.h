
/// start
#if 1
asm("st	$0, 0($sp)");
asm("li $3, 0x00800000");
#else
asm("sw	$0, 0($sp)");
#endif