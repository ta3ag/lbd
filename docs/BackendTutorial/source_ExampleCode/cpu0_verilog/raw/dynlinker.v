`define DYNLINKER_INFO_ADDR  'h70000
`define GPADDR    'h7FFF0

`ifdef DYNLINKER
  task setDynLinkerInfo; begin
  // caculate number of dynamic entries
    numDynEntry = 0;
    j = 0;
    for (i=0; i < 384 && j == 0; i=i+52) begin
       if (so_func_offset[i] == `MEMEMPTY && so_func_offset[i+1] == `MEMEMPTY && 
           so_func_offset[i+2] == `MEMEMPTY && so_func_offset[i+3] == `MEMEMPTY) begin
         numDynEntry = i/52;
         j = 1;
         $display("numDynEntry = %8x", numDynEntry);
       end
    end
  // save number of dynamic entries to memory address `DYNLINKER_INFO_ADDR
    m[`DYNLINKER_INFO_ADDR] = numDynEntry[31:24];
    m[`DYNLINKER_INFO_ADDR+1] = numDynEntry[23:16];
    m[`DYNLINKER_INFO_ADDR+2] = numDynEntry[15:8];
    m[`DYNLINKER_INFO_ADDR+3] = numDynEntry[7:0];
  // copy section .dynsym of ELF to memory address `DYNLINKER_INFO_ADDR+4
    i = `DYNLINKER_INFO_ADDR+4;
    for (j=0; j < (4*numDynEntry); j=j+4) begin
      m[i] = dsym[j];
      m[i+1] = dsym[j+1];
      m[i+2] = dsym[j+2];
      m[i+3] = dsym[j+3];
      i = i + 4;
    end
  // copy the offset values of section .text of shared library .so of ELF to 
  // memory address `DYNLINKER_INFO_ADDR+4+numDynEntry*4
    i = `DYNLINKER_INFO_ADDR+4+numDynEntry*4;
    l = 0;
    for (j=0; j < numDynEntry; j=j+1) begin
      for (k=0; k < 52; k=k+1) begin
        m[i] = so_func_offset[l];
        i = i + 1;
        l = l + 1;
      end
    end
  `ifdef DYNDEBUG
    i = `DYNLINKER_INFO_ADDR+4+numDynEntry*4;
    for (j=0; j < (8*numDynEntry); j=j+8) begin
       $display("%8x: %8x", i, {m[i], m[i+1], m[i+2], m[i+3]});
      i = i + 8;
    end
  `endif
  // copy section .dynstr of ELF to memory address 
  // `DYNLINKER_INFO_ADDR+4+numDynEntry*4+numDynEntry*52
    i=`DYNLINKER_INFO_ADDR+4+numDynEntry*4+numDynEntry*52;
    for (j=0; dstr[j] != `MEMEMPTY; j=j+1) begin
      m[i] = dstr[j];
      i = i + 1;
    end
  `ifdef DYNDEBUG
    $display("In setDynLinkerInfo()");
    for (i=`DYNLINKER_INFO_ADDR; i < `MEMSIZE; i=i+4) begin
       if (m[i] != `MEMEMPTY || m[i+1] != `MEMEMPTY || 
         m[i+2] != `MEMEMPTY || m[i+3] != `MEMEMPTY)
         $display("%8x: %8x", i, {m[i], m[i+1], m[i+2], m[i+3]});
    end
    $display("global address %8x", {m[`GPADDR], m[`GPADDR+1], 
             m[`GPADDR+2], m[`GPADDR+3]});
    $display("gp = %8x", gp);
  `endif
// below code set mem as follows,
//                                 -----------------------------------
// gp ---------------------------> | all 0                           |
// gp+16 ------------------------> | 0                          |
// gp+16+1*4 --------------------> | 1st plt entry address      | (4 bytes)
//                                 | ...                        |
// gp+16+(numDynEntry-1)*4 ------> | the last plt entry address |
//                                 ------------------------------
// gp ---------------------------> | all 0                           | (16 bytes)
// gp+16+0*8'h10 ----------------> | 32'h10: pointer to plt0         |
// gp+16+1*8'h10 ----------------> | 1st plt entry                   |
// gp+16+2*8'h10 ----------------> | 2nd plt entry                   |
//                                 | ...                             |
// gp+16+(numDynEntry-1)*8'h10 --> | the last plt entry              |
//                                 -----------------------------------
// note: gp point to the _GLOBAL_OFFSET_TABLE_, 
//       numDynEntry = actual number of functions + 1.
//   gp+1*4..gp+numDynEntry*4 set to 8'h10 plt0 which will jump to dynamic linker.
//   After dynamic linker load function to memory, it will set gp+idx*4 to 
//   function memory address. For example, if the function index is 2, then the 
//   gp+2*4 is set to the memory address of this loaded function. 
//   Then the the caller call 
//  "ld $t9, 2*4($gp)" and "ret $t9" will jump to this loaded function directly.

    gpPlt = gp+16+numDynEntry*4;
    // set (gpPlt-16..gpPlt-1) to 0
    for (j=16; j >= 1; j=j-1)
      m[gpPlt+j] = 8'h00;
    // put plt in (gpPlt..gpPlt+numDynEntry*8'h10+1)
    for (i=1; i < numDynEntry; i=i+1) begin
      j=i*4;
      // (gp+'8h10..gp+numDynEntry*'8h10+15) set to plt entry
      // addiu	$t9, $zero, dynsym_idx
      m[gpPlt+i*8'h10] = 8'h09;
      m[gpPlt+i*8'h10+1] = 8'h60;
      m[gpPlt+i*8'h10+2] = i[15:8];
      m[gpPlt+i*8'h10+3] = i[7:0];
      // st	$t9, j($gp)
      m[gpPlt+i*8'h10+4] = 8'h02;
      m[gpPlt+i*8'h10+5] = 8'h6a;
      m[gpPlt+i*8'h10+6] = 0;
      m[gpPlt+i*8'h10+7] = 0;
      // ld	$t9, ('16h0010)($gp)
      m[gpPlt+i*8'h10+8] = 8'h01;
      m[gpPlt+i*8'h10+9] = 8'h6a;
      m[gpPlt+i*8'h10+10] = 0;
      m[gpPlt+i*8'h10+11] = 8'h10;
      // ret	$t9
      m[gpPlt+i*8'h10+12] = 8'h3c;
      m[gpPlt+i*8'h10+13] = 8'h60;
      m[gpPlt+i*8'h10+14] = 0;
      m[gpPlt+i*8'h10+15] = 0;
    end

    // .got.plt offset(0x00.0x03) has been set to 0 in elf already.
    // Set .got.plt offset(8'h10..numDynEntry*'8h10) point to plt entry as above.
  `ifdef DYNDEBUG
         $display("numDynEntry = %8x", numDynEntry);
  `endif
//      j32=32'h1fc0; // m[32'h1fc]="something" will hang. Very tricky
    m[gp+16] = 8'h0;
    m[gp+16+1] = 8'h0;
    m[gp+16+2] = 8'h0;
    m[gp+16+3] = 8'h10;
    j32=gpPlt+16;
    for (i=1; i < numDynEntry; i=i+1) begin
      m[gp+16+i*4] = j32[31:24];
      m[gp+16+i*4+1] = j32[23:16];
      m[gp+16+i*4+2] = j32[15:8];
      m[gp+16+i*4+3] = j32[7:0];
      j32=j32+16;
    end
  `ifdef DYNDEBUG
    // show (gp..gp+numDynEntry*4-1)
    for (i=0; i < numDynEntry; i=i+1) begin
      $display("%8x: %8x", gp+16+i*4, {m[gp+16+i*4], m[gp+16+i*4+1], m[gp+16+i*4+2], m[gp+16+i*4+3]});
    end
    // show (gpPlt..gpPlt+(numDynEntry+1)*8'h10-1)
    for (i=0; i < numDynEntry; i=i+1) begin
      for (j=0; j < 16; j=j+4)
        $display("%8x: %8x", gpPlt+i*8'h10+j, 
                 {m[gpPlt+i*8'h10+j], 
                  m[gpPlt+i*8'h10+j+1], 
                  m[gpPlt+i*8'h10+j+2], 
                  m[gpPlt+i*8'h10+j+3]});
    end
  `endif
  end endtask
`endif

`ifdef DYNLINKER
  task dynLoader; begin
    $readmemh("global_offset", globalAddr);
    m[`GPADDR]   = globalAddr[0];
    m[`GPADDR+1] = globalAddr[1];
    m[`GPADDR+2] = globalAddr[2];
    m[`GPADDR+3] = globalAddr[3];
    gp[31:24] = globalAddr[0];
    gp[23:16] = globalAddr[1];
    gp[15:8] = globalAddr[2];
    gp[7:0] = globalAddr[3];
`ifdef DYNDEBUG
    $display("global address %8x", {m[`GPADDR], m[`GPADDR+1], 
             m[`GPADDR+2], m[`GPADDR+3]});
    $display("gp = %8x", gp);
`endif
`endif
`ifdef DYNLINKER
  // erase memory
    for (i=0; i < `MEMSIZE; i=i+1) begin
       flash[i] = `MEMEMPTY;
    end
    for (i=0; i < 192; i=i+1) begin
       dsym[i] = `MEMEMPTY;
    end
    for (i=0; i < 96; i=i+1) begin
       dstr[i] = `MEMEMPTY;
    end
    for (i=0; i <384; i=i+1) begin
       so_func_offset[i] = `MEMEMPTY;
    end
  // erase memory
    for (i=0; i < `MEMSIZE; i=i+1) begin
       flash[i] = `MEMEMPTY;
    end
    $readmemh("libso.hex", flash);
`ifdef DYNDEBUG
    for (i=0; i < `MEMSIZE && (flash[i] != `MEMEMPTY || flash[i+1] != `MEMEMPTY || 
         flash[i+2] != `MEMEMPTY || flash[i+3] != `MEMEMPTY); i=i+4) begin
       $display("%8x: %8x", i, {flash[i], flash[i+1], flash[i+2], flash[i+3]});
    end
`endif
    $readmemh("dynsym", dsym);
    $readmemh("dynstr", dstr);
    $readmemh("so_func_offset", so_func_offset);
    setDynLinkerInfo();
  end endtask
`endif

