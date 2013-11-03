//===-- CpuRegisterInfo.td - Cpu Register defs -----------*- tablegen -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

//===----------------------------------------------------------------------===//
//  Declarations that describe the CPU register file
//===----------------------------------------------------------------------===//


// We have banks of 16 registers each.
class CpuReg<string n> : Register<n> {
  field bits<4> Num;
  let Namespace = "Cpu";
}

// Cpu CPU Registers
class CpuGPRReg<bits<4> num, string n> : CpuReg<n> {
  let Num = num;
}

//===----------------------------------------------------------------------===//
//  Registers
//===----------------------------------------------------------------------===//
// The register string, such as "9" or "gp" will show on "llvm-objdump -d"
let Namespace = "Cpu" in {
  // General Purpose Registers
  def ZERO : CpuGPRReg<0,  "zero">, DwarfRegNum<[0]>;
  def AT   : CpuGPRReg<1,  "1">,    DwarfRegNum<[1]>;
  def V0   : CpuGPRReg<2,  "2">,    DwarfRegNum<[2]>;
  def V1   : CpuGPRReg<3,  "3">,    DwarfRegNum<[3]>;
  def A0   : CpuGPRReg<4,  "4">,    DwarfRegNum<[4]>;
  def A1   : CpuGPRReg<5,  "5">,    DwarfRegNum<[5]>;
  def T9   : CpuGPRReg<6,  "t9">,   DwarfRegNum<[6]>;
  def T0   : CpuGPRReg<7,  "t0">,   DwarfRegNum<[7]>;
  def S0   : CpuGPRReg<8,  "8">,    DwarfRegNum<[8]>;
  def S1   : CpuGPRReg<9,  "9">,    DwarfRegNum<[9]>;
  def S2   : CpuGPRReg<10, "10">,   DwarfRegNum<[10]>;
  def GP   : CpuGPRReg<11, "gp">,   DwarfRegNum<[11]>;
  def FP   : CpuGPRReg<12, "fp">,   DwarfRegNum<[12]>;
  def SP   : CpuGPRReg<13, "sp">,   DwarfRegNum<[13]>;
  def LR   : CpuGPRReg<14, "lr">,   DwarfRegNum<[14]>;
  def PC   : CpuGPRReg<15, "pc">,   DwarfRegNum<[15]>;
//  def MAR  : Register< 16, "mar">,  DwarfRegNum<[16]>;
//  def MDR  : Register< 17, "mdr">,  DwarfRegNum<[17]>;

  // Hi/Lo registers
  def HI   : Register<"hi">, DwarfRegNum<[18]>;
  def LO   : Register<"lo">, DwarfRegNum<[19]>;
  def SW   : Register<"sw">, DwarfRegNum<[20]>;
}

//===----------------------------------------------------------------------===//
// Register Classes
//===----------------------------------------------------------------------===//

def CPURegs : RegisterClass<"Cpu", [i32], 32, (add
  // Reserved
  ZERO, AT, 
  // Return Values and Arguments
  V0, V1, A0, A1, 
  // Not preserved across procedure calls
  T9, T0,
  // Callee save
  S0, S1, S2, 
  // Reserved
  GP, FP, 
  SP, LR, PC)>;

// Hi/Lo Registers
def HILO : RegisterClass<"Cpu", [i32], 32, (add HI, LO)>;

