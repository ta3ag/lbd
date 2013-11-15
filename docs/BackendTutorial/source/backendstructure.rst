.. _sec-backendstructure:

Backend structure
==================

This chapter introduce the back end class inherit tree and class members first. 
Next, following the back end structure, adding individual class implementation 
in each section. 
There are compiler knowledge like DAG (Directed-Acyclic-Graph) and instruction 
selection needed in this chapter. 
This chapter explains these knowledge just when needed. 
At the end of this chapter, we will have a back end to compile llvm 
intermediate code into cpu0 assembly code.

Many code are added in this chapter. They almost are common in every back end 
except the back end name (cpu0 or mips ...). Actually, we copy almost all the 
code from mips and replace the name with cpu0. Please focus on the classes 
relationship in this backend structure. Once knowing the structure, you can 
create your backend structure as quickly as we did, even though there are 3000 
lines of code in this chapter.

TargetMachine structure
-----------------------

Your back end should define a TargetMachine class, for example, we define the 
Cpu0TargetMachine class. 
Cpu0TargetMachine class contains it's own instruction class, frame/stack class, 
DAG (Directed-Acyclic-Graph) class, and register class. 
The Cpu0TargetMachine contents and it's own class as follows,

.. rubric:: include/llvm/Target/TargetMachine.h
.. code-block:: c++

  //- TargetMachine.h 
  class TargetMachine { 
    TargetMachine(const TargetMachine &) LLVM_DELETED_FUNCTION;
    void operator=(const TargetMachine &) LLVM_DELETED_FUNCTION;
  ...
  public: 
    // Interfaces to the major aspects of target machine information: 
    // -- Instruction opcode and operand information 
    // -- Pipelines and scheduling information 
    // -- Stack frame information 
    // -- Selection DAG lowering information 
    // 
    virtual const TargetInstrInfo         *getInstrInfo() const { return 0; } 
    virtual const TargetFrameLowering *getFrameLowering() const { return 0; } 
    virtual const TargetLowering    *getTargetLowering() const { return 0; } 
    virtual const TargetSelectionDAGInfo *getSelectionDAGInfo() const{ return 0; } 
    virtual const DataLayout             *getDataLayout() const { return 0; } 
    ... 
    /// getSubtarget - This method returns a pointer to the specified type of 
    /// TargetSubtargetInfo.  In debug builds, it verifies that the object being 
    /// returned is of the correct type. 
    template<typename STC> const STC &getSubtarget() const { 
    return *static_cast<const STC*>(getSubtargetImpl()); 
    } 
  
  } 
  ...
  class LLVMTargetMachine : public TargetMachine { 
  protected: // Can only create subclasses. 
    LLVMTargetMachine(const Target &T, StringRef TargetTriple, 
            StringRef CPU, StringRef FS, TargetOptions Options, 
            Reloc::Model RM, CodeModel::Model CM, 
            CodeGenOpt::Level OL); 
    ... 
  }; 


.. rubric:: lbdex/Chapter3_1/Cpu0TargetObjectFile.h
.. literalinclude:: ../../../lib/Target/Cpu0/Cpu0TargetObjectFile.h
    :end-before: const MCSection *SmallDataSection;
.. literalinclude:: ../../../lib/Target/Cpu0/Cpu0TargetObjectFile.h
    :start-after: // TODO: Classify globals as cpu0 wishes.

.. rubric:: lbdex/Chapter3_1/Cpu0TargetObjectFile.cpp
.. literalinclude:: ../../../lib/Target/Cpu0/Cpu0TargetObjectFile.cpp
    :end-before: // A address must be loaded from a small section

.. rubric:: lbdex/Chapter3_1/Cpu0TargetMachine.h
.. literalinclude:: ../../../lib/Target/Cpu0/Cpu0TargetMachine.h

.. rubric:: lbdex/Chapter3_1/Cpu0TargetMachine.cpp
.. literalinclude:: ../../../lib/Target/Cpu0/Cpu0TargetMachine.cpp
    :start-after: // Implements the info about Cpu0 target spec.
    :end-before: #include "llvm/Support/TargetRegistry.h"
.. literalinclude:: ../../../lib/Target/Cpu0/Cpu0TargetMachine.cpp
    :start-after: using namespace llvm;
    :end-before: virtual bool addInstSelector();
.. literalinclude:: ../../../lib/Target/Cpu0/Cpu0TargetMachine.cpp
    :start-after: virtual bool addPreEmitPass();
    :end-before: // Install an instruction selector pass
  
.. rubric:: include/llvm/Target/TargetInstInfo.h
.. code-block:: c++

  class TargetInstrInfo : public MCInstrInfo { 
    TargetInstrInfo(const TargetInstrInfo &) LLVM_DELETED_FUNCTION;
    void operator=(const TargetInstrInfo &) LLVM_DELETED_FUNCTION;
  public: 
    ... 
  }
  ...
  class TargetInstrInfoImpl : public TargetInstrInfo { 
  protected: 
    TargetInstrInfoImpl(int CallFrameSetupOpcode = -1, 
              int CallFrameDestroyOpcode = -1) 
    : TargetInstrInfo(CallFrameSetupOpcode, CallFrameDestroyOpcode) {} 
  public: 
    ... 
  } 
  

.. rubric:: lbdex/Chapter3_1/Cpu0.td
.. literalinclude:: ../../../lib/Target/Cpu0/Cpu0.td
    :start-after: include "Cpu0InstrInfo.td"
    :end-before: def Cpu0InstrInfo : InstrInfo;

.. rubric:: lbdex/Chapter3_1/Cpu0CallingConv.td
.. literalinclude:: ../../../lib/Target/Cpu0/Cpu0CallingConv.td
    :end-before: def RetCC_Cpu0EABI : CallingConv<[
.. literalinclude:: ../../../lib/Target/Cpu0/Cpu0CallingConv.td
    :start-after: // lbd document - mark - def RetCC_Cpu0

.. rubric:: lbdex/Chapter3_1/Cpu0FrameLowering.h
.. literalinclude:: ../../../lib/Target/Cpu0/Cpu0FrameLowering.h
    :end-before: void eliminateCallFramePseudoInstr(
.. literalinclude:: ../../../lib/Target/Cpu0/Cpu0FrameLowering.h
    :start-after: RegScavenger *RS) const;

.. rubric:: lbdex/Chapter3_1/Cpu0FrameLowering.cpp
.. literalinclude:: ../../../lib/Target/Cpu0/Cpu0FrameLowering.cpp
    :end-before: #include "Cpu0AnalyzeImmediate.h"
.. literalinclude:: ../../../lib/Target/Cpu0/Cpu0FrameLowering.cpp
    :start-after: #include "Cpu0AnalyzeImmediate.h"
    :end-before: #include "MCTargetDesc/Cpu0BaseInfo.h"
.. literalinclude:: ../../../lib/Target/Cpu0/Cpu0FrameLowering.cpp
    :start-after: #include "MCTargetDesc/Cpu0BaseInfo.h"
    :end-before: // Build an instruction sequence to load an immediate 
.. literalinclude:: ../../../lib/Target/Cpu0/Cpu0FrameLowering.cpp
    :start-after: // lbd document - mark - expandLargeImm
    :end-before: MachineBasicBlock &MBB   = MF.front();
.. literalinclude:: ../../../lib/Target/Cpu0/Cpu0FrameLowering.cpp
    :start-after: // lbd document - mark - if (Cpu0FI->needGPSaveRestore())
    :end-before: MachineBasicBlock::iterator MBBI = MBB.getLastNonDebugInstr();
.. code-block:: c++

    }

.. rubric:: lbdex/Chapter3_1/Cpu0InstrInfo.h
.. literalinclude:: ../../../lib/Target/Cpu0/Cpu0InstrInfo.h
    :end-before: virtual void copyPhysReg(MachineBasicBlock &MBB,
.. literalinclude:: ../../../lib/Target/Cpu0/Cpu0InstrInfo.h
    :start-after: unsigned Opc) const;

.. rubric:: lbdex/Chapter3_1/Cpu0InstrInfo.cpp
.. literalinclude:: ../../../lib/Target/Cpu0/Cpu0InstrInfo.cpp
    :end-before: #include "Cpu0MachineFunction.h"
.. literalinclude:: ../../../lib/Target/Cpu0/Cpu0InstrInfo.cpp
    :start-after: #include "llvm/CodeGen/MachineInstrBuilder.h"
    :end-before: Cpu0GenInstrInfo(Cpu0::ADJCALLSTACKDOWN, Cpu0::ADJCALLSTACKUP),
.. literalinclude:: ../../../lib/Target/Cpu0/Cpu0InstrInfo.cpp
    :start-after: Cpu0GenInstrInfo(Cpu0::ADJCALLSTACKDOWN, Cpu0::ADJCALLSTACKUP),
    :end-before: void Cpu0InstrInfo::

.. rubric:: lbdex/Chapter3_1/Cpu0ISelLowering.h
.. literalinclude:: ../../../lib/Target/Cpu0/Cpu0ISelLowering.h
    :end-before: // Jump and link (call)
.. literalinclude:: ../../../lib/Target/Cpu0/Cpu0ISelLowering.h
    :start-after: ThreadPointer,
    :end-before:  // DivRem(u)
.. literalinclude:: ../../../lib/Target/Cpu0/Cpu0ISelLowering.h
    :start-after: Sync
    :end-before: virtual MVT getShiftAmountTy(EVT LHSTy) const { return MVT::i32; }
.. literalinclude:: ../../../lib/Target/Cpu0/Cpu0ISelLowering.h
    :start-after: unsigned HiFlag, unsigned LoFlag) const;
    :end-before: // Lower Operand helpers
.. literalinclude:: ../../../lib/Target/Cpu0/Cpu0ISelLowering.h
    :start-after: SDValue LowerVASTART(SDValue Op, SelectionDAG &DAG) const;
    :end-before: // LowerFormalArguments: incoming arguments
.. literalinclude:: ../../../lib/Target/Cpu0/Cpu0ISelLowering.h
    :start-after: // LowerCall: outgoing arguments
    :end-before: virtual bool isOffsetFoldingLegal(const GlobalAddressSDNode *GA) const;
.. literalinclude:: ../../../lib/Target/Cpu0/Cpu0ISelLowering.h
    :start-after: virtual bool isOffsetFoldingLegal(const GlobalAddressSDNode *GA) const;

.. rubric:: lbdex/Chapter3_1/Cpu0ISelLowering.cpp
.. literalinclude:: ../../../lib/Target/Cpu0/Cpu0ISelLowering.cpp
    :end-before: #include "Cpu0MachineFunction.h"
.. literalinclude:: ../../../lib/Target/Cpu0/Cpu0ISelLowering.cpp
    :start-after: #include "Cpu0MachineFunction.h"
    :end-before: #include "MCTargetDesc/Cpu0BaseInfo.h"
.. literalinclude:: ../../../lib/Target/Cpu0/Cpu0ISelLowering.cpp
    :start-after: #include "MCTargetDesc/Cpu0BaseInfo.h"
    :end-before: #include "llvm/Support/CommandLine.h"
.. literalinclude:: ../../../lib/Target/Cpu0/Cpu0ISelLowering.cpp
    :start-after: #include "llvm/Support/CommandLine.h"
    :end-before: SDValue Cpu0TargetLowering::getGlobalReg
.. literalinclude:: ../../../lib/Target/Cpu0/Cpu0ISelLowering.cpp
    :start-after: // lbd document - mark - getTargetNodeName
    :end-before: // Set up the register classes
.. literalinclude:: ../../../lib/Target/Cpu0/Cpu0ISelLowering.cpp
    :start-after: MachinePointerInfo(SV), false, false, 0);
    :end-before: //===---------------
.. literalinclude:: ../../../lib/Target/Cpu0/Cpu0ISelLowering.cpp
    :start-after: // lbd document - mark - ReadByValArg
    :end-before: MachineFunction &MF = DAG.getMachineFunction();
