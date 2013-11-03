include_directories( ${CMAKE_CURRENT_BINARY_DIR}/.. ${CMAKE_CURRENT_SOURCE_DIR}/.. )

add_llvm_library(LLVMCpuAsmPrinter
  CpuInstPrinter.cpp
  )

add_dependencies(LLVMCpuAsmPrinter CpuCommonTableGen)
