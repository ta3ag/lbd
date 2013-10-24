//`define TRACE
`define DYNDEBUG
`define DYNLINKER
`define DYNLINKER_INFO_ADDR  'h70000

`define MEMSIZE   'h80000
`define MEMEMPTY   8'hFF
`define NULL       8'h00
`define IOADDR    'h80000
`define FLASHADDR 'hA0000
`define GPADDR    'h7FFF0

// Operand width
`define INT32 2'b11     // 32 bits
`define INT24 2'b10     // 24 bits
`define INT16 2'b01     // 16 bits
`define BYTE  2'b00     // 8  bits

`define EXE 3'b000
`define RESET 3'b001
`define ABORT 3'b010
`define IRQ 3'b011
`define ERROR 3'b100

// Reference web: http://ccckmit.wikidot.com/ocs:cpu0
module cpu0(input clock, reset, input [2:0] itype, output reg [2:0] tick, 
            output reg [31:0] ir, pc, mar, mdr, inout [31:0] dbus, 
            output reg m_en, m_rw, output reg [1:0] m_size);
  reg signed [31:0] R [0:15];
  // High and Low part of 64 bit result
  reg [7:0] op;
  reg [3:0] a, b, c;
  reg [4:0] c5;
  reg signed [31:0] c12, c16, uc16, c24, Ra, Rb, Rc, pc0; // pc0 : instruction pc
  reg [31:0] URa, URb, URc, HI, LO;

  // register name
  `define PC   R[15]   // Program Counter
  `define LR   R[14]   // Link Register
  `define SP   R[13]   // Stack Pointer
  `define SW   R[12]   // Status Word
  // SW Flage
  `define C    `SW[29] // Carry
  `define V    `SW[28] // Overflow
  `define MODE `SW[25:23] // itype
  `define I2   `SW[16] // Hardware Interrupt 1, IO1 interrupt, status, 1: in interrupt
  `define I1   `SW[15] // Hardware Interrupt 0, timer interrupt, status, 1: in interrupt
  `define I0   `SW[14] // Software interrupt, status, 1: in interrupt
  `define I    `SW[13] // Interrupt, 1: in interrupt
  `define I2E  `SW[8]  // Hardware Interrupt 1, IO1 interrupt, Enable
  `define I1E  `SW[7]  // Hardware Interrupt 0, timer interrupt, Enable
  `define I0E  `SW[6]  // Software Interrupt Enable
  `define IE   `SW[5]  // Interrupt Enable
  `define M    `SW[4]  // Mode bit
  `define TR   `SW[2] // Debug Trace
  `define Z    `SW[1] // Zero
  `define N    `SW[0] // Negative flag
  // Instruction Opcode 
  parameter [7:0] LD=8'h01,ST=8'h02,LB=8'h03,LBu=8'h04,SB=8'h05,LH=8'h06,
  LHu=8'h07,SH=8'h08,ADDiu=8'h09,ANDi=8'h0C,ORi=8'h0D,
  XORi=8'h0E,LUi=8'h0F,
  CMP=8'h10,
  ADDu=8'h11,SUBu=8'h12,ADD=8'h13,SUB=8'h14,MUL=8'h17,
  AND=8'h18,OR=8'h19,XOR=8'h1A,
  ROL=8'h1B,ROR=8'h1C,SRA=8'h1D,SHL=8'h1E,SHR=8'h1F,
  SRAV=8'h20,SHLV=8'h21,SHRV=8'h22,
  JEQ=8'h30,JNE=8'h31,JLT=8'h32,JGT=8'h33,JLE=8'h34,JGE=8'h35,
  JMP=8'h36,
  SWI=8'h3A,JSUB=8'h3B,RET=8'h3C,IRET=8'h3D,JALR=8'h3E,
  MULT=8'h41,MULTu=8'h42,DIV=8'h43,DIVu=8'h44,
  MFHI=8'h46,MFLO=8'h47,MTHI=8'h48,MTLO=8'h49;

  reg [0:0] inInt = 0;
  reg [2:0] state, next_state; 
  reg [2:0] st_taskInt, ns_taskInt; 
  parameter Reset=3'h0, Fetch=3'h1, Decode=3'h2, Execute=3'h3, WriteBack=3'h4;
  integer i;

  task memReadStart(input [31:0] addr, input [1:0] size); begin // Read Memory Word
    mar = addr;     // read(m[addr])
    m_rw = 1;     // Access Mode: read 
    m_en = 1;     // Enable read
    m_size = size;
  end endtask

  task memReadEnd(output [31:0] data); begin // Read Memory Finish, get data
    mdr = dbus; // get momory, dbus = m[addr]
    data = mdr; // return to data
    m_en = 0; // read complete
  end endtask

  // Write memory -- addr: address to write, data: date to write
  task memWriteStart(input [31:0] addr, input [31:0] data, input [1:0] size); begin 
    mar = addr;    // write(m[addr], data)
    mdr = data;
    m_rw = 0;    // access mode: write
    m_en = 1;     // Enable write
    m_size  = size;
  end endtask

  task memWriteEnd; begin // Write Memory Finish
    m_en = 0; // write complete
  end endtask

  task regSet(input [3:0] i, input [31:0] data); begin
    if (i != 0) R[i] = data;
  end endtask

  task regHILOSet(input [31:0] data1, input [31:0] data2); begin
    HI = data1;
    LO = data2;
  end endtask

  task outw(input [31:0] data); begin
    if (data[7:0] != 8'h00) begin
      $write("%c", data[7:0]);
      if (data[15:8] != 8'h00) 
        $write("%c", data[15:8]);
      if (data[23:16] != 8'h00) 
        $write("%c", data[23:16]);
      if (data[31:24] != 8'h00) 
        $write("%c", data[31:24]);
    end
  end endtask

  task outc(input [7:0] data); begin
      $write("%c", data[7:0]);
  end endtask

  task taskInterrupt(input [2:0] iMode); begin
  if (inInt == 0) begin
    case (iMode)
      `RESET: begin 
        `PC = 0; tick = 0; R[0] = 0; `SW = 0; `LR = -1;
        `IE = 0; `I0E = 0; `I1E = 0; `I2E = 0; `I = 0; `I0 = 0; `I1 = 0; `I2 = 0;
      end
      `ABORT: begin `LR = `PC; `PC = 4; end
      `IRQ:   begin `LR = `PC; `PC = 8; end
      `ERROR: begin `LR = `PC; `PC = 12; end
    endcase
    $display("taskInterrupt(%3b)", iMode);
    inInt = 1;
  end
  end endtask

  task taskExecute; begin
    m_en = 0;
    tick = tick+1;
    case (state)
    Fetch: begin  // Tick 1 : instruction fetch, throw PC to address bus, 
                  // memory.read(m[PC])
      memReadStart(`PC, `INT32);
      pc0  = `PC;
      `PC = `PC+4;
      next_state = Decode;
    end
    Decode: begin  // Tick 2 : instruction decode, ir = m[PC]
      memReadEnd(ir); // IR = dbus = m[PC]
      {op,a,b,c} = ir[31:12];
      c24 = $signed(ir[23:0]);
      c16 = $signed(ir[15:0]);
      uc16 = ir[15:0];
      c12 = $signed(ir[11:0]);
      c5  = ir[4:0];
      Ra = R[a];
      Rb = R[b];
      Rc = R[c];
      URa = R[a];
      URb = R[b];
      URc = R[c];
      next_state = Execute;
    end
    Execute: begin // Tick 3 : instruction execution
      case (op)
      // load and store instructions
      LD:    memReadStart(Rb+c16, `INT32);      // LD Ra,[Rb+Cx]; Ra<=[Rb+Cx]
      ST:    memWriteStart(Rb+c16, Ra, `INT32); // ST Ra,[Rb+Cx]; Ra=>[Rb+Cx]
      LB:    memReadStart(Rb+c16, `BYTE);     // LB Ra,[Rb+Cx]; Ra<=(byte)[Rb+Cx]
      LBu:   memReadStart(Rb+c16, `BYTE);     // LBu Ra,[Rb+Cx]; Ra<=(byte)[Rb+Cx]
      SB:    memWriteStart(Rb+c16, Ra, `BYTE);// SB Ra,[Rb+Cx]; Ra=>(byte)[Rb+Cx]
      LH:    memReadStart(Rb+c16, `INT16);     // LH Ra,[Rb+Cx]; Ra<=(2bytes)[Rb+Cx]
      LHu:   memReadStart(Rb+c16, `INT16);     // LHu Ra,[Rb+Cx]; Ra<=(2bytes)[Rb+Cx]
      SH:    memWriteStart(Rb+c16, Ra, `INT16);// SH Ra,[Rb+Cx]; Ra=>(2bytes)[Rb+Cx]
      // Mathematic 
      ADDiu: R[a] = Rb+c16;                   // ADDiu Ra, Rb+Cx; Ra<=Rb+Cx
      CMP:   begin `N=(Ra-Rb<0);`Z=(Ra-Rb==0); end // CMP Ra, Rb; SW=(Ra >=< Rb)
      ADDu:  regSet(a, Rb+Rc);               // ADDu Ra,Rb,Rc; Ra<=Rb+Rc
      ADD:   begin regSet(a, Rb+Rc); if (a < Rb) `V = 1; else `V =0; end
                                             // ADD Ra,Rb,Rc; Ra<=Rb+Rc
      SUBu:  regSet(a, Rb-Rc);               // SUBu Ra,Rb,Rc; Ra<=Rb-Rc
      SUB:   begin regSet(a, Rb-Rc); if (Rb < 0 && Rc > 0 && a >= 0) 
             `V = 1; else `V =0; end         // SUB Ra,Rb,Rc; Ra<=Rb-Rc
      MUL:   regSet(a, Rb*Rc);               // MUL Ra,Rb,Rc;     Ra<=Rb*Rc
      DIVu:  regHILOSet(URa%URb, URa/URb);   // DIVu URa,URb; HI<=URa%URb; LO<=URa/URb
                                             // without exception overflow
      DIV:   begin regHILOSet(Ra%Rb, Ra/Rb); 
             if ((Ra < 0 && Rb < 0) || (Ra == 0)) `V = 1; 
             else `V =0; end  // DIV Ra,Rb; HI<=Ra%Rb; LO<=Ra/Rb; With overflow
      AND:   regSet(a, Rb&Rc);               // AND Ra,Rb,Rc; Ra<=(Rb and Rc)
      ANDi:  regSet(a, Rb&uc16);             // ANDi Ra,Rb,c16; Ra<=(Rb and c16)
      OR:    regSet(a, Rb|Rc);               // OR Ra,Rb,Rc; Ra<=(Rb or Rc)
      ORi:   regSet(a, Rb|uc16);             // ORi Ra,Rb,c16; Ra<=(Rb or c16)
      XOR:   regSet(a, Rb^Rc);               // XOR Ra,Rb,Rc; Ra<=(Rb xor Rc)
      XORi:  regSet(a, Rb^uc16);             // XORi Ra,Rb,c16; Ra<=(Rb xor c16)
      LUi:   regSet(a, uc16<<16);
      SHL:   regSet(a, Rb<<c5);     // Shift Left; SHL Ra,Rb,Cx; Ra<=(Rb << Cx)
      SRA:   regSet(a, (Rb&'h80000000)|(Rb>>c5)); 
                                    // Shift Right with signed bit fill;
                                    // SHR Ra,Rb,Cx; Ra<=(Rb&0x80000000)|(Rb>>Cx)
      SHR:   regSet(a, Rb>>c5);     // Shift Right with 0 fill; 
                                    // SHR Ra,Rb,Cx; Ra<=(Rb >> Cx)
      SHLV:  regSet(a, Rb<<Rc);     // Shift Left; SHLV Ra,Rb,Rc; Ra<=(Rb << Rc)
      SRAV:  regSet(a, (Rb&'h80000000)|(Rb>>Rc)); 
                                    // Shift Right with signed bit fill;
                                    // SHRV Ra,Rb,Rc; Ra<=(Rb&0x80000000)|(Rb>>Rc)
      SHRV:  regSet(a, Rb>>Rc);     // Shift Right with 0 fill; 
                                    // SHRV Ra,Rb,Rc; Ra<=(Rb >> Rc)
      ROL:   regSet(a, (Rb<<c5)|(Rb>>(32-c5)));     // Rotate Left;
      ROR:   regSet(a, (Rb>>c5)|(Rb<<(32-c5)));     // Rotate Right;
      MFLO:  regSet(a, LO);         // MFLO Ra; Ra<=LO
      MFHI:  regSet(a, HI);         // MFHI Ra; Ra<=HI
      MTLO:  LO = Ra;               // MTLO Ra; LO<=Ra
      MTHI:  HI = Ra;               // MTHI Ra; HI<=Ra
      MULT:  {HI, LO}=Ra*Rb;        // MULT Ra,Rb; HI<=((Ra*Rb)>>32); 
                                    // LO<=((Ra*Rb) and 0x00000000ffffffff);
                                    // with exception overflow
      MULTu: {HI, LO}=URa*URb;      // MULT URa,URb; HI<=((URa*URb)>>32); 
                                    // LO<=((URa*URb) and 0x00000000ffffffff);
                                    // without exception overflow
      // Jump Instructions
      JEQ:   if (`Z) `PC=`PC+c24;            // JEQ Cx; if SW(=) PC  PC+Cx
      JNE:   if (!`Z) `PC=`PC+c24;           // JNE Cx; if SW(!=) PC PC+Cx
      JLT:   if (`N)`PC=`PC+c24;             // JLT Cx; if SW(<) PC  PC+Cx
      JGT:   if (!`N&&!`Z) `PC=`PC+c24;      // JGT Cx; if SW(>) PC  PC+Cx
      JLE:   if (`N || `Z) `PC=`PC+c24;      // JLE Cx; if SW(<=) PC PC+Cx    
      JGE:   if (!`N || `Z) `PC=`PC+c24;     // JGE Cx; if SW(>=) PC PC+Cx
      JMP:   `PC = `PC+c24;                  // JMP Cx; PC <= PC+Cx
      SWI:   begin 
        `LR=`PC;`PC= c24; `I0 = 1'b1; `I = 1'b1;
      end // Software Interrupt; SWI Cx; LR <= PC; PC <= Cx; INT<=1
      JSUB:  begin `LR=`PC;`PC=`PC + c24; end // JSUB Cx; LR<=PC; PC<=PC+Cx
      JALR:  begin R[a] =`PC;`PC=Rb; end // JALR Ra,Rb; Ra<=PC; PC<=Rb
      RET:   begin `PC=Ra; end               // RET; PC <= Ra
      IRET:  begin 
        `PC=Ra;`I = 1'b0; `MODE = `EXE;
      end // Interrupt Return; IRET; PC <= LR; INT<=0
      default : 
        $display("%4dns %8x : OP code %8x not support", $stime, pc0, op);
      endcase
      next_state = WriteBack;
    end
    WriteBack: begin // Read/Write finish, close memory
      case (op)
        LD, LB, LBu, LH, LHu  : memReadEnd(R[a]); 
                                          //read memory complete
        ST, SB, SH  : memWriteEnd(); 
                                          // write memory complete
      endcase
      case (op)
      MULT, MULTu, DIV, DIVu, MTHI, MTLO :
        if (`TR)
          $display("%4dns %8x : %8x HI=%8x LO=%8x SW=%8x", $stime, pc0, ir, HI, 
        LO, `SW);
      ST : begin
        if (`TR)
          $display("%4dns %8x : %8x m[%-04d+%-04d]=%-d  SW=%8x", $stime, pc0, ir, 
          R[b], c16, R[a], `SW);
        if (R[b]+c16 == `IOADDR) begin
          outw(R[a]);
        end
      end
      SB : begin
        if (`TR)
          $display("%4dns %8x : %8x m[%-04d+%-04d]=%c  SW=%8x", $stime, pc0, ir, 
        R[b], c16, R[a][7:0], `SW);
        if (R[b]+c16 == `IOADDR) begin
          outc(R[a][7:0]);
        end
      end
      default : 
        if (`TR)
          $display("%4dns %8x : %8x R[%02d]=%-8x=%-d SW=%8x", $stime, pc0, ir, a, 
          R[a], R[a], `SW);
      endcase
      if (op==RET && `PC < 0) begin
        $display("RET to PC < 0, finished!");
        $finish;
      end
      next_state = Fetch;
    end
    endcase
  end endtask

  always @(posedge clock) begin
    if (inInt == 0 && itype == `RESET) begin
      taskInterrupt(`RESET);
      `MODE = `RESET;
      state = Fetch;
    end else if (inInt == 0 && (state == Fetch) && (`IE && `I) && ((`I0E && `I0) || (`I1E && `I1) || (`I2E && `I2)) ) begin
      `MODE = `IRQ;
      taskInterrupt(`IRQ);
      state = Fetch;
    end else begin
      //`TR = 1;
      taskExecute();
      state = next_state;
    end
    pc = `PC;
  end
endmodule

module memory0(input clock, reset, en, rw, input [1:0] m_size, 
                input [31:0] abus, dbus_in, output [31:0] dbus_out);
  reg [7:0] m [0:`MEMSIZE-1];
`ifdef DYNLINKER
  reg [7:0] flash [0:`MEMSIZE-1];
  reg [7:0] dsym [0:192-1];
  reg [7:0] dstr [0:96-1];
  reg [7:0] so_func_offset[0:384-1];
  reg [7:0] globalAddr [0:3];
  reg [31:0] fabus;
  integer j, k, l, numDynEntry;
`endif
  reg [31:0] data;

  integer i;

`ifdef DYNLINKER
  task setDynLinkerInfo; begin
  // caculate number of dynamic entries
    numDynEntry = 0;
    j = 0;
    for (i=0; i < 192 && j == 0; i=i+8) begin
       if (dsym[i] == `MEMEMPTY && dsym[i+1] == `MEMEMPTY && 
           dsym[i+2] == `MEMEMPTY && dsym[i+3] == `MEMEMPTY &&
           dsym[i+4] == `MEMEMPTY && dsym[i+5] == `MEMEMPTY && 
           dsym[i+6] == `MEMEMPTY && dsym[i+7] == `MEMEMPTY) begin
         numDynEntry = i/8;
         j = 1;
         $display("numDynEntry = %8x", numDynEntry);
       end
    end
  // save number of dynamic entries to memory address `DYNLINKER_INFO_ADDR
    m[`DYNLINKER_INFO_ADDR] = numDynEntry[31:24];
    m[`DYNLINKER_INFO_ADDR+1] = numDynEntry[23:16];
    m[`DYNLINKER_INFO_ADDR+2] = numDynEntry[15:8];
    m[`DYNLINKER_INFO_ADDR+3] = numDynEntry[7:0];
    m[`DYNLINKER_INFO_ADDR+4] = 0;
    m[`DYNLINKER_INFO_ADDR+5] = 0;
    m[`DYNLINKER_INFO_ADDR+6] = 0;
    m[`DYNLINKER_INFO_ADDR+7] = 0;
  // copy section .dynsym of ELF to memory address `DYNLINKER_INFO_ADDR+8
    i = `DYNLINKER_INFO_ADDR+8;
    for (j=0; j < (8*numDynEntry); j=j+8) begin
      m[i] = dsym[j];
      m[i+1] = dsym[j+1];
      m[i+2] = dsym[j+2];
      m[i+3] = dsym[j+3];
      m[i+4] = dsym[j+4];
      m[i+5] = dsym[j+5];
      m[i+6] = dsym[j+6];
      m[i+7] = dsym[j+7];
      i = i + 8;
    end
  // copy the offset values of section .text of shared library .so of ELF to 
  // memory address `DYNLINKER_INFO_ADDR+8+numDynEntry*8
    i = `DYNLINKER_INFO_ADDR+8+numDynEntry*8;
    l = 0;
    for (j=0; j < numDynEntry; j=j+1) begin
      for (k=0; k < 52; k=k+1) begin
        m[i] = so_func_offset[l];
        i = i + 1;
        l = l + 1;
      end
    end
    i = `DYNLINKER_INFO_ADDR+8+numDynEntry*8;
    for (j=0; j < (8*numDynEntry); j=j+8) begin
       $display("%8x: %8x", i, {m[i], m[i+1], m[i+2], m[i+3]});
       $display("%8x: %8x", i+4, {m[i+4], m[i+5], m[i+6], m[i+7]});
      i = i + 8;
    end
  // copy section .dynstr of ELF to memory address 
  // `DYNLINKER_INFO_ADDR+numDynEntry*8+numDynEntry*52
    i=`DYNLINKER_INFO_ADDR+8+numDynEntry*8+numDynEntry*52;
    for (j=0; dstr[j] != `MEMEMPTY; j=j+1) begin
      m[i] = dstr[j];
      i = i + 1;
    end
    $display("In setDynLinkerInfo()");
  `ifdef DYNDEBUG
    for (i=`DYNLINKER_INFO_ADDR; i < `MEMSIZE; i=i+4) begin
       if (m[i] != `MEMEMPTY || m[i+1] != `MEMEMPTY || 
         m[i+2] != `MEMEMPTY || m[i+3] != `MEMEMPTY)
         $display("%8x: %8x", i, {m[i], m[i+1], m[i+2], m[i+3]});
    end
    $display("global address %8x", {m[`GPADDR], m[`GPADDR+1], 
             m[`GPADDR+2], m[`GPADDR+3]});
  `endif
  end endtask
`endif

  initial begin
  // erase memory
    for (i=0; i < `MEMSIZE; i=i+1) begin
       m[i] = `MEMEMPTY;
    end
  // display memory contents
    $readmemh("cpu0s.hex", m);
    `ifdef TRACE
      for (i=0; i < `MEMSIZE && (m[i] != `MEMEMPTY || m[i+1] != `MEMEMPTY || 
         m[i+2] != `MEMEMPTY || m[i+3] != `MEMEMPTY); i=i+4) begin
        $display("%8x: %8x", i, {m[i], m[i+1], m[i+2], m[i+3]});
      end
    `endif
`ifdef DYNLINKER
    $readmemh("global_offset", globalAddr);
    m[`GPADDR]   = globalAddr[0];
    m[`GPADDR+1] = globalAddr[1];
    m[`GPADDR+2] = globalAddr[2];
    m[`GPADDR+3] = globalAddr[3];
`ifdef DYNDEBUG
    $display("global address %8x", {m[`GPADDR], m[`GPADDR+1], 
             m[`GPADDR+2], m[`GPADDR+3]});
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
`endif
  end

  always @(clock or abus or en or rw or dbus_in) 
  begin
    if (abus >= 0 && abus <= `MEMSIZE-4) begin
      if (en == 1 && rw == 0) begin // r_w==0:write
        data = dbus_in;
        case (m_size)
        `BYTE:  {m[abus]} = dbus_in[7:0];
        `INT16: {m[abus], m[abus+1] } = dbus_in[15:0];
        `INT24: {m[abus], m[abus+1], m[abus+2]} = dbus_in[24:0];
        `INT32: {m[abus], m[abus+1], m[abus+2], m[abus+3]} = dbus_in;
        endcase
      end else if (en == 1 && rw == 1) begin// r_w==1:read
        case (m_size)
        `BYTE:  data = {8'h00  , 8'h00,   8'h00,   m[abus]      };
        `INT16: data = {8'h00  , 8'h00,   m[abus], m[abus+1]    };
        `INT24: data = {8'h00  , m[abus], m[abus+1], m[abus+2]  };
        `INT32: data = {m[abus], m[abus+1], m[abus+2], m[abus+3]};
        endcase
      end else
        data = 32'hZZZZZZZZ;