.. literalinclude:: ../../../lib/Target/Cpu0/Cpu0ISelLowering.cpp
    :start-after: } // if (!OutChains.empty())
    :end-before: // CCValAssign - represent the assignment of

.. code-block:: c++

    }

.. rubric:: lbdex/Chapter3_1/Cpu0MachineFunction.h
.. literalinclude:: ../../../lib/Target/Cpu0/Cpu0MachineFunction.h
    :end-before: virtual void anchor();
.. literalinclude:: ../../../lib/Target/Cpu0/Cpu0MachineFunction.h
    :start-after: virtual void anchor();
    :end-before: /// SRetReturnReg
.. literalinclude:: ../../../lib/Target/Cpu0/Cpu0MachineFunction.h
    :start-after: mutable int DynAllocFI;
    :end-before: bool EmitNOAT;
.. literalinclude:: ../../../lib/Target/Cpu0/Cpu0MachineFunction.h
    :start-after: bool EmitNOAT;
    :end-before: SRetReturnReg(0),
.. literalinclude:: ../../../lib/Target/Cpu0/Cpu0MachineFunction.h
    :start-after: EmitNOAT(false),
    :end-before: bool isInArgFI(int FI)
.. literalinclude:: ../../../lib/Target/Cpu0/Cpu0MachineFunction.h
    :start-after: void setVarArgsFrameIndex
    :end-before: bool getEmitNOAT
.. literalinclude:: ../../../lib/Target/Cpu0/Cpu0MachineFunction.h
    :start-after: void setEmitNOAT

.. rubric:: lbdex/Chapter3_1/Cpu0SelectionDAGInfo.h
.. literalinclude:: ../../../lib/Target/Cpu0/Cpu0SelectionDAGInfo.h

.. rubric:: lbdex/Chapter3_1/Cpu0SelectionDAGInfo.cpp
.. literalinclude:: ../../../lib/Target/Cpu0/Cpu0SelectionDAGInfo.cpp

.. rubric:: lbdex/Chapter3_1/Cpu0Subtarget.h
.. literalinclude:: ../../../lib/Target/Cpu0/Cpu0Subtarget.h
    :end-before: extern bool Cpu0NoCpload;
.. literalinclude:: ../../../lib/Target/Cpu0/Cpu0Subtarget.h
    :start-after: extern bool Cpu0NoCpload;
    :end-before: , Cpu032II,
.. literalinclude:: ../../../lib/Target/Cpu0/Cpu0Subtarget.h
    :start-after: Cpu032III
    :end-before: // UseSmallSection - Small section is used.
.. literalinclude:: ../../../lib/Target/Cpu0/Cpu0Subtarget.h
    :start-after: bool UseSmallSection;
    :end-before: bool hasCpu032I()
.. literalinclude:: ../../../lib/Target/Cpu0/Cpu0Subtarget.h
    :start-after:  bool useSmallSection()

.. rubric:: lbdex/Chapter3_1/Cpu0Subtarget.cpp
.. literalinclude:: ../../../lib/Target/Cpu0/Cpu0Subtarget.cpp
    :end-before: #include "llvm/Support/CommandLine.h"
.. literalinclude:: ../../../lib/Target/Cpu0/Cpu0Subtarget.cpp
    :start-after: #include "llvm/Support/CommandLine.h"
    :end-before: static cl::opt<bool> UseSmallSectionOpt
.. literalinclude:: ../../../lib/Target/Cpu0/Cpu0Subtarget.cpp
    :start-after: extern bool FixGlobalBaseReg;
    :end-before: // Set UseSmallSection.
.. literalinclude:: ../../../lib/Target/Cpu0/Cpu0Subtarget.cpp
    :start-after: FixGlobalBaseReg = true;

.. rubric:: lbdex/Chapter3_1/Cpu0RegisterInfo.h
.. literalinclude:: ../../../lib/Target/Cpu0/Cpu0RegisterInfo.h

.. rubric:: lbdex/Chapter3_1/Cpu0RegisterInfo.cpp
.. literalinclude:: ../../../lib/Target/Cpu0/Cpu0RegisterInfo.cpp
    :end-before: // Reserve FP if this function
.. literalinclude:: ../../../lib/Target/Cpu0/Cpu0RegisterInfo.cpp
    :start-after: Reserved.set(Cpu0::GP);
    :end-before: MachineInstr &MI = *II;
.. literalinclude:: ../../../lib/Target/Cpu0/Cpu0RegisterInfo.cpp
    :start-after: MI.getOperand(i+1).ChangeToImmediate(Offset);

.. rubric:: cmake_debug_build/lib/Target/Cpu0/Cpu0GenInstInfo.inc
.. code-block:: c++

  //- Cpu0GenInstInfo.inc which generate from Cpu0InstrInfo.td 
  #ifdef GET_INSTRINFO_HEADER 
  #undef GET_INSTRINFO_HEADER 
  namespace llvm { 
  struct Cpu0GenInstrInfo : public TargetInstrInfoImpl { 
    explicit Cpu0GenInstrInfo(int SO = -1, int DO = -1); 
  }; 
  } // End llvm namespace 
  #endif // GET_INSTRINFO_HEADER 
  
  #define GET_INSTRINFO_HEADER 
  #include "Cpu0GenInstrInfo.inc" 
  //- Cpu0InstInfo.h 
  class Cpu0InstrInfo : public Cpu0GenInstrInfo { 
    Cpu0TargetMachine &TM; 
  public: 
    explicit Cpu0InstrInfo(Cpu0TargetMachine &TM); 
  };

.. _backendstructure-f1: 
.. figure:: ../Fig/backendstructure/1.png
  :align: center

  TargetMachine class diagram 1

The Cpu0TargetMachine inherit tree is TargetMachine <- LLVMTargetMachine <- 
Cpu0TargetMachine. 
Cpu0TargetMachine has class Cpu0Subtarget, Cpu0InstrInfo, Cpu0FrameLowering, 
Cpu0TargetLowering and Cpu0SelectionDAGInfo. 
Class Cpu0Subtarget, Cpu0InstrInfo, Cpu0FrameLowering, Cpu0TargetLowering and 
Cpu0SelectionDAGInfo are inherited from parent class TargetSubtargetInfo, 
TargetInstrInfo, TargetFrameLowering, TargetLowering and TargetSelectionDAGInfo.

:num:`Figure #backendstructure-f1` shows Cpu0TargetMachine inherit tree and it's 
Cpu0InstrInfo class inherit tree. 
Cpu0TargetMachine contains Cpu0InstrInfo and ... other class. 
Cpu0InstrInfo contains Cpu0RegisterInfo class, RI. Cpu0InstrInfo.td and 
Cpu0RegisterInfo.td will generate Cpu0GenInstrInfo.inc and 
Cpu0GenRegisterInfo.inc which contain some member functions implementation for 
class Cpu0InstrInfo and Cpu0RegisterInfo.

:num:`Figure #backendstructure-f2` as below shows Cpu0TargetMachine contains 
class 
TSInfo: Cpu0SelectionDAGInfo, FrameLowering: Cpu0FrameLowering, Subtarget: 
Cpu0Subtarget and TLInfo: Cpu0TargetLowering.

.. _backendstructure-f2:  
.. figure:: ../Fig/backendstructure/2.png
  :align: center

  TargetMachine class diagram 2

.. _backendstructure-f3: 
.. figure:: ../Fig/backendstructure/3.png
  :align: center

  TargetMachine members and operators

:num:`Figure #backendstructure-f3` shows some members and operators (member function) 
of the parent class TargetMachine's. 
:num:`Figure #backendstructure-f4` as below shows some members of class InstrInfo, 
RegisterInfo and TargetLowering. 
Class DAGInfo is skipped here.

.. _backendstructure-f4: 
.. figure:: ../Fig/backendstructure/4.png
  :align: center

  Other class members and operators

Benefit from the inherit tree structure, we just need to implement few code in 
instruction, frame/stack, select DAG class. 
Many code implemented by their parent class. 
The llvm-tblgen generate Cpu0GenInstrInfo.inc from Cpu0InstrInfo.td. 
Cpu0InstrInfo.h extract those code it need from Cpu0GenInstrInfo.inc by define 
“#define GET_INSTRINFO_HEADER”. 
Following is the code fragment from Cpu0GenInstrInfo.inc. 
Code between “#if def  GET_INSTRINFO_HEADER” and “#endif // GET_INSTRINFO_HEADER” 
will be extracted by Cpu0InstrInfo.h.

.. rubric:: cmake_debug_build/lib/Target/Cpu0/Cpu0GenInstInfo.inc
.. code-block:: c++

  //- Cpu0GenInstInfo.inc which generate from Cpu0InstrInfo.td 
  #ifdef GET_INSTRINFO_HEADER 
  #undef GET_INSTRINFO_HEADER 
  namespace llvm { 
  struct Cpu0GenInstrInfo : public TargetInstrInfoImpl { 
    explicit Cpu0GenInstrInfo(int SO = -1, int DO = -1); 
  }; 
  } // End llvm namespace 
  #endif // GET_INSTRINFO_HEADER 

