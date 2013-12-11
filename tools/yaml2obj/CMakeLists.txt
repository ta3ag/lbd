<<<<<<< HEAD
=======
set(LLVM_LINK_COMPONENTS
  Object
  Support
  )

>>>>>>> llvmtrunk/master
add_llvm_utility(yaml2obj
  yaml2obj.cpp
  yaml2coff.cpp
  yaml2elf.cpp
  )

target_link_libraries(yaml2obj LLVMSupport)
