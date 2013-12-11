//===- MCJITTest.cpp - Unit tests for the MCJIT ---------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This test suite verifies basic MCJIT functionality when invoked form the C
// API.
//
//===----------------------------------------------------------------------===//

#include "llvm-c/Analysis.h"
#include "llvm-c/Core.h"
#include "llvm-c/ExecutionEngine.h"
#include "llvm-c/Target.h"
#include "llvm-c/Transforms/Scalar.h"
#include "llvm/Support/Host.h"
#include "MCJITTestAPICommon.h"
#include "gtest/gtest.h"

using namespace llvm;

<<<<<<< HEAD
=======
static bool didCallAllocateCodeSection;

static uint8_t *roundTripAllocateCodeSection(void *object, uintptr_t size,
                                             unsigned alignment,
                                             unsigned sectionID,
                                             const char *sectionName) {
  didCallAllocateCodeSection = true;
  return static_cast<SectionMemoryManager*>(object)->allocateCodeSection(
    size, alignment, sectionID, sectionName);
}

static uint8_t *roundTripAllocateDataSection(void *object, uintptr_t size,
                                             unsigned alignment,
                                             unsigned sectionID,
                                             const char *sectionName,
                                             LLVMBool isReadOnly) {
  return static_cast<SectionMemoryManager*>(object)->allocateDataSection(
    size, alignment, sectionID, sectionName, isReadOnly);
}

static LLVMBool roundTripFinalizeMemory(void *object, char **errMsg) {
  std::string errMsgString;
  bool result =
    static_cast<SectionMemoryManager*>(object)->finalizeMemory(&errMsgString);
  if (result) {
    *errMsg = LLVMCreateMessage(errMsgString.c_str());
    return 1;
  }
  return 0;
}

static void roundTripDestroy(void *object) {
  delete static_cast<SectionMemoryManager*>(object);
}

namespace {
>>>>>>> llvmtrunk/master
class MCJITCAPITest : public testing::Test, public MCJITTestAPICommon {
protected:
  MCJITCAPITest() {
    // The architectures below are known to be compatible with MCJIT as they
    // are copied from test/ExecutionEngine/MCJIT/lit.local.cfg and should be
    // kept in sync.
    SupportedArchs.push_back(Triple::arm);
    SupportedArchs.push_back(Triple::mips);
    SupportedArchs.push_back(Triple::x86);
    SupportedArchs.push_back(Triple::x86_64);

    // The operating systems below are known to be sufficiently incompatible
    // that they will fail the MCJIT C API tests.
    UnsupportedOSs.push_back(Triple::Cygwin);
  }
};

TEST_F(MCJITCAPITest, simple_function) {
  SKIP_UNSUPPORTED_PLATFORM;
  
  char *error = 0;
  
  // Creates a function that returns 42, compiles it, and runs it.
  
  LLVMModuleRef module = LLVMModuleCreateWithName("simple_module");

  LLVMSetTarget(module, HostTriple.c_str());
  
  LLVMValueRef function = LLVMAddFunction(
    module, "simple_function", LLVMFunctionType(LLVMInt32Type(), 0, 0, 0));
  LLVMSetFunctionCallConv(function, LLVMCCallConv);
  
  LLVMBasicBlockRef entry = LLVMAppendBasicBlock(function, "entry");
  LLVMBuilderRef builder = LLVMCreateBuilder();
  LLVMPositionBuilderAtEnd(builder, entry);
  LLVMBuildRet(builder, LLVMConstInt(LLVMInt32Type(), 42, 0));
  
  LLVMVerifyModule(module, LLVMAbortProcessAction, &error);
  LLVMDisposeMessage(error);
  
<<<<<<< HEAD
  LLVMDisposeBuilder(builder);
=======
  LLVMModuleRef Module;
  LLVMValueRef Function;
  LLVMMCJITCompilerOptions Options;
  LLVMExecutionEngineRef Engine;
  char *Error;
};
} // end anonymous namespace

TEST_F(MCJITCAPITest, simple_function) {
  SKIP_UNSUPPORTED_PLATFORM;
>>>>>>> llvmtrunk/master
  
  LLVMMCJITCompilerOptions options;
  LLVMInitializeMCJITCompilerOptions(&options, sizeof(options));
  options.OptLevel = 2;
  
  // Just ensure that this field still exists.
  options.NoFramePointerElim = false;
  
  LLVMExecutionEngineRef engine;
  ASSERT_EQ(
    0, LLVMCreateMCJITCompilerForModule(&engine, module, &options,
                                        sizeof(options), &error));
  
  LLVMPassManagerRef pass = LLVMCreatePassManager();
  LLVMAddTargetData(LLVMGetExecutionEngineTargetData(engine), pass);
  LLVMAddConstantPropagationPass(pass);
  LLVMAddInstructionCombiningPass(pass);
  LLVMRunPassManager(pass, module);
  LLVMDisposePassManager(pass);
  
  union {
    void *raw;
    int (*usable)();
  } functionPointer;
  functionPointer.raw = LLVMGetPointerToGlobal(engine, function);
  
  EXPECT_EQ(42, functionPointer.usable());
  
  LLVMDisposeExecutionEngine(engine);
}