Reference Write An LLVM Backend web site [#]_.

Now, the code in Chapter3_1/ add class Cpu0TargetMachine(Cpu0TargetMachine.h and 
cpp), Cpu0Subtarget (Cpu0Subtarget.h and .cpp), Cpu0InstrInfo (Cpu0InstrInfo.h 
and .cpp), Cpu0FrameLowering (Cpu0FrameLowering.h and .cpp), Cpu0TargetLowering 
(Cpu0ISelLowering.h and .cpp) and Cpu0SelectionDAGInfo ( Cpu0SelectionDAGInfo.h 
and .cpp). 
CMakeLists.txt  modified with those new added \*.cpp as follows,

.. rubric:: lbdex/Chapter3_1/CMakeLists.txt
.. literalinclude:: ../../../lib/Target/Cpu0/CMakeLists.txt
    :start-after: tablegen(LLVM Cpu0GenInstrInfo.inc -gen-instr-info)
    :end-before: tablegen(LLVM Cpu0GenSubtargetInfo.inc -gen-subtarget)
.. literalinclude:: ../../../lib/Target/Cpu0/CMakeLists.txt
    :start-after: # Cpu0CodeGen should match with LLVMBuild.txt Cpu0CodeGen
    :end-before: Cpu0AnalyzeImmediate.cpp
.. literalinclude:: ../../../lib/Target/Cpu0/CMakeLists.txt
    :start-after: Cpu0EmitGPRestore.cpp
    :end-before: Cpu0ISelDAGToDAG.cpp
.. literalinclude:: ../../../lib/Target/Cpu0/CMakeLists.txt
    :start-after: Cpu0ISelDAGToDAG.cpp
    :end-before: Cpu0MCInstLower.cpp
.. literalinclude:: ../../../lib/Target/Cpu0/CMakeLists.txt
    :start-after: Cpu0MachineFunction.cpp
    :end-before: Cpu0TargetMachine.cpp
.. literalinclude:: ../../../lib/Target/Cpu0/CMakeLists.txt
    :start-after: Cpu0TargetMachine.cpp
    :end-before: # Should match with "subdirectories =  MCTargetDesc TargetInfo" in LLVMBuild.txt

Please take a look for Chapter3_1 code. 
After that, building Chapter3_1 by make as chapter 2 (of course, you should remove old 
src/lib/Target/Cpu0 and replace them with src/lib/Target/Cpu0/lbdex/Chapter3_1/). 
You can remove cmake_debug_build/lib/Target/Cpu0/\*.inc before do “make” to ensure your code 
rebuild completely. 
By remove \*.inc, all files those have included .inc will be rebuild, then your 
Target library will be regenerated. 
Command as follows,

.. code-block:: bash

  118-165-78-230:cmake_debug_build Jonathan$ rm -rf lib/Target/Cpu0/*


Now, let's build Chapter3_1 as the following command, 

.. code-block:: bash

  118-165-75-57:ExampleCode Jonathan$ pwd
  /Users/Jonathan/llvm/test/src/lib/Target/Cpu0/lbdex
  118-165-75-57:lbdex Jonathan$ sh removecpu0.sh 
  118-165-75-57:lbdex Jonathan$ cp -rf Chapter3_1/
  * ../.
  
  118-165-78-230:InputFiles Jonathan$ /Users/Jonathan/llvm/test/cmake_debug_build/
  bin/Debug/llc -march=cpu0 -relocation-model=pic -filetype=asm ch3.bc -o 
  ch3.cpu0.s
  Assertion failed: (AsmInfo && "MCAsmInfo not initialized." 
  ...


The errors say that we have not Target AsmPrinter. 
Let's add it in next section.


Add AsmPrinter
--------------

Chapter3_2/cpu0 contains the Cpu0AsmPrinter definition. First, we add definitions in 
Cpu0.td to support AssemblyWriter. 
Cpu0.td is added with the following fragment,

.. rubric:: lbdex/Chapter3_2/Cpu0.td
.. literalinclude:: ../../../lib/Target/Cpu0/Cpu0.td
    :start-after: def Cpu0InstrInfo : InstrInfo;
    :end-before: def FeatureCpu032II
.. literalinclude:: ../../../lib/Target/Cpu0/Cpu0.td
    :start-after: "Cpu032III", "Cpu032III ISA Support (use instruction slt)">;
    :end-before: def : Proc<"cpu032II", [FeatureCpu032II]>;
.. literalinclude:: ../../../lib/Target/Cpu0/Cpu0.td
    :start-after: def : Proc<"cpu032III", [FeatureCpu032III]>;
    :end-before: def Cpu0AsmParser : AsmParser {
.. literalinclude:: ../../../lib/Target/Cpu0/Cpu0.td
    :start-after: } // def Cpu0AsmParserVariant
    :end-before: // def Cpu0InstrInfo : InstrInfo as before.
.. literalinclude:: ../../../lib/Target/Cpu0/Cpu0.td
    :start-after: let AssemblyParsers = [Cpu0AsmParser];
    :end-before: let AssemblyParserVariants = [Cpu0AsmParserVariant];
.. literalinclude:: ../../../lib/Target/Cpu0/Cpu0.td
    :start-after: let AssemblyParserVariants = [Cpu0AsmParserVariant];


As comments indicate, it will generate Cpu0GenAsmWrite.inc which is included 
by Cpu0InstPrinter.cpp as follows,

.. rubric:: lbdex/Chapter3_2/InstPrinter/Cpu0InstPrinter.h
.. literalinclude:: ../../../lib/Target/Cpu0/InstPrinter/Cpu0InstPrinter.h
    :end-before: void printMemOperandEA(const MCInst *MI, int opNum, raw_ostream &O);
.. literalinclude:: ../../../lib/Target/Cpu0/InstPrinter/Cpu0InstPrinter.h
    :start-after: void printMemOperandEA(const MCInst *MI, int opNum, raw_ostream &O);
 
.. rubric:: lbdex/Chapter3_2/InstPrinter/Cpu0InstPrinter.cpp
.. literalinclude:: ../../../lib/Target/Cpu0/InstPrinter/Cpu0InstPrinter.cpp
    :end-before: // Cpu0_GPREL is for llc -march=cpu0 -relocation-model=static
.. literalinclude:: ../../../lib/Target/Cpu0/InstPrinter/Cpu0InstPrinter.cpp
    :start-after: case MCSymbolRefExpr::VK_Cpu0_GOT_LO16:  OS << "%got_lo("; break;
    :end-before: // lbd document - mark - printMemOperandEA

.. rubric:: lbdex/Chapter3_2/InstPrinter/CMakeLists.txt
.. literalinclude:: ../../../lib/Target/Cpu0/InstPrinter/CMakeLists.txt

.. rubric:: lbdex/Chapter3_2/InstPrinter/LLVMBuild.txt
.. literalinclude:: ../../../lib/Target/Cpu0/InstPrinter/LLVMBuild.txt

Cpu0GenAsmWrite.inc has the implementation of 
Cpu0InstPrinter::printInstruction() and Cpu0InstPrinter::getRegisterName(). 
Both of these functions can be auto-generated from the information we defined 
in Cpu0InstrInfo.td and Cpu0RegisterInfo.td. 
To let these two functions work in our code, the only thing need to do is add a 
class Cpu0InstPrinter and include them as did in Chapter3_1.

File Chapter3_1/Cpu0/InstPrinter/Cpu0InstPrinter.cpp include Cpu0GenAsmWrite.inc and 
call the auto-generated functions from TableGen.

Next, add Cpu0MCInstLower (Cpu0MCInstLower.h, Cpu0MCInstLower.cpp), as well as 
Cpu0BaseInfo.h, 
Cpu0FixupKinds.h and Cpu0MCAsmInfo (Cpu0MCAsmInfo.h, Cpu0MCAsmInfo.cpp) in 
sub-directory MCTargetDesc as follows,

.. rubric:: lbdex/Chapter3_2/Cpu0MCInstLower.h
.. literalinclude:: ../../../lib/Target/Cpu0/Cpu0MCInstLower.h
    :end-before: void LowerCPLOAD
.. literalinclude:: ../../../lib/Target/Cpu0/Cpu0MCInstLower.h
    :start-after: void LowerCPRESTORE

.. rubric:: lbdex/Chapter3_2/Cpu0MCInstLower.cpp
.. literalinclude:: ../../../lib/Target/Cpu0/Cpu0MCInstLower.cpp
    :end-before: MCOperand Cpu0MCInstLower::LowerSymbolOperand
.. literalinclude:: ../../../lib/Target/Cpu0/Cpu0MCInstLower.cpp
    :start-after: // lbd document - mark - LowerCPRESTORE
    :end-before: case MachineOperand::MO_MachineBasicBlock:
.. literalinclude:: ../../../lib/Target/Cpu0/Cpu0MCInstLower.cpp
    :start-after: return LowerSymbolOperand(MO, MOTy, offset);

.. rubric:: lbdex/Chapter3_2/MCTargetDesc/Cpu0BaseInfo.h
.. literalinclude:: ../../../lib/Target/Cpu0/MCTargetDesc/Cpu0BaseInfo.h
    :end-before: #include "Cpu0FixupKinds.h"
.. literalinclude:: ../../../lib/Target/Cpu0/MCTargetDesc/Cpu0BaseInfo.h
    :start-after: #include "Cpu0FixupKinds.h"
    :end-before: enum TOF {
.. literalinclude:: ../../../lib/Target/Cpu0/MCTargetDesc/Cpu0BaseInfo.h
    :start-after: }; // enum TOF {
    :end-before: case Cpu0::HI:
.. literalinclude:: ../../../lib/Target/Cpu0/MCTargetDesc/Cpu0BaseInfo.h
    :start-after: return 19;
    :end-before: inline static std::pair
.. literalinclude:: ../../../lib/Target/Cpu0/MCTargetDesc/Cpu0BaseInfo.h
    :start-after: } // Cpu0GetSymAndOffset

.. rubric:: lbdex/Chapter3_2/MCTargetDesc/Cpu0MCAsmInfo.h
.. literalinclude:: ../../../lib/Target/Cpu0/MCTargetDesc/Cpu0MCAsmInfo.h

.. rubric:: lbdex/Chapter3_2/MCTargetDesc/Cpu0MCAsmInfo.cpp
.. literalinclude:: ../../../lib/Target/Cpu0/MCTargetDesc/Cpu0MCAsmInfo.cpp


Finally, add code in Cpu0MCTargetDesc.cpp to register Cpu0InstPrinter as 
follows,

.. rubric:: lbdex/Chapter3_2/MCTargetDesc/Cpu0MCTargetDesc.h
.. literalinclude:: ../../../lib/Target/Cpu0/MCTargetDesc/Cpu0MCTargetDesc.h
    :start-after: #define CPU0MCTARGETDESC_H
    :end-before: namespace llvm {
.. literalinclude:: ../../../lib/Target/Cpu0/MCTargetDesc/Cpu0MCTargetDesc.h
    :start-after: namespace llvm {
    :end-before: class MCRegisterInfo;
.. literalinclude:: ../../../lib/Target/Cpu0/MCTargetDesc/Cpu0MCTargetDesc.h
    :start-after: class MCRegisterInfo;
    :end-before: class Target;
.. literalinclude:: ../../../lib/Target/Cpu0/MCTargetDesc/Cpu0MCTargetDesc.h
    :start-after: class Target;
    :end-before: extern Target TheCpu0Target;
.. literalinclude:: ../../../lib/Target/Cpu0/MCTargetDesc/Cpu0MCTargetDesc.h
    :start-after: // lbd document - mark - createCpu0MCCodeEmitterEL
    :end-before: // lbd document - mark - createCpu0AsmBackendEL32

.. rubric:: lbdex/Chapter3_2/MCTargetDesc/Cpu0MCTargetDesc.cpp
.. literalinclude:: ../../../lib/Target/Cpu0/MCTargetDesc/Cpu0MCTargetDesc.cpp
    :start-after: // #include
    :end-before: #include "Cpu0MCTargetDesc.h"
.. literalinclude:: ../../../lib/Target/Cpu0/MCTargetDesc/Cpu0MCTargetDesc.cpp
    :start-after: #include "Cpu0MCTargetDesc.h"
    :end-before: #include "llvm/MC/MachineLocation.h"
.. literalinclude:: ../../../lib/Target/Cpu0/MCTargetDesc/Cpu0MCTargetDesc.cpp
    :start-after: using namespace llvm;
    :end-before: } else if (CPU == "cpu032II") {
.. literalinclude:: ../../../lib/Target/Cpu0/MCTargetDesc/Cpu0MCTargetDesc.cpp
    :start-after: Cpu0ArchFeature = "+cpu032II";
    :end-before: static MCStreamer *createMCStreamer
.. literalinclude:: ../../../lib/Target/Cpu0/MCTargetDesc/Cpu0MCTargetDesc.cpp
    :start-after: // lbd document - mark - createMCStreamer
    :end-before: // Register the MC Code Emitter
.. literalinclude:: ../../../lib/Target/Cpu0/MCTargetDesc/Cpu0MCTargetDesc.cpp
    :start-after: createCpu0AsmBackendEL32

.. rubric:: lbdex/Chapter3_2/MCTargetDesc/CMakeLists.txt
.. code-block:: c++

    Cpu0MCAsmInfo.cpp

.. rubric:: lbdex/Chapter3_2/MCTargetDesc/LLVMBuild.txt
.. code-block:: c++

                       Cpu0AsmPrinter 

Now, it's time to work with AsmPrinter. According section 
"section Target Registration" [#]_, we can register our AsmPrinter when we need it 
as the following function of LLVMInitializeCpu0AsmPrinter(),

.. rubric:: lbdex/Chapter3_2/Cpu0AsmPrinter.h
.. literalinclude:: ../../../lib/Target/Cpu0/Cpu0AsmPrinter.h

.. rubric:: lbdex/Chapter3_2/Cpu0AsmPrinter.cpp
.. literalinclude:: ../../../lib/Target/Cpu0/Cpu0AsmPrinter.cpp
    :end-before: void Cpu0AsmPrinter::EmitInstrWithMacroNoAT
.. literalinclude:: ../../../lib/Target/Cpu0/Cpu0AsmPrinter.cpp
    :start-after: // lbd document - mark - EmitInstrWithMacroNoAT
    :end-before: unsigned Opc = MI->getOpcode();
.. literalinclude:: ../../../lib/Target/Cpu0/Cpu0AsmPrinter.cpp
    :start-after: unsigned Opc = MI->getOpcode();
    :end-before: SmallVector<MCInst, 4> MCInsts;
.. literalinclude:: ../../../lib/Target/Cpu0/Cpu0AsmPrinter.cpp
    :start-after: // lbd document - mark - switch (Opc)
    :end-before: bool EmitCPLoad
.. literalinclude:: ../../../lib/Target/Cpu0/Cpu0AsmPrinter.cpp
    :start-after: EmitCPLoad = false;
    :end-before: // Emit .cpload directive if needed.
.. literalinclude:: ../../../lib/Target/Cpu0/Cpu0AsmPrinter.cpp
    :start-after: OutStreamer.EmitRawText(StringRef("\t.cpload\t$t9"));
    :end-before: } else if (EmitCPLoad) {
.. literalinclude:: ../../../lib/Target/Cpu0/Cpu0AsmPrinter.cpp
    :start-after: // lbd document - mark - EmitInstruction(*I)


The dynamic register mechanism is a good idea, right.

Add the following code to Cpu0ISelLowering.cpp.

.. rubric:: lbdex/Chapter3_2/Cpu0ISelLowering.cpp
.. code-block:: c++

  Cpu0TargetLowering::
  Cpu0TargetLowering(Cpu0TargetMachine &TM)
    : TargetLowering(TM, new Cpu0TargetObjectFile()),
      Subtarget(&TM.getSubtarget<Cpu0Subtarget>()) {

    // Set up the register classes
    addRegisterClass(MVT::i32, &Cpu0::CPURegsRegClass);

  //- Set .align 2
  // It will emit .align 2 later
    setMinFunctionAlignment(2);

  // must, computeRegisterProperties - Once all of the register classes are 
  //  added, this allows us to compute derived properties we expose.
    computeRegisterProperties();
  }

Add the following code to Cpu0MachineFunction.h since the Cpu0AsmPrinter.cpp
will call getEmitNOAT().

.. rubric:: lbdex/Chapter3_2/Cpu0MachineFunction.h
.. code-block:: c++

  class Cpu0FunctionInfo : public MachineFunctionInfo {
    ...
    bool EmitNOAT;

  public:
    Cpu0FunctionInfo(MachineFunction& MF)
    : ...
      EmitNOAT(false), 
      ...
      {}

    ...
    bool getEmitNOAT() const { return EmitNOAT; }
    void setEmitNOAT() { EmitNOAT = true; }
  };


Beyond add these new .cpp files to CMakeLists.txt, please remember to add 
subdirectory InstPrinter, enable asmprinter, add libraries AsmPrinter and 
Cpu0AsmPrinter to LLVMBuild.txt as follows,

.. rubric:: lbdex/Chapter3_2/CMakeLists.txt
.. code-block:: c++

  tablegen(LLVM Cpu0GenCodeEmitter.inc -gen-emitter)
  tablegen(LLVM Cpu0GenMCCodeEmitter.inc -gen-emitter -mc-emitter)
  
  tablegen(LLVM Cpu0GenAsmWriter.inc -gen-asm-writer)
  ...
  add_llvm_target(Cpu0CodeGen
    Cpu0AsmPrinter.cpp
    ...
    Cpu0MCInstLower.cpp
    ...
    )
  ...
  add_subdirectory(InstPrinter)
  ...

.. rubric:: lbdex/Chapter3_2/LLVMBuild.txt
.. code-block:: c++

  //  LLVMBuild.txt
  [common] 
  subdirectories = 
    InstPrinter 
    ...
  
  [component_0] 
  ...
  # Please enable asmprinter
  has_asmprinter = 1 
  ...
  
  [component_1] 
  # Add AsmPrinter Cpu0AsmPrinter
  required_libraries = 
                       AsmPrinter 
                       ... 
                       Cpu0AsmPrinter 
                       ...

Now, run Chapter3_2/Cpu0 for AsmPrinter support, will get error message as follows,

.. code-block:: bash

  118-165-78-230:InputFiles Jonathan$ /Users/Jonathan/llvm/test/cmake_debug_build/
  bin/Debug/llc -march=cpu0 -relocation-model=pic -filetype=asm ch3.bc -o 
  ch3.cpu0.s
  /Users/Jonathan/llvm/test/cmake_debug_build/bin/Debug/llc: target does not 
  support generation of this file type!

The ``llc`` fails to compile IR code into machine code since we didn't implement 
class Cpu0DAGToDAGISel. Before the implementation, we will introduce the LLVM 
Code Generation Sequence, DAG, and LLVM instruction selection in next 3 
sections.

LLVM Code Generation Sequence
-----------------------------

Following diagram came from tricore_llvm.pdf.

.. _backendstructure-f5: 
.. figure:: ../Fig/backendstructure/5.png
  :align: center

  tricore_llvm.pdf: Code generation sequence. On the path from LLVM code to 
  assembly code, numerous passes are run through and several data structures 
  are used to represent the intermediate results.

LLVM is a Static Single Assignment (SSA) based representation. 
LLVM provides an infinite virtual registers which can hold values of primitive 
type (integral, floating point, or pointer values). 
So, every operand can save in different virtual register in llvm SSA 
representation. 
Comment is “;” in llvm representation. 
Following is the llvm SSA instructions.

.. code-block:: c++

  store i32 0, i32* %a  ; store i32 type of 0 to virtual register %a, %a is
              ;  pointer type which point to i32 value
  store i32 %b, i32* %c ; store %b contents to %c point to, %b isi32 type virtual
              ;  register, %c is pointer type which point to i32 value.
  %a1 = load i32* %a    ; load the memory value where %a point to and assign the
              ;  memory value to %a1
  %a3 = add i32 %a2, 1  ; add %a2 and 1 and save to %a3

We explain the code generation process as below. 
If you don't feel comfortable, please check tricore_llvm.pdf section 4.2 first. 
You can  read “The LLVM Target-Independent Code Generator” from [#]_ 
and “LLVM Language Reference Manual” from [#]_ 
before go ahead, but we think read section 
4.2 of tricore_llvm.pdf is enough. 
We suggest you read the web site documents as above only when you are still not 
quite understand, even though you have read the articles of this section and 
next 2 sections for DAG and Instruction Selection.

1. Instruction Selection

.. code-block:: c++

  // In this stage, transfer the llvm opcode into machine opcode, but the operand
  //  still is llvm virtual operand.
      store i16 0, i16* %a // store 0 of i16 type to where virtual register %a
                 //  point to
  =>  addiu i16 0, i32* %a

2. Scheduling and Formation

.. code-block:: c++

  // In this stage, reorder the instructions sequence for optimization in
  //  instructions cycle or in register pressure.
      st i32 %a, i16* %b,  i16 5 // st %a to *(%b+5)
      st %b, i32* %c, i16 0
      %d = ld i32* %c
  
  // Transfer above instructions order as follows. In RISC like Mips the ld %c use
  //  the previous instruction st %c, must wait more than 1
  // cycles. Meaning the ld cannot follow st immediately.
  =>  st %b, i32* %c, i16 0
      st i32 %a, i16* %b,  i16 5
      %d = ld i32* %c, i16 0
  // If without reorder instructions, a instruction nop which do nothing must be
  //  filled, contribute one instruction cycle more than optimization. (Actually,
  //  Mips is scheduled with hardware dynamically and will insert nop between st
  //  and ld instructions if compiler didn't insert nop.)
      st i32 %a, i16* %b,  i16 5
      st %b, i32* %c, i16 0
      nop
      %d = ld i32* %c, i16 0
  
  // Minimum register pressure
  //  Suppose %c is alive after the instructions basic block (meaning %c will be
  //  used after the basic block), %a and %b are not alive after that.
  // The following no reorder version need 3 registers at least
      %a = add i32 1, i32 0
      %b = add i32 2, i32 0
      st %a,  i32* %c, 1
      st %b,  i32* %c, 2
  
  // The reorder version need 2 registers only (by allocate %a and %b in the same
  //  register)
  => %a = add i32 1, i32 0
      st %a,  i32* %c, 1
      %b = add i32 2, i32 0
      st %b,  i32* %c, 2

3. SSA-based Machine Code Optimization

    For example, common expression remove, shown in next section DAG.
  
4. Register Allocation

    Allocate real register for virtual register.
  
5. Prologue/Epilogue Code Insertion

    Explain in section Add Prologue/Epilogue functions
  
6. Late Machine Code Optimizations

    Any “last-minute” peephole optimizations of the final machine code can be 
    applied during this phase. 
    For example, replace x = x * 2 by x = x < 1 for integer operand.
  
7. Code Emission
    Finally, the completed machine code is emitted. For static compilation, 
    the end result is an assembly code file; for JIT compilation, the opcodes 
    of the machine instructions are written into memory. 

The llvm code generation sequence also can be obtained by 
``llc -debug-pass=Structure`` as the following. The first 4 code generation 
sequences from :num:`Figure #backendstructure-f5` are in the 
**'DAG->DAG Pattern Instruction Selection'** of the ``llc -debug-pass=Structure`` 
displayed. The order of Peephole Optimizations and Prologue/Epilogue Insertion
is inconsistent in them (please check the * in the following). 
No need to bother since the the LLVM is under development and changed all the 
time. 

.. code-block:: bash

  118-165-79-200:InputFiles Jonathan$ llc --help-hidden
  OVERVIEW: llvm system compiler
  
  USAGE: llc [options] <input bitcode>
  
  OPTIONS:
  ...
    -debug-pass                             - Print PassManager debugging information
      =None                                 -   disable debug output
      =Arguments                            -   print pass arguments to pass to 'opt'
      =Structure                            -   print pass structure before run()
      =Executions                           -   print pass name before it is executed
      =Details                              -   print pass details when it is executed
  
  118-165-79-200:InputFiles Jonathan$ llc -march=mips -debug-pass=Structure ch3.bc
  ...
  Target Library Information
  Target Transform Info
  Data Layout
  Target Pass Configuration
  No Alias Analysis (always returns 'may' alias)
  Type-Based Alias Analysis
  Basic Alias Analysis (stateless AA impl)
  Create Garbage Collector Module Metadata
  Machine Module Information
  Machine Branch Probability Analysis
    ModulePass Manager
      FunctionPass Manager
        Preliminary module verification
        Dominator Tree Construction
        Module Verifier
        Natural Loop Information
        Loop Pass Manager
          Canonicalize natural loops
        Scalar Evolution Analysis
        Loop Pass Manager
          Canonicalize natural loops
          Induction Variable Users
          Loop Strength Reduction
        Lower Garbage Collection Instructions
        Remove unreachable blocks from the CFG
        Exception handling preparation
        Optimize for code generation
        Insert stack protectors
        Preliminary module verification
        Dominator Tree Construction
        Module Verifier
        Machine Function Analysis
        Natural Loop Information
        Branch Probability Analysis
      * MIPS DAG->DAG Pattern Instruction Selection
        Expand ISel Pseudo-instructions
        Tail Duplication
        Optimize machine instruction PHIs
        MachineDominator Tree Construction
        Slot index numbering
        Merge disjoint stack slots
        Local Stack Slot Allocation
        Remove dead machine instructions
        MachineDominator Tree Construction
        Machine Natural Loop Construction
        Machine Loop Invariant Code Motion
        Machine Common Subexpression Elimination
        Machine code sinking
      * Peephole Optimizations
        Process Implicit Definitions
        Remove unreachable machine basic blocks
        Live Variable Analysis
        Eliminate PHI nodes for register allocation
        Two-Address instruction pass
        Slot index numbering
        Live Interval Analysis
        Debug Variable Analysis
        Simple Register Coalescing
        Live Stack Slot Analysis
        Calculate spill weights
        Virtual Register Map
        Live Register Matrix
        Bundle Machine CFG Edges
        Spill Code Placement Analysis
      * Greedy Register Allocator
        Virtual Register Rewriter
        Stack Slot Coloring
        Machine Loop Invariant Code Motion
      * Prologue/Epilogue Insertion & Frame Finalization
        Control Flow Optimizer
        Tail Duplication
        Machine Copy Propagation Pass
      * Post-RA pseudo instruction expansion pass
        MachineDominator Tree Construction
        Machine Natural Loop Construction
        Post RA top-down list latency scheduler
        Analyze Machine Code For Garbage Collection
        Machine Block Frequency Analysis
        Branch Probability Basic Block Placement
        Mips Delay Slot Filler
        Mips Long Branch
        MachineDominator Tree Construction
        Machine Natural Loop Construction
      * Mips Assembly Printer
        Delete Garbage Collector Information


DAG (Directed Acyclic Graph)
----------------------------

Many important techniques for local optimization begin by transforming a basic 
block into DAG. For example, the basic block code and it's corresponding DAG as 
:num:`Figure #backendstructure-f6`.

.. _backendstructure-f6: 
.. figure:: ../Fig/backendstructure/6.png
  :align: center

  DAG example

If b is not live on exit from the block, then we can do common expression 
remove to get the following code.

.. code-block:: c++

  a = b + c
  d = a – d
  c = d + c

As you can imagine, the common expression remove can apply in IR or machine 
code.

DAG like a tree which opcode is the node and operand (register and 
const/immediate/offset) is leaf. 
It can also be represented by list as prefix order in tree. 
For example, (+ b, c), (+ b, 1) is IR DAG representation.


Instruction Selection
---------------------

In back end, we need to translate IR code into machine code at Instruction 
Selection Process as :num:`Figure #backendstructure-f7`.

.. _backendstructure-f7: 
.. figure:: ../Fig/backendstructure/7.png
  :align: center

  IR and it's corresponding machine instruction

For machine instruction selection, the better solution is represent IR and 
machine instruction by DAG. 
In :num:`Figure #backendstructure-f8`, we skip the register leaf. 
The rj + rk is IR DAG representation (for symbol notation, not llvm SSA form). 
ADD is machine instruction.

.. _backendstructure-f8: 
.. figure:: ../Fig/backendstructure/8.png
  :align: center

  Instruction DAG representation

The IR DAG and machine instruction DAG can also represented as list. 
For example, (+ ri, rj), (- ri, 1) are lists for IR DAG; (ADD ri, rj), 
(SUBI ri, 1) are lists for machine instruction DAG.

Now, let's recall the ADDiu instruction defined on Cpu0InstrInfo.td in the 
previous chapter. List them again as follows,

.. rubric:: lbdex/Chapter3_2/Cpu0InstrFormats.td
.. code-block:: c++

  //===----------------------------------------------------------------------===//
  // Format L instruction class in Cpu0 : <|opcode|ra|rb|cx|>
  //===----------------------------------------------------------------------===//
  
  class FL<bits<8> op, dag outs, dag ins, string asmstr, list<dag> pattern,
           InstrItinClass itin>: Cpu0Inst<outs, ins, asmstr, pattern, itin, FrmL>
  {
    bits<4>  ra;
    bits<4>  rb;
    bits<16> imm16;
  
    let Opcode = op;
  
    let Inst{23-20} = ra;
    let Inst{19-16} = rb;
    let Inst{15-0}  = imm16;
  }


.. rubric:: lbdex/Chapter3_2/Cpu0InstrInfo.td
.. code-block:: c++

  // Arithmetic and logical instructions with 2 register operands.
  class ArithLogicI<bits<8> op, string instr_asm, SDNode OpNode,
                    Operand Od, PatLeaf imm_type, RegisterClass RC> :
    FL<op, (outs RC:$ra), (ins RC:$rb, Od:$imm16),
       !strconcat(instr_asm, "\t$ra, $rb, $imm16"),
       [(set RC:$ra, (OpNode RC:$rb, imm_type:$imm16))], IIAlu> {
    let isReMaterializable = 1;
  }
  ...
  def ADDiu   : ArithLogicI<0x09, "addiu", add, simm16, immSExt16, CPURegs>;

:num:`Figure #backendstructure-f9` show how the pattern match work in the IR node 
**add** and instruction ADDiu defined in Cpu0InstrInfo.td. This example 
IR node "add %a, 5", will be translated to "addiu %r1, 5" since the IR 
pattern[(set RC:$ra, (OpNode RC:$rb, imm_type:$imm16))] is set in ADDiu and the
2nd operand is signed immediate which matched "%a, 5". In addition to pattern 
match, the .td also set assembly string "addiu" and op code 0x09. 
With this information, the LLVM TableGen will generate instruction both in 
assembly and binary automatically (the binary instruction in obj file of ELF 
format which will shown at later chapter). 
Similarly, the machine instruction DAG node LD and ST can be got from IR DAG 
node **load** and **store**.
 
.. _backendstructure-f9: 
.. figure:: ../Fig/backendstructure/9.png
  :align: center

  Pattern match for ADDiu instruction and IR node add


Some cpu/fpu (floating point processor) has multiply-and-add floating point 
instruction, fmadd. 
It can be represented by DAG list (fadd (fmul ra, rc), rb). 
For this implementation, we can assign fmadd DAG pattern to instruction td as 
follows,

.. code-block:: c++

  def FMADDS : AForm_1<59, 29,
            (ops F4RC:$FRT, F4RC:$FRA, F4RC:$FRC, F4RC:$FRB),
            "fmadds $FRT, $FRA, $FRC, $FRB",
            [(set F4RC:$FRT, (fadd (fmul F4RC:$FRA, F4RC:$FRC),
                         F4RC:$FRB))]>;

Similar with ADDiu, [(set F4RC:$FRT, (fadd (fmul F4RC:$FRA, F4RC:$FRC), 
F4RC:$FRB))] is the pattern which include node **fmul** and node **fadd**.

Now, for the following basic block notation IR and llvm SSA IR code,

.. code-block:: c++

  d = a * c
  e = d + b
  ...
  
  %d = fmul %a, %c
  %e = fadd %d, %b
  ...

The llvm SelectionDAG Optimization Phase (is part of Instruction Selection 
Process) prefered to translate this 2 IR DAG node (fmul %a, %b) (fadd %d, %c) 
into one machine instruction DAG node (**fmadd** %a, %c, %b), than translate 
them into 2 machine instruction nodes **fmul** and **fadd**.

.. code-block:: c++

  %e = fmadd %a, %c, %b
  ...

As you can see, the IR notation representation is easier to read then llvm SSA 
IR form. 
So, we  use the notation form in this book sometimes.

For the following basic block code,

.. code-block:: c++

  a = b + c   // in notation IR form
  d = a – d
  %e = fmadd %a, %c, %b // in llvm SSA IR form

We can apply :num:`Figure #backendstructure-f7` Instruction tree pattern to get the 
following machine code,

.. code-block:: c++

  load  rb, M(sp+8); // assume b allocate in sp+8, sp is stack point register
  load  rc, M(sp+16);
  add ra, rb, rc;
  load  rd, M(sp+24);
  sub rd, ra, rd;
  fmadd re, ra, rc, rb;


Add Cpu0DAGToDAGISel class
--------------------------

The IR DAG to machine instruction DAG transformation is introduced in the 
previous section. 
Now, let's check what IR DAG nodes the file ch3.bc has. List ch3.ll as follows,

.. code-block:: c++

  // ch3.ll
  define i32 @main() nounwind uwtable { 
  %1 = alloca i32, align 4 
  store i32 0, i32* %1 
  ret i32 0 
  } 

As above, ch3.ll use the IR DAG node **store**, **ret**. Actually, it also use 
**add** for sp (stack point) register adjust. 
So, the definitions in Cpu0InstrInfo.td as follows is enough. 
IR DAG is defined in file  include/llvm/Target/TargetSelectionDAG.td.

.. rubric:: lbdex/Chapter3_2/Cpu0InstrInfo.td
.. code-block:: c++

  //===----------------------------------------------------------------------===//

  /// Load and Store Instructions
  ///  aligned
  defm LD     : LoadM32<0x01,  "ld",  load_a>;
  defm ST     : StoreM32<0x02, "st",  store_a>;

  /// Arithmetic Instructions (ALU Immediate)
  // IR "add" defined in include/llvm/Target/TargetSelectionDAG.td, line 315 (def add).
  def ADDiu   : ArithLogicI<0x09, "addiu", add, simm16, immSExt16, CPURegs>;

  let isReturn=1, isTerminator=1, hasDelaySlot=1, isCodeGenOnly=1,
      isBarrier=1, hasCtrlDep=1 in
    def RET : FJ <0x3c, (outs), (ins CPURegs:$target),
                  "ret\t$target", [(Cpu0Ret CPURegs:$target)], IIBranch>;

  //===----------------------------------------------------------------------===//


Add class Cpu0DAGToDAGISel (Cpu0ISelDAGToDAG.cpp) to CMakeLists.txt, and add 
following fragment to Cpu0TargetMachine.cpp,

.. rubric:: lbdex/Chapter3_3/CMakeLists.txt
.. code-block:: c++

  add_llvm_target(...
    ...
    Cpu0ISelDAGToDAG.cpp
    ...
    )

The following code in Cpu0TargetMachine.cpp will create a pass in instruction 
selection stage.

.. rubric:: lbdex/Chapter3_3/Cpu0TargetMachine.cpp
.. literalinclude:: ../../../lib/Target/Cpu0/Cpu0TargetMachine.cpp
    :start-after: virtual bool addPreRegAlloc();
    :end-before: virtual bool addPreRegAlloc();
.. literalinclude:: ../../../lib/Target/Cpu0/Cpu0TargetMachine.cpp
    :start-after: virtual bool addPreEmitPass();
    :end-before: TargetPassConfig *Cpu0TargetMachine::createPassConfig
.. literalinclude:: ../../../lib/Target/Cpu0/Cpu0TargetMachine.cpp
    :start-after: // lbd document - mark - addInstSelector()
    :end-before: // Implemented by targets that want to run passes immediately

.. rubric:: lbdex/Chapter3_3/Cpu0ISelDAGToDAG.cpp
.. literalinclude:: ../../../lib/Target/Cpu0/Cpu0ISelDAGToDAG.cpp
    :end-before: #include "Cpu0MachineFunction.h"
.. literalinclude:: ../../../lib/Target/Cpu0/Cpu0ISelDAGToDAG.cpp
    :start-after: #include "Cpu0MachineFunction.h"
    :end-before: std::pair<SDNode*, SDNode*> SelectMULT
.. literalinclude:: ../../../lib/Target/Cpu0/Cpu0ISelDAGToDAG.cpp
    :start-after: std::pair<SDNode*, SDNode*> SelectMULT
    :end-before: void InitGlobalBaseReg(MachineFunction &MF);
.. literalinclude:: ../../../lib/Target/Cpu0/Cpu0ISelDAGToDAG.cpp
    :start-after: void InitGlobalBaseReg(MachineFunction &MF);
    :end-before: /// getGlobalBaseReg
.. literalinclude:: ../../../lib/Target/Cpu0/Cpu0ISelDAGToDAG.cpp
    :start-after: // lbd document - mark - getGlobalBaseReg()
    :end-before: // on PIC code Load GA
.. literalinclude:: ../../../lib/Target/Cpu0/Cpu0ISelDAGToDAG.cpp
    :start-after: // lbd document - mark - if (CurDAG->isBaseWithConstantOffset(Addr))
    :end-before: /// Select multiply instructions.
.. literalinclude:: ../../../lib/Target/Cpu0/Cpu0ISelDAGToDAG.cpp
    :start-after: // lbd document - mark - SelectMULT
    :end-before: DebugLoc dl = Node->getDebugLoc();
.. literalinclude:: ../../../lib/Target/Cpu0/Cpu0ISelDAGToDAG.cpp
    :start-after: DebugLoc dl = Node->getDebugLoc();
    :end-before: EVT NodeTy = Node->getValueType(0);
.. literalinclude:: ../../../lib/Target/Cpu0/Cpu0ISelDAGToDAG.cpp
    :start-after: unsigned MultOpc;
    :end-before: case ISD::SUBE:
.. literalinclude:: ../../../lib/Target/Cpu0/Cpu0ISelDAGToDAG.cpp
    :start-after: return getGlobalBaseReg();


This version adding the following code in Cpu0InstInfo.cpp to enable debug 
information which called by llvm at proper time.

.. rubric:: lbdex/Chapter3_3/Cpu0InstrInfo.h
.. code-block:: c++

  class Cpu0InstrInfo : public Cpu0GenInstrInfo {
    ...
    virtual MachineInstr* emitFrameIndexDebugValue(MachineFunction &MF,
                                                   int FrameIx, uint64_t Offset,
                                                   const MDNode *MDPtr,
                                                   DebugLoc DL) const;
  };

.. rubric:: lbdex/Chapter3_3/Cpu0InstrInfo.cpp
.. literalinclude:: ../../../lib/Target/Cpu0/Cpu0InstrInfo.cpp
    :start-after: #include "Cpu0MachineFunction.h"
    :end-before: #define GET_INSTRINFO_CTOR
.. literalinclude:: ../../../lib/Target/Cpu0/Cpu0InstrInfo.cpp
    :start-after: // lbd document - mark - loadRegFromStackSlot
    :end-before: // Cpu0InstrInfo::expandPostRAPseudo


Build Chapter3_3, run it, we find the error message in Chapter3_2 is gone. 
The new error message for Chapter3_3 as follows,

.. code-block:: bash

  118-165-78-230:InputFiles Jonathan$ /Users/Jonathan/llvm/test/cmake_debug_build/
  bin/Debug/llc -march=cpu0 -relocation-model=pic -filetype=asm ch3.bc -o 
  ch3.cpu0.s
  ...
  LLVM ERROR: Cannot select: 0x7f80f182d310: ch = <<Unknown Target Node #190>> 
  ...
    0x7f80f182d210: i32 = Register %LR [ID=4]

Handle return register lr 
-----------------------------

.. rubric:: lbdex/Chapter3_4/Cpu0InstrFormats.td
.. literalinclude:: ../../../lib/Target/Cpu0/Cpu0InstrFormats.td
    :start-after: // lbd document - mark - class Cpu0Inst
    :end-before: // Pseudo-instructions for alternate assembly syntax

.. rubric:: lbdex/Chapter3_4/Cpu0InstrInfo.td
.. literalinclude:: ../../../lib/Target/Cpu0/Cpu0InstrInfo.td
    :start-after: def JR
    :end-before: def RET

.. rubric:: lbdex/Chapter3_4/Cpu0InstrInfo.h
.. literalinclude:: ../../../lib/Target/Cpu0/Cpu0InstrInfo.h
    :start-after: // lbd document - mark - emitFrameIndexDebugValue
    :end-before: };

.. rubric:: lbdex/Chapter3_4/Cpu0InstrInfo.cpp
.. literalinclude:: ../../../lib/Target/Cpu0/Cpu0InstrInfo.cpp
    :start-after: // lbd document - mark - emitFrameIndexDebugValue

To handle IR ret, these code in Cpu0InstrInfo.td do things as below.

1. Declare a pseudo node to take care the IR Cpu0ISD::Ret by the following code,

.. rubric:: lbdex/Chapter9_3/Cpu0InstrInfo.td
.. literalinclude:: ../../../lib/Target/Cpu0/Cpu0InstrInfo.td
    :start-after: def Cpu0GPRel
    :end-before: // These are target-independent nodes
.. literalinclude:: ../../../lib/Target/Cpu0/Cpu0InstrInfo.td
    :start-after: def JR
    :end-before: def RET

2. After instruction selection, the Cpu0::Ret is replaced by Cpu0::RetLR 
   as below. This effect came from "def RetLR" as step 1.

.. code-block:: bash

  ===== Instruction selection begins: BB#0 'entry'
  Selecting: 0x1ea4050: ch = Cpu0ISD::Ret 0x1ea3f50, 0x1ea3e50, 
  0x1ea3f50:1 [ID=27]

  ISEL: Starting pattern match on root node: 0x1ea4050: ch = Cpu0ISD::Ret 
  0x1ea3f50, 0x1ea3e50, 0x1ea3f50:1 [ID=27]

    Morphed node: 0x1ea4050: ch = RetLR 0x1ea3e50, 0x1ea3f50, 0x1ea3f50:1
  ...
  ISEL: Match complete!
  => 0x1ea4050: ch = RetLR 0x1ea3e50, 0x1ea3f50, 0x1ea3f50:1
  ...
  ===== Instruction selection ends:
  Selected selection DAG: BB#0 'main:entry'
  SelectionDAG has 28 nodes:
  ...
      0x1ea3e50: <multiple use>
      0x1ea3f50: <multiple use>
      0x1ea3f50: <multiple use>
    0x1ea4050: ch = RetLR 0x1ea3e50, 0x1ea3f50, 0x1ea3f50:1


3. Expand the Cpu0::RetLR into instruction **ret $lr** in "Post-RA pseudo 
   instruction expansion pass" stage by the code in Chapter3_4/Cpu0InstrInfo.cpp 
   as above. This stage is after the register allocation, so we can replace the
   V0 ($r2) by LR ($lr) without any side effect.

4. Print assembly or obj according the information (those \*.inc generated by 
   TableGen from \*.td) generated by the following code at "Cpu0 Assembly 
   Printer" stage.

.. rubric:: lbdex/Chapter2/Cpu0InstrInfo.td
.. literalinclude:: ../../../lib/Target/Cpu0/Cpu0InstrInfo.td
    :start-after: // lbd document - mark - class UncondBranch
    :end-before: // Jump and Link (Call)
.. literalinclude:: ../../../lib/Target/Cpu0/Cpu0InstrInfo.td
    :start-after: def RetLR
    :end-before: def IRET

.. table:: Handle return register lr

  ===================================================  ========================================
  Stage                                                Function   
  ===================================================  ========================================
  Write Code                                           Declare a pseudo node Cpu0::RetLR
  -                                                    for IR Cpu0::Ret;
  Instruction selection                                Cpu0::Ret is replaced by Cpu0::RetLR
  Post-RA pseudo instruction expansion pass            Cpu0::RetLR -> ret $lr
  Cpu0 Assembly Printer                                Print according "def RET"
  ===================================================  ========================================

Build Chapter3_4, run it, we find the error message in Chapter3_3 is gone. 
The new error message for Chapter3_4 as follows,

  118-165-78-230:InputFiles Jonathan$ /Users/Jonathan/llvm/test/cmake_debug_build/
  bin/Debug/llc -march=cpu0 -relocation-model=pic -filetype=asm ch3.bc -o 
  ch3.cpu0.s
  ...
  Target didn't implement TargetInstrInfo::storeRegToStackSlot!
  1.  Running pass 'Function Pass Manager' on module 'ch3.bc'.
  2.  Running pass 'Prologue/Epilogue Insertion & Frame Finalization' on function 
  '@main'
  ...

Add Prologue/Epilogue functions
-------------------------------

Following came from tricore_llvm.pdf section “4.4.2 Non-static Register 
Information ”.

For some target architectures, some aspects of the target architecture’s 
register set are dependent upon variable factors and have to be determined at 
runtime. 
As a consequence, they cannot be generated statically from a TableGen 
description – although that would be possible for the bulk of them in the case 
of the TriCore backend. 
Among them are the following points:

- Callee-saved registers. Normally, the ABI specifies a set of registers that a 
  function must save on entry and restore on return if their contents are 
  possibly modified during execution.

- Reserved registers. Although the set of unavailable registers is already 
  defined in the TableGen file, TriCoreRegisterInfo contains a method that marks 
  all non-allocatable register numbers in a bit vector. 

The following methods are implemented:

- emitPrologue() inserts prologue code at the beginning of a function. Thanks 
  to TriCore’s context model, this is a trivial task as it is not required to 
  save any registers manually. The only thing that has to be done is reserving 
  space for the function’s stack frame by decrementing the stack pointer. 
  In addition, if the function needs a frame pointer, the frame register %a14 is 
  set to the old value of the stack pointer beforehand.

- emitEpilogue() is intended to emit instructions to destroy the stack frame 
  and restore all previously saved registers before returning from a function. 
  However, as %a10 (stack pointer), %a11 (return address), and %a14 (frame 
  pointer, if any) are all part of the upper context, no epilogue code is needed 
  at all. All cleanup operations are performed implicitly by the ret instruction. 

- eliminateFrameIndex() is called for each instruction that references a word 
  of data in a stack slot. All previous passes of the code generator have been 
  addressing stack slots through an abstract frame index and an immediate offset. 
  The purpose of this function is to translate such a reference into a 
  register–offset pair. Depending on whether the machine function that contains 
  the instruction has a fixed or a variable stack frame, either the stack pointer 
  %a10 or the frame pointer %a14 is used as the base register. 
  The offset is computed accordingly. 
  :num:`Figure #backendstructure-f10` demonstrates for both cases how a stack slot 
  is addressed. 

If the addressing mode of the affected instruction cannot handle the address 
because the offset is too large (the offset field has 10 bits for the BO 
addressing mode and 16 bits for the BOL mode), a sequence of instructions is 
emitted that explicitly computes the effective address. 
Interim results are put into an unused address register. 
If none is available, an already occupied address register is scavenged. 
For this purpose, LLVM’s framework offers a class named RegScavenger that 
takes care of all the details.

.. _backendstructure-f10: 
.. figure:: ../Fig/backendstructure/10.png
  :align: center

  Addressing of a variable a located on the stack. 
  If the stack frame has a variable size, slot must be addressed relative to 
  the frame pointer

We will explain the Prologue and Epilogue further by example code. 
So for the following llvm IR code, Cpu0 back end will emit the corresponding 
machine instructions as follows,

.. code-block:: bash

  define i32 @main() nounwind uwtable { 
    %1 = alloca i32, align 4 
    store i32 0, i32* %1 
    ret i32 0 
  }
  
    .section .mdebug.abi32
    .previous
    .file "ch3.bc"
    .text
    .globl  main//static void expandLargeImm\\n
    .align  2
    .type main,@function
    .ent  main                    # @main
  main:
    .cfi_startproc
    .frame  $sp,8,$lr
    .mask   0x00000000,0
    .set  noreorder
    .set  nomacro
  # BB#0:
    addiu $sp, $sp, -8
  $tmp1:
    .cfi_def_cfa_offset 8
    addiu $2, $zero, 0
    st  $2, 4($sp)
    addiu $sp, $sp, 8
    ret $lr
    .set  macro
    .set  reorder
    .end  main
  $tmp2:
    .size main, ($tmp2)-main
    .cfi_endproc

LLVM get the stack size by parsing IR and counting how many virtual registers 
is assigned to local variables. After that, it call emitPrologue(). 
This function will emit machine instructions to adjust sp (stack pointer 
register) for local variables since we don't use fp (frame pointer register). 
For our example, it will emit the instructions,

.. code-block:: c++

  addiu $sp, $sp, -8

The  emitEpilogue will emit “addiu  $sp, $sp, 8”, where 8 is the stack size.

Since Instruction Selection and Register Allocation occurs before 
Prologue/Epilogue Code Insertion, eliminateFrameIndex() is called after machine 
instruction and real register allocated. 
It translate the frame index of local variable (%1 and %2 in the following 
example) into stack offset according the frame index order upward (stack grow 
up downward from high address to low address, 0($sp) is the top, 52($sp) is the 
bottom) as follows,

.. code-block:: bash

  define i32 @main() nounwind uwtable { 
       %1 = alloca i32, align 4 
       %2 = alloca i32, align 4 
      ...
      store i32 0, i32* %1
      store i32 5, i32* %2, align 4 
      ...
      ret i32 0 
  
  => # BB#0: 
    addiu $sp, $sp, -56 
  $tmp1: 
    addiu $3, $zero, 0 
    st  $3, 52($sp)   // %1 is the first frame index local variable, so allocate
                      // in 52($sp)
    addiu $2, $zero, 5 
    st  $2, 48($sp)   // %2 is the second frame index local variable, so 
                      // allocate in 48($sp)
    ...
    ret $lr

The Prologue and Epilogue functions as follows,

.. rubric:: lbdex/Chapter3_5/Cpu0FrameLowering.h
.. literalinclude:: ../../../lib/Target/Cpu0/Cpu0FrameLowering.h
    :start-after: // lbd document - mark - explicit Cpu0FrameLowering
    :end-before: bool hasFP(const MachineFunction &MF) const;

.. rubric:: lbdex/Chapter3_5/Cpu0FrameLowering.cpp
.. literalinclude:: ../../../lib/Target/Cpu0/Cpu0FrameLowering.cpp
    :start-after: // lbd document - mark - expandLargeImm
    :end-before: // lbd document - mark - Cpu0::SP
.. literalinclude:: ../../../lib/Target/Cpu0/Cpu0FrameLowering.cpp
    :start-after: // lbd document - mark - Cpu0::ADDu
    :end-before: unsigned RegSize = 4;
.. literalinclude:: ../../../lib/Target/Cpu0/Cpu0FrameLowering.cpp
    :start-after: unsigned RegSize = 4;
    :end-before: Cpu0FI->needGPSaveRestore() ?
.. literalinclude:: ../../../lib/Target/Cpu0/Cpu0FrameLowering.cpp
    :start-after: (MFI->getObjectOffset(Cpu0FI->getGPFI()) + RegSize) :
    :end-before: // if framepointer enabled, set it to point to the stack pointer.
.. literalinclude:: ../../../lib/Target/Cpu0/Cpu0FrameLowering.cpp
    :start-after: // lbd document - mark - if (Cpu0FI->needGPSaveRestore())
    :end-before: // lbd document - mark - emitEpilogue() Cpu0::SP
.. literalinclude:: ../../../lib/Target/Cpu0/Cpu0FrameLowering.cpp
    :start-after: // lbd document - mark - emitEpilogue() Cpu0::ADDu
    :end-before: // if framepointer enabled, restore the stack pointer.
.. literalinclude:: ../../../lib/Target/Cpu0/Cpu0FrameLowering.cpp
    :start-after: // lbd document - mark - emitEpilogue() if (hasFP(MF))
    :end-before: bool Cpu0FrameLowering::spillCalleeSavedRegisters

.. rubric:: lbdex/Chapter3_5/Cpu0AnalyzeImmediate.h
.. literalinclude:: ../../../lib/Target/Cpu0/Cpu0AnalyzeImmediate.h

.. rubric:: lbdex/Chapter3_5/Cpu0AnalyzeImmediate.cpp
.. literalinclude:: ../../../lib/Target/Cpu0/Cpu0AnalyzeImmediate.cpp

.. rubric:: lbdex/Chapter3_5/Cpu0RegisterInfo.cpp
.. literalinclude:: ../../../lib/Target/Cpu0/Cpu0RegisterInfo.cpp
    :start-after: // lbd document - mark - getReservedRegs
    :end-before: unsigned Cpu0RegisterInfo::

.. rubric:: lbdex/Chapter3_5/CMakeLists.txt
.. code-block:: c++

  add_llvm_target(...
    ...
    Cpu0AnalyzeImmediate.cpp
    ...
    )


After add these Prologue and Epilogue functions, and build with Chapter3_5/Cpu0. 
Now we are ready to compile our example code ch3.bc into cpu0 assembly code. 
Following is the command and output file ch3.cpu0.s,

.. code-block:: bash

  118-165-78-12:InputFiles Jonathan$ /Users/Jonathan/llvm/test/cmake_debug_build/
  bin/Debug/llc -march=cpu0 -relocation-model=pic -filetype=asm -debug ch3.bc -o -
  Args: /Users/Jonathan/llvm/test/cmake_debug_build/bin/Debug/llc -march=cpu0 
  -relocation-model=pic -filetype=asm -debug ch3.bc -o ch3.cpu0.s 
  118-165-78-12:InputFiles Jonathan$ cat ch3.cpu0.s 
    .section .mdebug.abi32
    .previous
    .file "ch3.bc"
    .text
    .globl  main
    .align  2
    .type main,@function
    .ent  main                    # @main
  main:
    .cfi_startproc
    .frame  $sp,8,$lr
    .mask   0x00000000,0
    .set  noreorder
    .set  nomacro
  # BB#0:
    addiu $sp, $sp, -8
  $tmp1:
    .cfi_def_cfa_offset 8
    addiu $2, $zero, 0
    st  $2, 4($sp)
    addiu $sp, $sp, 8
    ret $lr
    .set  macro
    .set  reorder
    .end  main
  $tmp2:
    .size main, ($tmp2)-main
    .cfi_endproc

To see how the **'DAG->DAG Pattern Instruction Selection'** work in llc, let's 
compile with ``llc -debug`` option and see what happens.

.. code-block:: bash

  118-165-78-12:InputFiles Jonathan$ /Users/Jonathan/llvm/test/cmake_debug_build/
  bin/Debug/llc -march=cpu0 -relocation-model=pic -filetype=asm -debug ch3.bc -o -
  Args: /Users/Jonathan/llvm/test/cmake_debug_build/bin/Debug/llc -march=cpu0 
  -relocation-model=pic -filetype=asm -debug ch3.bc -o - 
  ...
  Optimized legalized selection DAG: BB#0 'main:'
  SelectionDAG has 8 nodes:
    0x7fbe4082d010: i32 = Constant<0> [ORD=1] [ID=1]
  
    0x7fbe4082d410: i32 = Register %V0 [ID=4]
  
        0x7fbe40410668: ch = EntryToken [ORD=1] [ID=0]
  
        0x7fbe4082d010: <multiple use>
        0x7fbe4082d110: i32 = FrameIndex<0> [ORD=1] [ID=2]
  
        0x7fbe4082d210: i32 = undef [ORD=1] [ID=3]
  
      0x7fbe4082d310: ch = store 0x7fbe40410668, 0x7fbe4082d010, 0x7fbe4082d110, 
      0x7fbe4082d210<ST4[%1]> [ORD=1] [ID=5]
  
      0x7fbe4082d410: <multiple use>
      0x7fbe4082d010: <multiple use>
    0x7fbe4082d510: ch,glue = CopyToReg 0x7fbe4082d310, 0x7fbe4082d410, 
    0x7fbe4082d010 [ID=6]
  
      0x7fbe4082d510: <multiple use>
      0x7fbe4082d410: <multiple use>
      0x7fbe4082d510: <multiple use>
    0x7fbe4082d610: ch = Cpu0ISD::Ret 0x7fbe4082d510, 0x7fbe4082d410, 
    0x7fbe4082d510:1 [ID=7]
  
  
  ===== Instruction selection begins: BB#0 ''
  Selecting: 0x7fbe4082d610: ch = Cpu0ISD::Ret 0x7fbe4082d510, 0x7fbe4082d410, 
  0x7fbe4082d510:1 [ID=7]
  
  ISEL: Starting pattern match on root node: 0x7fbe4082d610: ch = Cpu0ISD::Ret 
  0x7fbe4082d510, 0x7fbe4082d410, 0x7fbe4082d510:1 [ID=7]
  
    Morphed node: 0x7fbe4082d610: ch = RET 0x7fbe4082d410, 0x7fbe4082d510, 
    0x7fbe4082d510:1
    
  ISEL: Match complete!
  => 0x7fbe4082d610: ch = RET 0x7fbe4082d410, 0x7fbe4082d510, 0x7fbe4082d510:1
  
  Selecting: 0x7fbe4082d510: ch,glue = CopyToReg 0x7fbe4082d310, 0x7fbe4082d410, 
  0x7fbe4082d010 [ID=6]
  
  => 0x7fbe4082d510: ch,glue = CopyToReg 0x7fbe4082d310, 0x7fbe4082d410, 
  0x7fbe4082d010
  
  Selecting: 0x7fbe4082d310: ch = store 0x7fbe40410668, 0x7fbe4082d010, 
  0x7fbe4082d110, 0x7fbe4082d210<ST4[%1]> [ORD=1] [ID=5]
  
  ISEL: Starting pattern match on root node: 0x7fbe4082d310: ch = store 0x7fbe40410668, 
  0x7fbe4082d010, 0x7fbe4082d110, 0x7fbe4082d210<ST4[%1]> [ORD=1] [ID=5]
  
    Initial Opcode index to 166
    Morphed node: 0x7fbe4082d310: ch = ST 0x7fbe4082d010, 0x7fbe4082d710, 
    0x7fbe4082d810, 0x7fbe40410668<Mem:ST4[%1]> [ORD=1]
  
  ISEL: Match complete!
  => 0x7fbe4082d310: ch = ST 0x7fbe4082d010, 0x7fbe4082d710, 0x7fbe4082d810, 
  0x7fbe40410668<Mem:ST4[%1]> [ORD=1]
  
  Selecting: 0x7fbe4082d410: i32 = Register %V0 [ID=4]
  
  => 0x7fbe4082d410: i32 = Register %V0
  
  Selecting: 0x7fbe4082d010: i32 = Constant<0> [ORD=1] [ID=1]
  
  ISEL: Starting pattern match on root node: 0x7fbe4082d010: i32 = 
  Constant<0> [ORD=1] [ID=1]
  
    Initial Opcode index to 1201
    Morphed node: 0x7fbe4082d010: i32 = ADDiu 0x7fbe4082d110, 0x7fbe4082d810 [ORD=1]
  
  ISEL: Match complete!
  => 0x7fbe4082d010: i32 = ADDiu 0x7fbe4082d110, 0x7fbe4082d810 [ORD=1]
  
  Selecting: 0x7fbe40410668: ch = EntryToken [ORD=1] [ID=0]
  
  => 0x7fbe40410668: ch = EntryToken [ORD=1]
  
  ===== Instruction selection ends:


Summary above translation into Table: Chapter 3 .bc IR instructions.

.. table:: Chapter 3 .bc IR instructions

  =============================  ==================================  ==========
  .bc                            Optimized legalized selection DAG   Cpu0
  =============================  ==================================  ==========
  constant 0                     constant 0                          addiu         
  store                          store                               st
  ret                            Cpu0ISD::Ret                        ret
  =============================  ==================================  ==========

From above ``llc -debug`` display, we see the **store** and **ret** are 
translated into **store** and **Cpu0ISD::Ret** in stage Optimized legalized 
selection DAG, and then translated into Cpu0 instructions **st** and **ret** 
finally. 
Since store use **constant 0** (**store i32 0, i32\* %1** in this example), the 
constant 0 will be translated into **"addiu $2, $zero, 0"** via the following 
pattern defined in Cpu0InstrInfo.td.


.. rubric:: lbdex/Chapter3_5/Cpu0InstrInfo.td
.. code-block:: c++

  // Small immediates
  def : Pat<(i32 immSExt16:$in),
            (ADDiu ZERO, imm:$in)>;
  def : Pat<(i32 immZExt16:$in),
            (ORi ZERO, imm:$in)>;
  def : Pat<(i32 immLow16Zero:$in),
            (LUi (HI16 imm:$in))>;
  
  // Arbitrary immediates
  def : Pat<(i32 imm:$imm),
            (ORi (LUi (HI16 imm:$imm)), (LO16 imm:$imm))>;


At this point, we have translated the very simple main() function with return 0
single instruction. The Cpu0AnalyzeImmediate.cpp defined as above and the 
Cpu0InstrInfo.td instructions add as below, which takes care 
the 32 bits stack size adjustments.

.. rubric:: lbdex/Chapter3_5/Cpu0InstrInfo.td
.. code-block:: c++

  def shamt       : Operand<i32>;
  
  // Unsigned Operand
  def uimm16      : Operand<i32> {
    let PrintMethod = "printUnsignedImm";
  }
  ...
  // Transformation Function - get the lower 16 bits.
  def LO16 : SDNodeXForm<imm, [{
    return getImm(N, N->getZExtValue() & 0xffff);
  }]>;

  // Transformation Function - get the higher 16 bits.
  def HI16 : SDNodeXForm<imm, [{
    return getImm(N, (N->getZExtValue() >> 16) & 0xffff);
  }]>;
  ...
  // Node immediate fits as 16-bit zero extended on target immediate.
  // The LO16 param means that only the lower 16 bits of the node
  // immediate are caught.
  // e.g. addiu, sltiu
  def immZExt16  : PatLeaf<(imm), [{
    if (N->getValueType(0) == MVT::i32)
      return (uint32_t)N->getZExtValue() == (unsigned short)N->getZExtValue();
    else
      return (uint64_t)N->getZExtValue() == (unsigned short)N->getZExtValue();
  }], LO16>;

  // Immediate can be loaded with LUi (32-bit int with lower 16-bit cleared).
  def immLow16Zero : PatLeaf<(imm), [{
    int64_t Val = N->getSExtValue();
    return isInt<32>(Val) && !(Val & 0xffff);
  }]>;
  
  // shamt field must fit in 5 bits.
  def immZExt5 : ImmLeaf<i32, [{return Imm == (Imm & 0x1f);}]>;
  ...
  // Arithmetic and logical instructions with 3 register operands.
  class ArithLogicR<bits<8> op, string instr_asm, SDNode OpNode,
                    InstrItinClass itin, RegisterClass RC, bit isComm = 0>:
    FA<op, (outs RC:$ra), (ins RC:$rb, RC:$rc),
       !strconcat(instr_asm, "\t$ra, $rb, $rc"),
       [(set RC:$ra, (OpNode RC:$rb, RC:$rc))], itin> {
    let shamt = 0;
    let isCommutable = isComm;	// e.g. add rb rc =  add rc rb
    let isReMaterializable = 1;
  }
  ...
  
  // Shifts
  class shift_rotate_imm<bits<8> op, bits<4> isRotate, string instr_asm,
                         SDNode OpNode, PatFrag PF, Operand ImmOpnd,
                         RegisterClass RC>:
    FA<op, (outs RC:$ra), (ins RC:$rb, ImmOpnd:$shamt),
       !strconcat(instr_asm, "\t$ra, $rb, $shamt"),
       [(set RC:$ra, (OpNode RC:$rb, PF:$shamt))], IIAlu> {
    let rc = isRotate;
    let shamt = shamt;
  }
  
  // 32-bit shift instructions.
  class shift_rotate_imm32<bits<8> func, bits<4> isRotate, string instr_asm,
                           SDNode OpNode>:
    shift_rotate_imm<func, isRotate, instr_asm, OpNode, immZExt5, shamt, CPURegs>;

  // Load Upper Imediate
  class LoadUpper<bits<8> op, string instr_asm, RegisterClass RC, Operand Imm>:
    FL<op, (outs RC:$ra), (ins Imm:$imm16),
       !strconcat(instr_asm, "\t$ra, $imm16"), [], IIAlu> {
    let rb = 0;
    let neverHasSideEffects = 1;
    let isReMaterializable = 1;
  }
  ...
  def ORi     : ArithLogicI<0x0d, "ori", or, uimm16, immZExt16, CPURegs>;
  def LUi     : LoadUpper<0x0f, "lui", CPURegs, uimm16>;
  
  /// Arithmetic Instructions (3-Operand, R-Type)
  def ADDu    : ArithLogicR<0x11, "addu", add, IIAlu, CPURegs, 1>;
  
  /// Shift Instructions
  def SHL     : shift_rotate_imm32<0x1e, 0x00, "shl", shl>;
  ...

  // Small immediates
  ...
  def : Pat<(i32 immZExt16:$in),
            (ORi ZERO, imm:$in)>;
  def : Pat<(i32 immLow16Zero:$in),
            (LUi (HI16 imm:$in))>;

  // Arbitrary immediates
  def : Pat<(i32 imm:$imm),
            (ORi (LUi (HI16 imm:$imm)), (LO16 imm:$imm))>;

The Cpu0AnalyzeImmediate.cpp written in recursive and a little complicate in 
logic. Anyway, the recursive
skills is used in the front end compile book, you should fimiliar with it.
Instead tracking the code, listing the stack size and the instructions 
generated in Table: Cpu0 stack adjustment instructions before replace addiu and 
shl with lui instruction as follows (Cpu0 stack adjustment instructions after 
replace addiu and shl with lui instruction as below),

.. table:: Cpu0 stack adjustment instructions before replace addiu and shl with lui instruction

  ====================  ================  ==================================  ==================================
  stack size range      ex. stack size    Cpu0 Prologue instructions          Cpu0 Epilogue instructions
  ====================  ================  ==================================  ==================================
  0 ~ 0x7fff            - 0x7fff          - addiu $sp, $sp, 32767;            - addiu $sp, $sp, 32767;
  0x8000 ~ 0xffff       - 0x8000          - addiu $sp, $sp, -32768;           - addiu $1, $zero, 1;
                                                                              - shl $1, $1, 16;
                                                                              - addiu $1, $1, -32768;
                                                                              - addu $sp, $sp, $1;
  x10000 ~ 0xffffffff   - 0x7fffffff      - addiu $1, $zero, -1;              - addiu $1, $zero, 1;
                                          - shl $1, $1, 31;                   - shl $1, $1, 31;
                                          - addiu $1, $1, 1;                  - addiu $1, $1, -1;
                                          - addu $sp, $sp, $1;                - addu $sp, $sp, $1;
  x10000 ~ 0xffffffff   - 0x90008000      - addiu $1, $zero, -9;              - addiu $1, $zero, -28671;
                                          - shl $1, $1, 28;                   - shl $1, $1, 16
                                          - addiu $1, $1, -32768;             - addiu $1, $1, -32768;
                                          - addu $sp, $sp, $1;                - addu $sp, $sp, $1;
  ====================  ================  ==================================  ==================================

Assume sp = 0xa0008000 and stack size = 0x90008000, then (0xa0008000 - 
0x90008000) => 0x10000000. Verify with the Cpu0 Prologue instructions as 
follows,

1. "addiu	$1, $zero, -9" => ($1 = 0 + 0xfffffff7) => $1 = 0xfffffff7.
2. "shl	$1, $1, 28;" => $1 = 0x70000000.
3. "addiu	$1, $1, -32768" => $1 = (0x70000000 + 0xffff8000) => $1 = 0x6fff8000.
4. "addu	$sp, $sp, $1" => $sp = (0xa0008000 + 0x6fff8000) => $sp = 0x10000000.

Verify with the Cpu0 Epilogue instructions with sp = 0x10000000 and stack size = 
0x90008000 as follows,

1. "addiu	$1, $zero, -28671" => ($1 = 0 + 0xffff9001) => $1 = 0xffff9001.
2. "shl	$1, $1, 16;" => $1 = 0x90010000.
3. "addiu	$1, $1, -32768" => $1 = (0x90010000 + 0xffff8000) => $1 = 0x90008000.
4. "addu	$sp, $sp, $1" => $sp = (0x10000000 + 0x90008000) => $sp = 0xa0008000.


The Cpu0AnalyzeImmediate::GetShortestSeq() will call Cpu0AnalyzeImmediate::
ReplaceADDiuSHLWithLUi() to replace addiu and shl with single instruction lui 
only. The effect as the following table.

.. table:: Cpu0 stack adjustment instructions after replace addiu and shl with lui instruction

  ======  ====================  ================  ==================================  ==================================
          stack size range      ex. stack size    Cpu0 Prologue instructions          Cpu0 Epilogue instructions
  ======  ====================  ================  ==================================  ==================================
  old     x10000 ~ 0xffffffff   - 0x90008000      - addiu $1, $zero, -9;              - addiu $1, $zero, -28671;
                                                  - shl $1, $1, 28;                   - shl $1, $1, 16
                                                  - addiu $1, $1, -32768;             - addiu $1, $1, -32768;
                                                  - addu $sp, $sp, $1;                - addu $sp, $sp, $1;
  ------  --------------------  ----------------  ----------------------------------  ----------------------------------
  new     x10000 ~ 0xffffffff   - 0x90008000      - lui	$1, 28671;                    - lui	$1, 36865;
                                                  - ori	$1, $1, 32768;                - addiu $1, $1, -32768;
                                                  - addu $sp, $sp, $1;                - addu $sp, $sp, $1;
  ======  ====================  ================  ==================================  ==================================


Assume sp = 0xa0008000 and stack size = 0x90008000, then (0xa0008000 - 
0x90008000) => 0x10000000. Verify with the Cpu0 Prologue instructions as 
follows,

1. "lui	$1, 28671" => $1 = 0x6fff0000.
2. "ori	$1, $1, 32768" => $1 = (0x6fff0000 + 0x00008000) => $1 = 0x6fff8000.
3. "addu	$sp, $sp, $1" => $sp = (0xa0008000 + 0x6fff8000) => $sp = 0x10000000.

Verify with the Cpu0 Epilogue instructions with sp = 0x10000000 and stack size = 
0x90008000 as follows,

1. "lui	$1, 36865" => $1 = 0x90010000.
2. "addiu $1, $1, -32768" => $1 = (0x90010000 + 0xffff8000) => $1 = 0x90008000.
3. "addu $sp, $sp, $1" => $sp = (0x10000000 + 0x90008000) => $sp = 0xa0008000.



Summary of this Chapter
-----------------------

Summary the functions for llvm backend stages as the following table.

.. code-block:: bash

  118-165-79-200:InputFiles Jonathan$ /Users/Jonathan/llvm/test/cmake_debug_build/
  bin/Debug/llc -march=cpu0 -relocation-model=pic -filetype=asm ch3.bc 
  -debug-pass=Structure -o -
  ...
  Machine Branch Probability Analysis
    ModulePass Manager
      FunctionPass Manager
        ...
        CPU0 DAG->DAG Pattern Instruction Selection
          Initial selection DAG
          Optimized lowered selection DAG
          Type-legalized selection DAG
          Optimized type-legalized selection DAG
          Legalized selection DAG
          Optimized legalized selection DAG
          Instruction selection
          Selected selection DAG
          Scheduling
        ...
        Greedy Register Allocator
        ...
        Post-RA pseudo instruction expansion pass
        ...
        Cpu0 Assembly Printer


.. table:: functions for llvm backend stage

  ===================================================  ===========================================
  Stage                                                Function   
  ===================================================  ===========================================
  Before CPU0 DAG->DAG Pattern Instruction Selection   - Cpu0TargetLowering::LowerFormalArguments
                                                       - Cpu0TargetLowering::LowerReturn
  Instruction selection                                - Cpu0DAGToDAGISel::Select
  Prologue/Epilogue Insertion & Frame Finalization     - Cpu0FrameLowering.cpp
                                                       - Cpu0RegisterInfo::eliminateFrameIndex()
  Cpu0 Assembly Printer                                - Cpu0AsmPrinter.cpp -> Cpu0MCInstLower.cpp
                                                       - Cpu0InstPrinter.cpp
  ===================================================  ===========================================

We add a pass in Instruction Section stage in section "Add Cpu0DAGToDAGISel 
class". You can embed your code into other pass like that. Please check 
CodeGen/Passes.h for the information. Remember the pass is called according 
the function unit as the ``llc -debug-pass=Structure`` indicated.

We have finished a simple assembler for cpu0 which only support **ld**, 
**st**, **addiu**, **ori**, **lui**, **addu**, **shl** and **ret** 8 
instructions.

We are satisfied with this result. 
But you may think “After so many codes we program, and just get these 8 
instructions”. 
The point is we have created a frame work for cpu0 target machine (please 
look back the llvm back end structure class inherit tree early in this 
chapter). 
Until now, we have over 3000 lines of source code with comments which include 
files \*.cpp, \*.h, \*.td, CMakeLists.txt and LLVMBuild.txt. 
It can be counted by command ``wc `find dir -name *.cpp``` for files \*.cpp, 
\*.h, \*.td, \*.txt. 
LLVM front end tutorial have 700 lines of source code without comments totally. 
Don't feel down with this result. 
In reality, write a back end is warm up slowly but run fast. 
Clang has over 500,000 lines of source code with comments in clang/lib 
directory which include C++ and Obj C support. 
Mips back end has only 15,000 lines with comments. 
Even the complicate X86 CPU which CISC outside and RISC inside (micro 
instruction), has only 45,000 lines with comments. 
In next chapter, we will show you that add a new instruction support is as easy 
as 123.



.. [#] http://llvm.org/docs/WritingAnLLVMBackend.html#target-machine

.. [#] http://jonathan2251.github.com/lbd/llvmstructure.html#target-registration

.. [#] http://llvm.org/docs/CodeGenerator.html

.. [#] http://llvm.org/docs/LangRef.html