`ifdef DYNLINKER
    end else if (abus >= `FLASHADDR && abus <= `FLASHADDR+`MEMSIZE-4) begin
      fabus = abus-`FLASHADDR;
      if (en == 1 && rw == 0) begin // r_w==0:write
        data = dbus_in;
        case (m_size)
        `BYTE:  {flash[fabus]} = dbus_in[7:0];
        `INT16: {flash[fabus], flash[fabus+1] } = dbus_in[15:0];
        `INT24: {flash[fabus], flash[fabus+1], flash[fabus+2]} = dbus_in[24:0];
        `INT32: {flash[fabus], flash[fabus+1], flash[fabus+2], flash[fabus+3]} = dbus_in;
        endcase
      end else if (en == 1 && rw == 1) begin// r_w==1:read
        case (m_size)
        `BYTE:  data = {8'h00  , 8'h00,   8'h00,   flash[fabus]      };
        `INT16: data = {8'h00  , 8'h00,   flash[fabus], flash[fabus+1]    };
        `INT24: data = {8'h00  , flash[fabus], flash[fabus+1], flash[fabus+2]  };
        `INT32: data = {flash[fabus], flash[fabus+1], flash[fabus+2], flash[fabus+3]};
        endcase
      end else
        data = 32'hZZZZZZZZ;
`endif
    end else 
      data = 32'hZZZZZZZZ;
  end
  assign dbus_out = data;
endmodule

module main;
  reg clock;
  reg [2:0] itype;
  wire [2:0] tick;
  wire [31:0] pc, ir, mar, mdr, dbus;
  wire m_en, m_rw;
  wire [1:0] m_size;

  cpu0 cpu(.clock(clock), .itype(itype), .pc(pc), .tick(tick), .ir(ir),
  .mar(mar), .mdr(mdr), .dbus(dbus), .m_en(m_en), .m_rw(m_rw), .m_size(m_size));

  memory0 mem(.clock(clock), .reset(reset), .en(m_en), .rw(m_rw), .m_size(m_size), 
  .abus(mar), .dbus_in(mdr), .dbus_out(dbus));

  initial
  begin
    clock = 0;
    itype = `RESET;
    #300000000 $finish;
  end

  always #10 clock=clock+1;

endmodule
