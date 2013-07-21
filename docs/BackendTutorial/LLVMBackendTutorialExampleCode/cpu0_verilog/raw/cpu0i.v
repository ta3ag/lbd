`define MEMSIZE 'h7000
`define MEMEMPTY 8'hFF
`define IOADDR  'h7000

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
  reg signed [31:0] R [0:15], HI, LO;
  // High and Low part of 64 bit result
  reg [7:0] op;
  reg [3:0] a, b, c;
  reg [4:0] c5;
  reg signed [31:0] c12, c16, uc16, c24, Ra, Rb, Rc, pc0; // pc0 : instruction pc

  // register name
  `define PC   R[15]   // Program Counter
  `define LR   R[14]   // Link Register
  `define SP   R[13]   // Stack Pointer
  `define SW   R[12]   // Status Word
  // SW Flage
  `define N    `SW[31] // Negative flag
  `define Z    `SW[30] // Zero
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
  `define M    `SW[0]  // Mode bit
  // Instruction Opcode 
  parameter [7:0] LD=8'h01,ST=8'h02,LB=8'h03,LBu=8'h04,SB=8'h05,LH=8'h06,
  LHu=8'h07,SH=8'h08,ADDiu=8'h09,ANDi=8'h0C,ORi=8'h0D,
  XORi=8'h0E,
  CMP=8'h10,
  ADDu=8'h11,SUBu=8'h12,ADD=8'h13,SUB=8'h14,MUL=8'h15,SDIV=8'h16,
  AND=8'h18,OR=8'h19,XOR=8'h1A,
  SRA=8'h1B,ROL=8'h1C,ROR=8'h1D,SHL=8'h1E,SHR=8'h1F,
  JEQ=8'h20,JNE=8'h21,JLT=8'h22,JGT=8'h23,JLE=8'h24,JGE=8'h25,
  JMP=8'h26,
  SWI=8'h2A,JSUB=8'h2B,RET=8'h2C,IRET=8'h2D,JALR=8'h2E,
  MFHI=8'h40,MFLO=8'h41,MTHI=8'h42,MTLO=8'h43,
  MULT=8'h50;

  reg [0:0] inInt = 0;
  reg [2:0] state, next_state; 
  parameter Reset=3'h0, Fetch=3'h1, Decode=3'h2, Execute=3'h3, WriteBack=3'h4;

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

  task regSet(input [3:0] i, input [31:0] data, input [2:0] itype); begin
    if (i != 0) R[i] = data;
    if (i == 12) itype = `MODE;
  end endtask

  task regHILOSet(input [31:0] data1, input [31:0] data2); begin
    HI = data1;
    LO = data2;
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
      ADDu:  regSet(a, Rb+Rc, itype);               // ADDu Ra,Rb,Rc; Ra<=Rb+Rc
      ADD:   begin regSet(a, Rb+Rc, itype); if (a < Rb) `V = 1; else `V =0; end
                                             // ADD Ra,Rb,Rc; Ra<=Rb+Rc
      SUBu:  regSet(a, Rb-Rc, itype);               // SUBu Ra,Rb,Rc; Ra<=Rb-Rc
      SUB:   begin regSet(a, Rb-Rc, itype); if (Rb < 0 && Rc > 0 && a >= 0) 
             `V = 1; else `V =0; end         // SUB Ra,Rb,Rc; Ra<=Rb-Rc
      MUL:   regSet(a, Rb*Rc, itype);               // MUL Ra,Rb,Rc;     Ra<=Rb*Rc
      SDIV:  regHILOSet(Ra%Rb, Ra/Rb);          // SDIV Ra,Rb; HI<=Ra%Rb; LO<=Ra/Rb
                                           // with exception overflow
      AND:   regSet(a, Rb&Rc, itype);               // AND Ra,Rb,Rc; Ra<=(Rb and Rc)
      ANDi:  regSet(a, Rb&uc16, itype);             // ANDi Ra,Rb,c16; Ra<=(Rb and c16)
      OR:    regSet(a, Rb|Rc, itype);               // OR Ra,Rb,Rc; Ra<=(Rb or Rc)
      ORi:   regSet(a, Rb|uc16, itype);             // ORi Ra,Rb,c16; Ra<=(Rb or c16)
      XOR:   regSet(a, Rb^Rc, itype);               // XOR Ra,Rb,Rc; Ra<=(Rb xor Rc)
      XORi:  regSet(a, Rb^uc16, itype);             // XORi Ra,Rb,c16; Ra<=(Rb xor c16)
      SHL:   regSet(a, Rb<<c5, itype);     // Shift Left; SHL Ra,Rb,Cx; Ra<=(Rb << Cx)
      SRA:   regSet(a, (Rb&'h80000000)|(Rb>>c5), itype); 
                                  // Shift Right with signed bit fill;
                                  // SHR Ra,Rb,Cx; Ra<=(Rb&0x80000000)|(Rb>>Cx)
      SHR:   regSet(a, Rb>>c5, itype);     // Shift Right with 0 fill; 
                                  // SHR Ra,Rb,Cx; Ra<=(Rb >> Cx)
      ROL:   regSet(a, (Rb<<c5)|(Rb>>(32-c5)), itype);     // Rotate Left;
      ROR:   regSet(a, (Rb>>c5)|(Rb<<(32-c5)), itype);     // Rotate Right;
      MFLO:  regSet(a, LO, itype);            // MFLO Ra; Ra<=LO
      MFHI:  regSet(a, HI, itype);            // MFHI Ra; Ra<=HI
      MTLO:  LO = Ra;             // MTLO Ra; LO<=Ra
      MTHI:  HI = Ra;             // MTHI Ra; HI<=Ra
      MULT:  {HI, LO}=Ra*Rb; // MULT Ra,Rb; HI<=((Ra*Rb)>>32); 
                            // LO<=((Ra*Rb) and 0x00000000ffffffff);
                            // with exception overflow
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
      JALR:  begin `LR=`PC;`PC=Ra; end // JALR Ra,Rb; Ra<=PC; PC<=Rb
      RET:   begin `PC=`LR; end               // RET; PC <= LR
      IRET:  begin 
        `PC=Ra;`I = 1'b0; `MODE = `EXE;
      end // Interrupt Return; IRET; PC <= LR; INT<=0
      endcase
      next_state = WriteBack;
    end
    WriteBack: begin // Read/Write finish, close memory
      case (op)
        LD, LB, LBu, LH, LHu  : memReadEnd(R[a]); 
                                          //read memory complete
        ST, SB, SH: memWriteEnd(); 
                                          // write memory complete
      endcase
      case (op)
      MULT, SDIV, MTHI, MTLO :
        $display("%4dns %8x : %8x HI=%8x LO=%8x SW=%8x", $stime, pc0, ir, HI, 
        LO, `SW);
      LD :
          $display("%4dns %8x : %8x m[%-04d+%-04d]=%-d  SW=%8x", $stime, pc0, ir, 
          R[b], c16, R[a], `SW);
      ST :
        if (R[b]+c16 == `IOADDR)
          $display("%4dns %8x : %8x OUTPUT=%-d", $stime, pc0, ir, R[a]);
        else
          $display("%4dns %8x : %8x m[%-04d+%-04d]=%-d  SW=%8x", $stime, pc0, ir, 
          R[b], c16, R[a], `SW);
      default : 
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
      taskExecute();
      state = next_state;
    end
    pc = `PC;
  end
endmodule

module memory0(input clock, reset, en, rw, input [1:0] m_size, 
                input [31:0] abus, dbus_in, output [31:0] dbus_out);
  reg [7:0] m [0:`MEMSIZE-1];
  reg [31:0] data;

  integer i;
  initial begin
  // erase memory
    for (i=0; i < `MEMSIZE; i=i+1) begin
       m[i] = `MEMEMPTY;
    end
  // display memory contents
    $readmemh("cpu0s.hex", m);
    for (i=0; i < `MEMSIZE && (m[i] != `MEMEMPTY || m[i+1] != `MEMEMPTY || m[i+2] != `MEMEMPTY || m[i+3] != `MEMEMPTY); i=i+4) begin
       $display("%8x: %8x", i, {m[i], m[i+1], m[i+2], m[i+3]});
    end
  end

  always @(clock or abus or en or rw or dbus_in) 
  begin
    if (abus >=0 && abus <= `MEMSIZE-4) begin
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
    #300000 $finish;
  end

/*  always #1000 begin
    itype = `IRQ; #200 itype = `EXE;
  end
*/
  always #10 clock=clock+1;

endmodule

