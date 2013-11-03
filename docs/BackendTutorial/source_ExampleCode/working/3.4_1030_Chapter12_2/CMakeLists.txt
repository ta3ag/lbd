# CMakeLists.txt
# Our td all in Cpu.td, CpuRegisterInfo.td and CpuInstrInfo.td included in 
#  Cpu.td.
set(LLVM_TARGET_DEFINITIONS Cpu.td)

# Generate CpuGenRegisterInfo.inc and CpuGenInstrInfo.inc which included by 
#  your hand code C++ files. 
# CpuGenRegisterInfo.inc came from CpuRegisterInfo.td, CpuGenInstrInfo.inc 
#  came from CpuInstrInfo.td.
tablegen(LLVM CpuGenRegisterInfo.inc -gen-register-info)
tablegen(LLVM CpuGenInstrInfo.inc -gen-instr-info)
tablegen(LLVM CpuGenDisassemblerTables.inc -gen-disassembler)
tablegen(LLVM CpuGenCodeEmitter.inc -gen-emitter)
tablegen(LLVM CpuGenMCCodeEmitter.inc -gen-emitter -mc-emitter)

tablegen(LLVM CpuGenAsmWriter.inc -gen-asm-writer)
tablegen(LLVM CpuGenDAGISel.inc -gen-dag-isel)
tablegen(LLVM CpuGenCallingConv.inc -gen-callingconv)
tablegen(LLVM CpuGenSubtargetInfo.inc -gen-subtarget)
tablegen(LLVM CpuGenAsmMatcher.inc -gen-asm-matcher)

# CpuCommonTableGen must be defined
add_public_tablegen_target(CpuCommonTableGen)

# CpuCodeGen should match with LLVMBuild.txt CpuCodeGen
add_llvm_target(CpuCodeGen
  CpuAnalyzeImmediate.cpp
  CpuAsmPrinter.cpp
  CpuDelUselessJMP.cpp
  CpuEmitGPRestore.cpp
  CpuInstrInfo.cpp
  CpuISelDAGToDAG.cpp
  CpuISelLowering.cpp
  CpuFrameLowering.cpp
  CpuMCInstLower.cpp
  CpuMachineFunction.cpp
  CpuRegisterInfo.cpp
  CpuSubtarget.cpp
  CpuTargetMachine.cpp
  CpuTargetObjectFile.cpp
  CpuSelectionDAGInfo.cpp
  )

# Should match with "subdirectories =  MCTargetDesc TargetInfo" in LLVMBuild.txt
add_subdirectory(InstPrinter)
add_subdirectory(Disassembler)
add_subdirectory(TargetInfo)
add_subdirectory(MCTargetDesc)
add_subdirectory(AsmParser)
