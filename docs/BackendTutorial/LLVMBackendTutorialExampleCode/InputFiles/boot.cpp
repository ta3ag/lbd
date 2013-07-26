
#define OUT_MEM 0x7000 // 28672

// boot:
  asm("jmp 12"); // RESET: jmp RESET_START;
  asm("jmp 4");  // ERROR: jmp ERR_HANDLE;
  asm("jmp 4");  // IRQ: jmp IRQ_HANDLE;
  asm("jmp -4"); // ERR_HANDLE: jmp ERR_HANDLE; (loop forever)

  // RESET_START:
  #include "InitRegs.cpp"
  
  asm("addiu $sp, $zero, 0x6ffc");
  asm("addiu $3, $ZERO, 0x50"); // Assume main() entry point is address 0x50
  asm("iret $3");

