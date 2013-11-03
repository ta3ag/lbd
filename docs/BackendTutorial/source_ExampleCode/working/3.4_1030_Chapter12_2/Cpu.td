//===-- Cpu.td - Describe the Cpu Target Machine ---------*- tablegen -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
// This is the top level entry point for the Cpu target.
//===----------------------------------------------------------------------===//

//===----------------------------------------------------------------------===//
// Target-independent interfaces
//===----------------------------------------------------------------------===//

include "llvm/Target/Target.td"

//===----------------------------------------------------------------------===//
// Register File, Calling Conv, Instruction Descriptions
//===----------------------------------------------------------------------===//

include "CpuRegisterInfo.td"
include "CpuSchedule.td"
include "CpuInstrInfo.td"
include "CpuCallingConv.td"

def CpuInstrInfo : InstrInfo;

// Without this will have error: 'cpu32' is not a recognized processor for 
//  this target (ignoring processor)
//===----------------------------------------------------------------------===//
// Cpu Subtarget features                                                    //
//===----------------------------------------------------------------------===//

def FeatureCpu32      : SubtargetFeature<"cpu32", "CpuArchVersion", "Cpu32",
                                "Cpu32 ISA Support">;

//===----------------------------------------------------------------------===//
// Cpu processors supported.
//===----------------------------------------------------------------------===//

class Proc<string Name, list<SubtargetFeature> Features>
 : Processor<Name, CpuGenericItineraries, Features>;

def : Proc<"cpu32", [FeatureCpu32]>;

def CpuAsmWriter : AsmWriter {
  string AsmWriterClassName  = "InstPrinter";
  bit isMCAsmWriter = 1;
}

def CpuAsmParser : AsmParser {
  let ShouldEmitMatchRegisterName = 0;
}

def CpuAsmParserVariant : AsmParserVariant {
  int Variant = 0;

  // Recognize hard coded registers.
  string RegisterPrefix = "$";
}

// Will generate CpuGenAsmWrite.inc included by CpuInstPrinter.cpp, contents 
//  as follows,
// void CpuInstPrinter::printInstruction(const MCInst *MI, raw_ostream &O) {...}
// const char *CpuInstPrinter::getRegisterName(unsigned RegNo) {...}
def Cpu : Target {
// def CpuInstrInfo : InstrInfo as before.
  let InstructionSet = CpuInstrInfo;
  let AssemblyParsers = [CpuAsmParser];
  let AssemblyWriters = [CpuAsmWriter];
  let AssemblyParserVariants = [CpuAsmParserVariant];
}
