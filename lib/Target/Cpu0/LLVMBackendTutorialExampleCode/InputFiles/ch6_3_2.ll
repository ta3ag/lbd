define zeroext i1 @verify_load_bool() #0 {
entry:
  %retval = alloca i1, align 1
  %0 = load i1* %retval
  ret i1 %0
}
