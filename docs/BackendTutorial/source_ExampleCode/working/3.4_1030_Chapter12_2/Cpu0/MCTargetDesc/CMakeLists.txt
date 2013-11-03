# MCTargetDesc/CMakeLists.txt
add_llvm_library(LLVMCpuDesc
  CpuAsmBackend.cpp
  CpuMCAsmInfo.cpp
  CpuMCCodeEmitter.cpp
  CpuMCTargetDesc.cpp
  CpuELFObjectWriter.cpp
  )

add_dependencies(LLVMCpuDesc CpuCommonTableGen)
