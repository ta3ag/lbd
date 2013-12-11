add_llvm_library(LLVMInstrumentation
  AddressSanitizer.cpp
  BoundsChecking.cpp
<<<<<<< HEAD
  EdgeProfiling.cpp
=======
  DataFlowSanitizer.cpp
  DebugIR.cpp
>>>>>>> llvmtrunk/master
  GCOVProfiling.cpp
  MemorySanitizer.cpp
  Instrumentation.cpp
  ThreadSanitizer.cpp
  )

add_dependencies(LLVMInstrumentation intrinsics_gen)
