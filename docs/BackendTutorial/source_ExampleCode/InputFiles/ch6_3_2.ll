; /Users/Jonathan/llvm/test/cmake_debug_build/bin/Debug/llc -march=cpu0 -relocation-model=pic -filetype=asm ch6_3_2.ll -o -

; /// start
define zeroext i1 @verify_load_bool() #0 {
entry:
  %retval = alloca i1, align 1
  %0 = load i1* %retval
  ret i1 %0
}
