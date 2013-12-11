<<<<<<< HEAD:test/ExecutionEngine/MCJIT/simpletest-remote.ll
; RUN: %lli_mcjit -remote-mcjit %s > /dev/null
; XFAIL: arm, mips
=======
; RUN: %lli_mcjit -remote-mcjit -mcjit-remote-process=lli-child-target %s > /dev/null
>>>>>>> llvmtrunk/master:test/ExecutionEngine/MCJIT/remote/simpletest-remote.ll

define i32 @bar() {
	ret i32 0
}

define i32 @main() {
	%r = call i32 @bar( )		; <i32> [#uses=1]
	ret i32 %r
}

