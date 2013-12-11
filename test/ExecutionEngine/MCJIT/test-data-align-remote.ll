<<<<<<< HEAD:test/ExecutionEngine/MCJIT/test-data-align-remote.ll
; RUN:  %lli_mcjit -remote-mcjit -O0 %s
; XFAIL: armv7, mips
=======
; RUN:  %lli_mcjit -remote-mcjit -O0 -mcjit-remote-process=lli-child-target %s
>>>>>>> llvmtrunk/master:test/ExecutionEngine/MCJIT/remote/test-data-align-remote.ll

; Check that a variable is always aligned as specified.

@var = global i32 0, align 32
define i32 @main() {
  %addr = ptrtoint i32* @var to i64
  %mask = and i64 %addr, 31
  %tst = icmp eq i64 %mask, 0
  br i1 %tst, label %good, label %bad
good:
  ret i32 0
bad:
  ret i32 1
}
