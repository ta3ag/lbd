.. _sec-globalvars:

Global variables
==================

In the previous two chapters, we only access the local variables. 
This chapter will deal global variable access translation. 

The global variable DAG translation is different from the previous DAG 
translation we have now. 
It create DAG nodes at run time in our backend C++ code according the 
``llc -relocation-model`` option while the others of DAG just do IR DAG to 
Machine DAG translation directly according the input file IR DAG.


Global variable
----------------

Chapter6_1/ support the global variable, let's compile ch6_1.cpp with this version 
first, and explain the code changes after that.

.. rubric:: lbdex/InputFiles/ch6_1.cpp
.. literalinclude:: ../lbdex/InputFiles/ch6_1.cpp
    :start-after: /// start

.. code-block:: bash

  118-165-78-166:InputFiles Jonathan$ llvm-dis ch6_1.bc -o -
  ; ModuleID = 'ch6_1.bc'
  target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-
  f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-
  n8:16:32:64-S128"
  target triple = "x86_64-apple-macosx10.8.0"
  
  @gStart = global i32 2, align 4
  @gI = global i32 100, align 4
  
  define i32 @_Z3funv() nounwind uwtable ssp {
    %1 = alloca i32, align 4
    %c = alloca i32, align 4
    store i32 0, i32* %1
    store i32 0, i32* %c, align 4
    %2 = load i32* @gI, align 4
    store i32 %2, i32* %c, align 4
    %3 = load i32* %c, align 4
    ret i32 %3
  }

Cpu0 global variable options
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Cpu0 like Mips supports both static and pic mode. There are two different layout 
of global variables for static mode which controlled by option cpu0-use-small-section. 
Chapter6_1/ support the global variable translation. 
Let's run Chapter6_1/ with ch6_1.cpp via three different options 
``llc  -relocation-model=static -cpu0-use-small-section=false``, 
``llc  -relocation-model=static -cpu0-use-small-section=true`` and 
``llc  -relocation-model=pic`` to trace the DAG and Cpu0 instructions.

.. code-block:: bash

  118-165-78-166:InputFiles Jonathan$ clang -target mips-unknown-linux-gnu -c 
  ch6_1.cpp -emit-llvm -o ch6_1.bc
  118-165-78-166:InputFiles Jonathan$ /Users/Jonathan/llvm/test/cmake_debug_build/
  bin/Debug/llc -march=cpu0 -relocation-model=static -cpu0-use-small-section=false 
  -filetype=asm -debug ch6_1.bc -o -
  
  ...
  Type-legalized selection DAG: BB#0 '_Z3funv:entry'
  SelectionDAG has 12 nodes:
    ...
        0x7ffd5902cc10: <multiple use>
      0x7ffd5902cf10: ch = store 0x7ffd5902cd10, 0x7ffd5902ca10, 0x7ffd5902ce10, 
      0x7ffd5902cc10<ST4[%c]> [ORD=2] [ID=-3]
  
      0x7ffd5902d010: i32 = GlobalAddress<i32* @gI> 0 [ORD=3] [ID=-3]
  
      0x7ffd5902cc10: <multiple use>
    0x7ffd5902d110: i32,ch = load 0x7ffd5902cf10, 0x7ffd5902d010, 
    0x7ffd5902cc10<LD4[@gI]> [ORD=3] [ID=-3]
    ...
  
  Legalized selection DAG: BB#0 '_Z3funv:entry'
  SelectionDAG has 16 nodes:
    ...
        0x7ffd5902cc10: <multiple use>
      0x7ffd5902cf10: ch = store 0x7ffd5902cd10, 0x7ffd5902ca10, 0x7ffd5902ce10, 
      0x7ffd5902cc10<ST4[%c]> [ORD=2] [ID=8]
  
          0x7ffd5902d310: i32 = TargetGlobalAddress<i32* @gI> 0 [TF=5]
  
        0x7ffd5902d710: i32 = Cpu0ISD::Hi 0x7ffd5902d310
  
          0x7ffd5902d610: i32 = TargetGlobalAddress<i32* @gI> 0 [TF=6]
  
        0x7ffd5902d810: i32 = Cpu0ISD::Lo 0x7ffd5902d610
  
      0x7ffd5902fe10: i32 = add 0x7ffd5902d710, 0x7ffd5902d810
  
      0x7ffd5902cc10: <multiple use>
    0x7ffd5902d110: i32,ch = load 0x7ffd5902cf10, 0x7ffd5902fe10, 
    0x7ffd5902cc10<LD4[@gI]> [ORD=3] [ID=9]
    ...
  
  	addiu	$2, $zero, %hi(gI)
  	shl	$2, $2, 16
  	addiu	$2, $2, %lo(gI)
  	ld	$2, 0($2)
  	...
  	.type	gStart,@object          # @gStart
  	.data
  	.globl	gStart
  	.align	2
  gStart:
  	.4byte	2                       # 0x2
  	.size	gStart, 4
  
  	.type	gI,@object              # @gI
  	.globl	gI
  	.align	2
  gI:
  	.4byte	100                     # 0x64
  	.size	gI, 4

.. code-block:: bash

  118-165-78-166:InputFiles Jonathan$ /Users/Jonathan/llvm/test/cmake_debug_build/
  bin/Debug/llc -march=cpu0 -relocation-model=static -cpu0-use-small-section=true 
  -filetype=asm -debug ch6_1.bc -o -
  
  ...
  Type-legalized selection DAG: BB#0 '_Z3funv:entry'
  SelectionDAG has 12 nodes:
    ...
        0x7fc5f382cc10: <multiple use>
      0x7fc5f382cf10: ch = store 0x7fc5f382cd10, 0x7fc5f382ca10, 0x7fc5f382ce10, 
      0x7fc5f382cc10<ST4[%c]> [ORD=2] [ID=-3]
  
      0x7fc5f382d010: i32 = GlobalAddress<i32* @gI> 0 [ORD=3] [ID=-3]
  
      0x7fc5f382cc10: <multiple use>
    0x7fc5f382d110: i32,ch = load 0x7fc5f382cf10, 0x7fc5f382d010, 
    0x7fc5f382cc10<LD4[@gI]> [ORD=3] [ID=-3]
  
  Legalized selection DAG: BB#0 '_Z3funv:entry'
  SelectionDAG has 15 nodes:
    ...
        0x7fc5f382cc10: <multiple use>
      0x7fc5f382cf10: ch = store 0x7fc5f382cd10, 0x7fc5f382ca10, 0x7fc5f382ce10, 
      0x7fc5f382cc10<ST4[%c]> [ORD=2] [ID=8]
  
        0x7fc5f382d710: i32 = GLOBAL_OFFSET_TABLE
  
          0x7fc5f382d310: i32 = TargetGlobalAddress<i32* @gI> 0 [TF=4]
  
        0x7fc5f382d610: i32 = Cpu0ISD::GPRel 0x7fc5f382d310
  
      0x7fc5f382d810: i32 = add 0x7fc5f382d710, 0x7fc5f382d610
  
      0x7fc5f382cc10: <multiple use>
    0x7fc5f382d110: i32,ch = load 0x7fc5f382cf10, 0x7fc5f382d810, 
    0x7fc5f382cc10<LD4[@gI]> [ORD=3] [ID=9]
    ...
  
  	addiu	$2, $gp, %gp_rel(gI)
  	ld	$2, 0($2)
  	...
  	.type	gStart,@object          # @gStart
  	.section	.sdata,"aw",@progbits
  	.globl	gStart
  	.align	2
  gStart:
  	.4byte	2                       # 0x2
  	.size	gStart, 4
  
  	.type	gI,@object              # @gI
  	.globl	gI
  	.align	2
  gI:
  	.4byte	100                     # 0x64
  	.size	gI, 4

.. code-block:: bash

  118-165-78-166:InputFiles Jonathan$ /Users/Jonathan/llvm/test/cmake_debug_build/
  bin/Debug/llc -march=cpu0 -relocation-model=pic -filetype=asm -debug ch6_1.bc 
  -o -
  
  ...
  Type-legalized selection DAG: BB#0 '_Z3funv:entry'
  SelectionDAG has 12 nodes:
    ...
        0x7fad7102cc10: <multiple use>
      0x7fad7102cf10: ch = store 0x7fad7102cd10, 0x7fad7102ca10, 0x7fad7102ce10, 
      0x7fad7102cc10<ST4[%c]> [ORD=2] [ID=-3]
  
      0x7fad7102d010: i32 = GlobalAddress<i32* @gI> 0 [ORD=3] [ID=-3]
  
      0x7fad7102cc10: <multiple use>
    0x7fad7102d110: i32,ch = load 0x7fad7102cf10, 0x7fad7102d010, 
    0x7fad7102cc10<LD4[@gI]> [ORD=3] [ID=-3]
    ...
  Legalized selection DAG: BB#0 '_Z3funv:entry'
  SelectionDAG has 15 nodes:
    0x7ff3c9c10b98: ch = EntryToken [ORD=1] [ID=0]
    ...
        0x7fad7102cc10: <multiple use>
      0x7fad7102cf10: ch = store 0x7fad7102cd10, 0x7fad7102ca10, 0x7fad7102ce10, 
      0x7fad7102cc10<ST4[%c]> [ORD=2] [ID=8]
  
        0x7fad70c10b98: <multiple use>
          0x7fad7102d610: i32 = Register %GP
  
          0x7fad7102d310: i32 = TargetGlobalAddress<i32* @gI> 0 [TF=1]
  
        0x7fad7102d710: i32 = Cpu0ISD::Wrapper 0x7fad7102d610, 0x7fad7102d310
  
        0x7fad7102cc10: <multiple use>
      0x7fad7102d810: i32,ch = load 0x7fad70c10b98, 0x7fad7102d710, 
      0x7fad7102cc10<LD4[<unknown>]>
      
      0x7ff3ca02cc10: <multiple use>
    0x7ff3ca02d110: i32,ch = load 0x7ff3ca02cf10, 0x7ff3ca02d810, 
    0x7ff3ca02cc10<LD4[@gI]> [ORD=3] [ID=9]
    ...
	  .set	noreorder
	  .cpload	$6
	  .set	nomacro
    ...
  	ld	$2, %got(gI)($gp)
  	ld	$2, 0($2)
    ...
  	.type	gStart,@object          # @gStart
  	.data
  	.globl	gStart
  	.align	2
  gStart:
  	.4byte	2                       # 0x2
  	.size	gStart, 4
  
  	.type	gI,@object              # @gI
  	.globl	gI
  	.align	2
  gI:
  	.4byte	100                     # 0x64
  	.size	gI, 4


Summary above information to Table: Cpu0 global variable options.

.. table:: Cpu0 global variable options

  ============================  ====================  ===================  =================================================
  option name                   default               other option value   discription
  ============================  ====================  ===================  =================================================
  -relocation-model             pic                   static               - pic: Postion Independent Address
                                                                           - static: Absolute Address
  -cpu0-use-small-section       false                 true                 - false: .data or .bss, 16 bits addressable
                                                                           - true: .sdata or .sbss, 32 bits addressable
  ============================  ====================  ===================  =================================================
  

.. csv-table:: Cpu0 DAGs and instructions for -relocation-model=static
   :header: "option: cpu0-use-small-section", "false", "true"
   :widths: 20, 20, 20

   "addressing mode", "absolute", "$gp relative"
   "addressing", "absolute", "$gp+offset"
   "Legalized selection DAG", "(add Cpu0ISD::Hi<gI offset Hi16> Cpu0ISD::Lo<gI offset Lo16>)", "(add GLOBAL_OFFSET_TABLE, Cpu0ISD::GPRel<gI offset>)"
   "Cpu0", "addiu $2, $zero, %hi(gI); shl $2, $2, 16; addiu $2, $2, %lo(gI);", "addiu	$2, $gp, %gp_rel(gI);"
   "relocation records solved", "link time", "link time"

- In static, cpu0-use-small-section=true, offset between gI and .data can be calculated since the $gp is assigned at fixed address of the start of global address table.
- In "static, cpu0-use-small-section=false", the gI high and low address (%hi(gI) and %lo(gI)) are translated into absolute address. 

.. csv-table:: Cpu0 DAGs and instructions for -relocation-model=pic
   :header: "option: cpu0-use-small-section", "false", "true"
   :widths: 20, 20, 20

   "addressing mode","$gp relative", "$gp relative"
   "addressing", "$gp+offset", "$gp+offset"
   "Legalized selection DAG", "(load (Cpu0ISD::Wrapper %GP, <gI offset>))", "(load EntryToken, (Cpu0ISD::Wrapper (add Cpu0ISD::Hi<gI offset Hi16>, Register %GP), Cpu0ISD::Lo<gI offset Lo16>))"
   "Cpu0", "ld $2, %got(gI)($gp);", "addiu	$2, $zero, %got_hi(gI); shl $2, $2, 16; add $2, $2, $gp; ld $2, %got_lo(gI)($2);"
   "relocation records solved", "link/load time", "link/load time"

- In pic, offset between gI and .data cannot be calculated if the function is loaded at run time (dynamic link); the offset can be calculated if use static link.
- In C, all variable names binding staticly. In C++, the overload variable or function are binding dynamicly.

According book of system program, there are Absolute Addressing Mode and 
Position Independent Addressing Mode. The dynamic function must compiled with 
Position Independent Addressing Mode. In principle, option -relocation-model is 
used to generate Absolute Addressing or Position Independent Addressing.
The exception is -relocation-model=static and -cpu0-use-small-section=false.
In this case, the register $gp is reserved to set at the start address of global 
variable area. Cpu0 use $gp relative addressing in this mode.

To support global variable, first add **UseSmallSectionOpt** command variable to 
Cpu0Subtarget.cpp. 
After that, user can run llc with option ``llc -cpu0-use-small-section=false`` 
to specify **UseSmallSectionOpt** to false. 
The default of **UseSmallSectionOpt** is false if without specify it further. 
About the **cl::opt** command line variable, you can refer to [#]_ further.

.. rubric:: lbdex/Chapter6_1/Cpu0Subtarget.h
.. code-block:: c++

  class Cpu0Subtarget : public Cpu0GenSubtargetInfo {
    ...
    // UseSmallSection - Small section is used.
    bool UseSmallSection;
    ...
    bool useSmallSection() const { return UseSmallSection; }
  };

.. rubric:: lbdex/Chapter6_1/Cpu0Subtarget.cpp
.. code-block:: c++

  static cl::opt<bool>
  UseSmallSectionOpt("cpu0-use-small-section", cl::Hidden, cl::init(false),
                   cl::desc("Use small section. Only work with -relocation-model="
                   "static. pic always not use small section."));
  
  static cl::opt<bool>
  ReserveGPOpt("cpu0-reserve-gp", cl::Hidden, cl::init(false),
                   cl::desc("Never allocate $gp to variable"));
  
  static cl::opt<bool>
  NoCploadOpt("cpu0-no-cpload", cl::Hidden, cl::init(false),
                   cl::desc("No issue .cpload"));
  
  bool Cpu0ReserveGP;
  bool Cpu0NoCpload;
  
  extern bool FixGlobalBaseReg;

The ReserveGPOpt and NoCploadOpt are used in Cpu0 linker at later Chapter.
Next add file Cpu0TargetObjectFile.h, Cpu0TargetObjectFile.cpp and the 
following code to Cpu0RegisterInfo.cpp and Cpu0ISelLowering.cpp.

.. rubric:: lbdex/Chapter6_1/Cpu0TargetObjectFile.h
.. literalinclude:: ../../../lib/Target/Cpu0/Cpu0TargetObjectFile.h

.. rubric:: lbdex/Chapter6_1/Cpu0TargetObjectFile.cpp
.. literalinclude:: ../../../lib/Target/Cpu0/Cpu0TargetObjectFile.cpp

.. rubric:: lbdex/Chapter6_1/Cpu0RegisterInfo.cpp
.. code-block:: c++

  // pure virtual method
  BitVector Cpu0RegisterInfo::
  getReservedRegs(const MachineFunction &MF) const {
    ...
    // Reserve GP if small section is used.
    if (Subtarget.useSmallSection()) {
      Reserved.set(Cpu0::GP);
    }
    ...
  }

.. rubric:: lbdex/Chapter6_1/Cpu0ISelLowering.cpp
.. code-block:: c++

  #include "Cpu0MachineFunction.h"
  ...
  #include "Cpu0TargetObjectFile.h"
  ...
  #include "MCTargetDesc/Cpu0BaseInfo.h"
  ...
  #include "llvm/Support/CommandLine.h"
  SDValue Cpu0TargetLowering::getGlobalReg(SelectionDAG &DAG, EVT Ty) const {
    Cpu0FunctionInfo *FI = DAG.getMachineFunction().getInfo<Cpu0FunctionInfo>();
    return DAG.getRegister(FI->getGlobalBaseReg(), Ty);
  }

  static SDValue getTargetNode(SDValue Op, SelectionDAG &DAG, unsigned Flag) {
    EVT Ty = Op.getValueType();

    if (GlobalAddressSDNode *N = dyn_cast<GlobalAddressSDNode>(Op))
      return DAG.getTargetGlobalAddress(N->getGlobal(), Op.getDebugLoc(), Ty, 0,
                                        Flag);
    if (ExternalSymbolSDNode *N = dyn_cast<ExternalSymbolSDNode>(Op))
      return DAG.getTargetExternalSymbol(N->getSymbol(), Ty, Flag);
    if (BlockAddressSDNode *N = dyn_cast<BlockAddressSDNode>(Op))
      return DAG.getTargetBlockAddress(N->getBlockAddress(), Ty, 0, Flag);
    if (JumpTableSDNode *N = dyn_cast<JumpTableSDNode>(Op))
      return DAG.getTargetJumpTable(N->getIndex(), Ty, Flag);
    if (ConstantPoolSDNode *N = dyn_cast<ConstantPoolSDNode>(Op))
      return DAG.getTargetConstantPool(N->getConstVal(), Ty, N->getAlignment(),
                                       N->getOffset(), Flag);

    llvm_unreachable("Unexpected node type.");
    return SDValue();
  }

  SDValue Cpu0TargetLowering::getAddrLocal(SDValue Op, SelectionDAG &DAG) const {
    DebugLoc DL = Op.getDebugLoc();
    EVT Ty = Op.getValueType();
    unsigned GOTFlag = Cpu0II::MO_GOT;
    SDValue GOT = DAG.getNode(Cpu0ISD::Wrapper, DL, Ty, getGlobalReg(DAG, Ty),
                              getTargetNode(Op, DAG, GOTFlag));
    SDValue Load = DAG.getLoad(Ty, DL, DAG.getEntryNode(), GOT,
                               MachinePointerInfo::getGOT(), false, false, false,
                               0);
    unsigned LoFlag = Cpu0II::MO_ABS_LO;
    SDValue Lo = DAG.getNode(Cpu0ISD::Lo, DL, Ty, getTargetNode(Op, DAG, LoFlag));
    return DAG.getNode(ISD::ADD, DL, Ty, Load, Lo);
  }

  SDValue Cpu0TargetLowering::getAddrGlobal(SDValue Op, SelectionDAG &DAG,
                                            unsigned Flag) const {
    DebugLoc DL = Op.getDebugLoc();
    EVT Ty = Op.getValueType();
    SDValue Tgt = DAG.getNode(Cpu0ISD::Wrapper, DL, Ty, getGlobalReg(DAG, Ty),
                              getTargetNode(Op, DAG, Flag));
    return DAG.getLoad(Ty, DL, DAG.getEntryNode(), Tgt,
                       MachinePointerInfo::getGOT(), false, false, false, 0);
  }

  SDValue Cpu0TargetLowering::getAddrGlobalLargeGOT(SDValue Op, SelectionDAG &DAG,
                                                    unsigned HiFlag,
                                                    unsigned LoFlag) const {
    DebugLoc DL = Op.getDebugLoc();
    EVT Ty = Op.getValueType();
    SDValue Hi = DAG.getNode(Cpu0ISD::Hi, DL, Ty, getTargetNode(Op, DAG, HiFlag));
    Hi = DAG.getNode(ISD::ADD, DL, Ty, Hi, getGlobalReg(DAG, Ty));
    SDValue Wrapper = DAG.getNode(Cpu0ISD::Wrapper, DL, Ty, Hi,
                                  getTargetNode(Op, DAG, LoFlag));
    return DAG.getLoad(Ty, DL, DAG.getEntryNode(), Wrapper,
                       MachinePointerInfo::getGOT(), false, false, false, 0);
  }

  const char *Cpu0TargetLowering::getTargetNodeName(unsigned Opcode) const {
    switch (Opcode) {
    case Cpu0ISD::JmpLink:           return "Cpu0ISD::JmpLink";
    case Cpu0ISD::Hi:                return "Cpu0ISD::Hi";
    case Cpu0ISD::Lo:                return "Cpu0ISD::Lo";
    case Cpu0ISD::GPRel:             return "Cpu0ISD::GPRel";
    case Cpu0ISD::Ret:               return "Cpu0ISD::Ret";
    case Cpu0ISD::DivRem:            return "Cpu0ISD::DivRem";
    case Cpu0ISD::DivRemU:           return "Cpu0ISD::DivRemU";
    case Cpu0ISD::Wrapper:           return "Cpu0ISD::Wrapper";
    default:                         return NULL;
    }
  }

  Cpu0TargetLowering::
  Cpu0TargetLowering(Cpu0TargetMachine &TM)
    : TargetLowering(TM, new Cpu0TargetObjectFile()),
      Subtarget(&TM.getSubtarget<Cpu0Subtarget>()) {
     ...
    // Cpu0 Custom Operations
    setOperationAction(ISD::GlobalAddress,      MVT::i32,   Custom);
    ...
  }
  ...
  SDValue Cpu0TargetLowering::
  LowerOperation(SDValue Op, SelectionDAG &DAG) const
  {
    switch (Op.getOpcode())
    {
      case ISD::GlobalAddress:      return LowerGlobalAddress(Op, DAG);
    }
    return SDValue();
  }
    
  //===----------------------------------------------------------------------===//
  //  Lower helper functions
  //===----------------------------------------------------------------------===//
    
  //===----------------------------------------------------------------------===//
  //  Misc Lower Operation implementation
  //===----------------------------------------------------------------------===//
    
  SDValue Cpu0TargetLowering::LowerGlobalAddress(SDValue Op,
                                                 SelectionDAG &DAG) const {
    // FIXME there isn't actually debug info here
    DebugLoc dl = Op.getDebugLoc();
    const GlobalValue *GV = cast<GlobalAddressSDNode>(Op)->getGlobal();
    
    if (getTargetMachine().getRelocationModel() != Reloc::PIC_) {
      SDVTList VTs = DAG.getVTList(MVT::i32);
    
      Cpu0TargetObjectFile &TLOF = (Cpu0TargetObjectFile&)getObjFileLowering();
    
      // %gp_rel relocation
      if (TLOF.IsGlobalInSmallSection(GV, getTargetMachine())) {
        SDValue GA = DAG.getTargetGlobalAddress(GV, dl, MVT::i32, 0,
                                                Cpu0II::MO_GPREL);
        SDValue GPRelNode = DAG.getNode(Cpu0ISD::GPRel, dl, VTs, &GA, 1);
        SDValue GOT = DAG.getGLOBAL_OFFSET_TABLE(MVT::i32);
        return DAG.getNode(ISD::ADD, dl, MVT::i32, GOT, GPRelNode);
      }
      // %hi/%lo relocation
      SDValue GAHi = DAG.getTargetGlobalAddress(GV, dl, MVT::i32, 0,
                                                Cpu0II::MO_ABS_HI);
      SDValue GALo = DAG.getTargetGlobalAddress(GV, dl, MVT::i32, 0,
                                                Cpu0II::MO_ABS_LO);
      SDValue HiPart = DAG.getNode(Cpu0ISD::Hi, dl, VTs, &GAHi, 1);
      SDValue Lo = DAG.getNode(Cpu0ISD::Lo, dl, MVT::i32, GALo);
      return DAG.getNode(ISD::ADD, dl, MVT::i32, HiPart, Lo);
    }
    
    if (GV->hasInternalLinkage() || (GV->hasLocalLinkage() && !isa<Function>(GV)))
      return getAddrLocal(Op, DAG);

    if (TLOF.IsGlobalInSmallSection(GV, getTargetMachine()))
      return getAddrGlobal(Op, DAG, Cpu0II::MO_GOT16);
    else
      return getAddrGlobalLargeGOT(Op, DAG, Cpu0II::MO_GOT_HI16,
                                   Cpu0II::MO_GOT_LO16);
  }

The setOperationAction(ISD::GlobalAddress, MVT::i32, Custom) tells ``llc`` that 
we implement global address operation in C++ function 
Cpu0TargetLowering::LowerOperation(). LLVM will call this function only when 
llvm want to translate IR DAG of loading global variable into machine code. 
Since there are many Custom type of setOperationAction(ISD::XXX, MVT::XXX, 
Custom) in construction function Cpu0TargetLowering(), and each of them will 
trigger llvm calling Cpu0TargetLowering::LowerOperation() in stage 
"Legalized selection DAG" . 
The global address access can be identified by check if the DAG node of 
opcode is equal to ISD::GlobalAddress. 

Finally, add the following code in Cpu0InstrInfo.td.

.. rubric:: lbdex/Chapter6_1/Cpu0InstrInfo.td
.. code-block:: c++

  // Hi and Lo nodes are used to handle global addresses. Used on
  // Cpu0ISelLowering to lower stuff like GlobalAddress, ExternalSymbol
  // static model. (nothing to do with Cpu0 Registers Hi and Lo)
  def Cpu0Hi    : SDNode<"Cpu0ISD::Hi", SDTIntUnaryOp>;
  def Cpu0Lo    : SDNode<"Cpu0ISD::Lo", SDTIntUnaryOp>;
  def Cpu0GPRel : SDNode<"Cpu0ISD::GPRel", SDTIntUnaryOp>;
  ...
  // hi/lo relocs
  def : Pat<(Cpu0Hi tglobaladdr:$in), (SHL (ADDiu ZERO, tglobaladdr:$in), 16)>;
  // Expect cpu0 add LUi support, like Mips
  //def : Pat<(Cpu0Hi tglobaladdr:$in), (LUi tglobaladdr:$in)>;
  def : Pat<(Cpu0Lo tglobaladdr:$in), (ADDiu ZERO, tglobaladdr:$in)>;
  
  def : Pat<(add CPURegs:$hi, (Cpu0Lo tglobaladdr:$lo)),
        (ADDiu CPURegs:$hi, tglobaladdr:$lo)>;
  
  // gp_rel relocs
  def : Pat<(add CPURegs:$gp, (Cpu0GPRel tglobaladdr:$in)),
        (ADDiu CPURegs:$gp, tglobaladdr:$in)>;

Static mode
~~~~~~~~~~~~

From Table: Cpu0 global variable options, option cpu0-use-small-section=false 
put the global varibale in data/bss while cpu0-use-small-section=true in 
sdata/sbss. The sdata stands for small data area.
Section data and sdata are areas for global variable with initial value (such 
as int gI = 100 in this example) while Section bss and sbss are areas for 
global variables without initial value (for example, int gI;).

data or bss
++++++++++++

The data/bss are 32 bits addressable areas since Cpu0 is a 32 bits architecture. 
Option cpu0-use-small-section=false will generate the following instructions.

.. code-block:: bash

    ...
  	addiu	$2, $zero, %hi(gI)
  	shl	$2, $2, 16
  	addiu	$2, $2, %lo(gI)
  	ld	$2, 0($2)
  	...
  	.type	gStart,@object          # @gStart
  	.data
  	.globl	gStart
  	.align	2
  gStart:
  	.4byte	2                       # 0x2
  	.size	gStart, 4
  
  	.type	gI,@object              # @gI
  	.globl	gI
  	.align	2
  gI:
  	.4byte	100                     # 0x64
  	.size	gI, 4
  	
Above code, it loads the high address part of gI PC relative address (16 bits) 
to register $2 and shift 16 bits. 
Now, the register $2 got it's high part of gI absolute address. 
Next, it add register $2 and low part of gI absolute address into $2. 
At this point, it get the gI memory address. Finally, it get the gI content by 
instruction "ld $2, 0($2)". 
The ``llc -relocation-model=static`` is for absolute address mode which must be 
used in static link mode. The dynamic link must be encoded with Position 
Independent Addressing. 
As you can see, the PC relative address can be solved in static link. 
In static, the function fun() is included to the whole execution file, ELF. 
The offset between .data and instruction "addiu $2, $zero, %hi(gI)" can be 
caculated. Since use PC relative address coding, this program can be loaded 
to any address and run well there.
If this program use absolute address and will be loaded at a specific address 
known at link stage, the relocation record of gI variable access instruction 
such as "addiu $2, $zero, %hi(gI)" and "addiu	$2, $2, %lo(gI)" can be solved 
at link time.
If this program use absolute address and the loading address is known at load 
time, then this relocation record will be solved by loader at loading time. 

IsGlobalInSmallSection() return true or false depends on UseSmallSectionOpt. 

The code fragment of LowerGlobalAddress() as the following corresponding option 
``llc -relocation-model=static -cpu0-use-small-section=true`` will translate DAG 
(GlobalAddress<i32* @gI> 0) into 
(add Cpu0ISD::Hi<gI offset Hi16> Cpu0ISD::Lo<gI offset Lo16>) in 
stage "Legalized selection DAG" as below.

.. rubric:: lbdex/Chapter6_1/Cpu0ISelLowering.cpp
.. code-block:: c++

    //  Cpu0ISelLowering.cpp
    ...
        // %hi/%lo relocation
        SDValue GAHi = DAG.getTargetGlobalAddress(GV, dl, MVT::i32, 0,
                                                  Cpu0II::MO_ABS_HI);
        SDValue GALo = DAG.getTargetGlobalAddress(GV, dl, MVT::i32, 0,
                                                  Cpu0II::MO_ABS_LO);
        SDValue HiPart = DAG.getNode(Cpu0ISD::Hi, dl, VTs, &GAHi, 1);
        SDValue Lo = DAG.getNode(Cpu0ISD::Lo, dl, MVT::i32, GALo);
        return DAG.getNode(ISD::ADD, dl, MVT::i32, HiPart, Lo);


.. code-block:: bash

  118-165-78-166:InputFiles Jonathan$ clang -target mips-unknown-linux-gnu -c 
  ch6_1.cpp -emit-llvm -o ch6_1.bc
  118-165-78-166:InputFiles Jonathan$ /Users/Jonathan/llvm/test/cmake_debug_build/
  bin/Debug/llc -march=cpu0 -relocation-model=static -cpu0-use-small-section=false 
  -filetype=asm -debug ch6_1.bc -o -
  
  ...
  Type-legalized selection DAG: BB#0 '_Z3funv:entry'
  SelectionDAG has 12 nodes:
    ...
        0x7ffd5902cc10: <multiple use>
      0x7ffd5902cf10: ch = store 0x7ffd5902cd10, 0x7ffd5902ca10, 0x7ffd5902ce10, 
      0x7ffd5902cc10<ST4[%c]> [ORD=2] [ID=-3]
  
      0x7ffd5902d010: i32 = GlobalAddress<i32* @gI> 0 [ORD=3] [ID=-3]
  
      0x7ffd5902cc10: <multiple use>
    0x7ffd5902d110: i32,ch = load 0x7ffd5902cf10, 0x7ffd5902d010, 
    0x7ffd5902cc10<LD4[@gI]> [ORD=3] [ID=-3]
    ...
  
  Legalized selection DAG: BB#0 '_Z3funv:entry'
  SelectionDAG has 16 nodes:
    ...
        0x7ffd5902cc10: <multiple use>
      0x7ffd5902cf10: ch = store 0x7ffd5902cd10, 0x7ffd5902ca10, 0x7ffd5902ce10, 
      0x7ffd5902cc10<ST4[%c]> [ORD=2] [ID=8]
  
          0x7ffd5902d310: i32 = TargetGlobalAddress<i32* @gI> 0 [TF=5]
  
        0x7ffd5902d710: i32 = Cpu0ISD::Hi 0x7ffd5902d310
  
          0x7ffd5902d610: i32 = TargetGlobalAddress<i32* @gI> 0 [TF=6]
  
        0x7ffd5902d810: i32 = Cpu0ISD::Lo 0x7ffd5902d610
  
      0x7ffd5902fe10: i32 = add 0x7ffd5902d710, 0x7ffd5902d810
  
      0x7ffd5902cc10: <multiple use>
    0x7ffd5902d110: i32,ch = load 0x7ffd5902cf10, 0x7ffd5902fe10, 
    0x7ffd5902cc10<LD4[@gI]> [ORD=3] [ID=9]


Finally, the pattern defined in Cpu0InstrInfo.td as the following will translate  
DAG (add Cpu0ISD::Hi<gI offset Hi16> Cpu0ISD::Lo<gI offset Lo16>) into Cpu0 
instructions as below.

.. rubric:: lbdex/Chapter6_1/Cpu0InstrInfo.td
.. code-block:: c++

  // Hi and Lo nodes are used to handle global addresses. Used on
  // Cpu0ISelLowering to lower stuff like GlobalAddress, ExternalSymbol
  // static model. (nothing to do with Cpu0 Registers Hi and Lo)
  def Cpu0Hi    : SDNode<"Cpu0ISD::Hi", SDTIntUnaryOp>;
  def Cpu0Lo    : SDNode<"Cpu0ISD::Lo", SDTIntUnaryOp>;
  ...
  // hi/lo relocs
  def : Pat<(Cpu0Hi tglobaladdr:$in), (LUi tglobaladdr:$in)>;
  def : Pat<(Cpu0Lo tglobaladdr:$in), (ADDiu ZERO, tglobaladdr:$in)>;
  
  def : Pat<(add CPURegs:$hi, (Cpu0Lo tglobaladdr:$lo)),
        (ADDiu CPURegs:$hi, tglobaladdr:$lo)>;

.. code-block:: bash

    ...
  	addiu	$2, $zero, %hi(gI)
  	shl	$2, $2, 16
  	addiu	$2, $2, %lo(gI)
  	...


As above, Pat<(...),(...)> include two lists of DAGs. 
The left is IR DAG and the right is machine instruction DAG. 
Pat<(Cpu0Hi tglobaladdr:$in), (SHL (ADDiu ZERO, tglobaladdr:$in), 16)>; will 
translate DAG (Cpu0ISD::Hi tglobaladdr) into (shl (addiu ZERO, tglobaladdr), 16).
Pat<(Cpu0Lo tglobaladdr:$in), (ADDiu ZERO, tglobaladdr:$in)>; will translate 
(Cpu0ISD::Hi tglobaladdr) into (addiu ZERO, tglobaladdr).
Pat<(add CPURegs:$hi, (Cpu0Lo tglobaladdr:$lo)), (ADDiu CPURegs:$hi, tglobaladdr:$lo)>;
will translate DAG (add Cpu0ISD::Hi, Cpu0ISD::Lo) into Cpu0 instruction 
(add Cpu0ISD::Hi, Cpu0ISD::Lo).


sdata or sbss
++++++++++++++

The sdata/sbss are 16 bits addressable areas which planed in ELF for fast access. 
Option cpu0-use-small-section=true will generate the following instructions.

.. code-block:: bash

    	addiu	$2, $gp, %gp_rel(gI)
    	ld	$2, 0($2)
    	...
  	.type	gStart,@object          # @gStart
  	.section	.sdata,"aw",@progbits
  	.globl	gStart
  	.align	2
  gStart:
  	.4byte	2                       # 0x2
  	.size	gStart, 4
  
  	.type	gI,@object              # @gI
  	.globl	gI
  	.align	2
  gI:
  	.4byte	100                     # 0x64
  	.size	gI, 4


The code fragment of LowerGlobalAddress() as the following corresponding option 
``llc -relocation-model=static -cpu0-use-small-section=true`` will translate DAG 
(GlobalAddress<i32* @gI> 0) into 
(add GLOBAL_OFFSET_TABLE Cpu0ISD::GPRel<gI offset>) in 
stage "Legalized selection DAG" as below.

.. rubric:: lbdex/Chapter6_1/Cpu0ISelLowering.cpp
.. code-block:: c++

    //  Cpu0ISelLowering.cpp
      ...
      // %gp_rel relocation
      if (TLOF.IsGlobalInSmallSection(GV, getTargetMachine())) {
        SDValue GA = DAG.getTargetGlobalAddress(GV, dl, MVT::i32, 0,
                                                Cpu0II::MO_GPREL);
        SDValue GPRelNode = DAG.getNode(Cpu0ISD::GPRel, dl, VTs, &GA, 1);
        SDValue GOT = DAG.getGLOBAL_OFFSET_TABLE(MVT::i32);
        return DAG.getNode(ISD::ADD, dl, MVT::i32, GOT, GPRelNode);
      }

.. code-block:: bash

  ...
  Type-legalized selection DAG: BB#0 '_Z3funv:entry'
  SelectionDAG has 12 nodes:
    ...
        0x7fc5f382cc10: <multiple use>
      0x7fc5f382cf10: ch = store 0x7fc5f382cd10, 0x7fc5f382ca10, 0x7fc5f382ce10, 
      0x7fc5f382cc10<ST4[%c]> [ORD=2] [ID=-3]
  
      0x7fc5f382d010: i32 = GlobalAddress<i32* @gI> 0 [ORD=3] [ID=-3]
  
      0x7fc5f382cc10: <multiple use>
    0x7fc5f382d110: i32,ch = load 0x7fc5f382cf10, 0x7fc5f382d010, 
    0x7fc5f382cc10<LD4[@gI]> [ORD=3] [ID=-3]
  
  Legalized selection DAG: BB#0 '_Z3funv:entry'
  SelectionDAG has 15 nodes:
    ...
        0x7fc5f382cc10: <multiple use>
      0x7fc5f382cf10: ch = store 0x7fc5f382cd10, 0x7fc5f382ca10, 0x7fc5f382ce10, 
      0x7fc5f382cc10<ST4[%c]> [ORD=2] [ID=8]
  
        0x7fc5f382d710: i32 = GLOBAL_OFFSET_TABLE
  
          0x7fc5f382d310: i32 = TargetGlobalAddress<i32* @gI> 0 [TF=4]
  
        0x7fc5f382d610: i32 = Cpu0ISD::GPRel 0x7fc5f382d310
  
      0x7fc5f382d810: i32 = add 0x7fc5f382d710, 0x7fc5f382d610
  
      0x7fc5f382cc10: <multiple use>
    0x7fc5f382d110: i32,ch = load 0x7fc5f382cf10, 0x7fc5f382d810, 
    0x7fc5f382cc10<LD4[@gI]> [ORD=3] [ID=9]
    ...


Finally, the pattern defined in Cpu0InstrInfo.td as the following will translate  
DAG (add GLOBAL_OFFSET_TABLE Cpu0ISD::GPRel<gI offset>) into Cpu0 
instruction as below. The following code in Cpu0ISelDAGToDAG.cpp make the 
GLOBAL_OFFSET_TABLE translate into $gp as below.

.. rubric:: lbdex/Chapter6_1/Cpu0ISelDAGToDAG.cpp
.. code-block:: c++

  /// getGlobalBaseReg - Output the instructions required to put the
  /// GOT address into a register.
  SDNode *Cpu0DAGToDAGISel::getGlobalBaseReg() {
    unsigned GlobalBaseReg = MF->getInfo<Cpu0FunctionInfo>()->getGlobalBaseReg();
    return CurDAG->getRegister(GlobalBaseReg, TLI.getPointerTy()).getNode();
  }

  /// Select instructions not customized! Used for
  /// expanded, promoted and normal instructions
  SDNode* Cpu0DAGToDAGISel::Select(SDNode *Node) {
    ...
    // Get target GOT address.
    // For global variables as follows,
    //- @gI = global i32 100, align 4
    //- %2 = load i32* @gI, align 4
    // =>
    //- .cpload	$gp
    //- ld	$2, %got(gI)($gp)
    case ISD::GLOBAL_OFFSET_TABLE:
      return getGlobalBaseReg();
    ...
  }

.. rubric:: lbdex/Chapter6_1/Cpu0InstrInfo.td
.. code-block:: c++

  //  Cpu0InstrInfo.td
  def Cpu0GPRel : SDNode<"Cpu0ISD::GPRel", SDTIntUnaryOp>;
  ...
  // gp_rel relocs
  def : Pat<(add CPURegs:$gp, (Cpu0GPRel tglobaladdr:$in)),
            (ADD CPURegs:$gp, (ADDiu ZERO, tglobaladdr:$in))>;


.. code-block:: bash

  	addiu	$2, $gp, %gp_rel(gI)
  	...

Pat<(add CPURegs:$gp, (Cpu0GPRel tglobaladdr:$in)), (ADD CPURegs:$gp, (ADDiu 
ZERO, tglobaladdr:$in))>; will translate (add $gp Cpu0ISD::GPRel tglobaladdr) 
into (add $gp, (addiu ZERO, tglobaladdr)).

In this mode, the $gp content is assigned at compile/link time, changed only at 
program be loaded, and is fixed during the program running; while the 
-relocation-model=pic the $gp can be changed during program running. 
For this example, if $gp is assigned to the start address of .sdata by loader 
when program ch6_1.cpu0.s is loaded, then linker can caculate %gp_rel(gI) = 
(the relative address distance between gI and start of .sdata section. 
Which meaning this relocation record can be solved at link time, that's why it 
is static mode. 

In this mode, we reserve $gp to a specfic fixed address of both linker and 
loader agree to. So, the $gp cannot be allocated as a general purpose for 
variables. The following code tells llvm never allocate $gp for variables.

.. rubric:: lbdex/Chapter6_1/Cpu0Subtarget.cpp
.. code-block:: c++

  Cpu0Subtarget::Cpu0Subtarget(const std::string &TT, const std::string &CPU,
                               const std::string &FS, bool little, 
                               Reloc::Model _RM) :
    Cpu0GenSubtargetInfo(TT, CPU, FS),
    Cpu0ABI(UnknownABI), IsLittle(little), RM(_RM)
  {
    ...
    // Set UseSmallSection.
    UseSmallSection = UseSmallSectionOpt;
    Cpu0ReserveGP = ReserveGPOpt;
    Cpu0NoCpload = NoCploadOpt;
    if (RM == Reloc::Static && !UseSmallSection && !Cpu0ReserveGP)
      FixGlobalBaseReg = false;
    else
      FixGlobalBaseReg = true;
  }

.. rubric:: lbdex/Chapter6_1/Cpu0RegisterInfo.cpp
.. code-block:: c++

  // pure virtual method
  BitVector Cpu0RegisterInfo::
  getReservedRegs(const MachineFunction &MF) const {
    ...
    const Cpu0FunctionInfo *Cpu0FI = MF.getInfo<Cpu0FunctionInfo>();
    // Reserve GP if globalBaseRegFixed()
    if (Cpu0FI->globalBaseRegFixed())
      Reserved.set(Cpu0::GP);
    }
    ...
  }


pic mode
~~~~~~~~~

sdata or sbss
++++++++++++++

Option ``llc -relocation-model=pic -cpu0-use-small-section=true`` will 
generate the following instructions.

.. code-block:: bash

    ...
	  .set	noreorder
	  .cpload	$6
	  .set	nomacro
    ...
  	ld	$2, %got(gI)($gp)
  	ld	$2, 0($2)
    ...
  	.type	gStart,@object          # @gStart
  	.data
  	.globl	gStart
  	.align	2
  gStart:
  	.4byte	2                       # 0x2
  	.size	gStart, 4
  
  	.type	gI,@object              # @gI
  	.globl	gI
  	.align	2
  gI:
  	.4byte	100                     # 0x64
  	.size	gI, 4

The following code fragment of Cpu0AsmPrinter.cpp will emit **.cpload** asm 
pseudo instruction at function entry point as below.

.. rubric:: lbdex/Chapter6_1/Cpu0MachineFunction.h
.. code-block:: c++

  
  //===-- Cpu0MachineFunction.h - Private data used for Cpu0 ----*- C++ -*-=//
  ...
  class Cpu0FunctionInfo : public MachineFunctionInfo {
    virtual void anchor();
    ...

    /// GlobalBaseReg - keeps track of the virtual register initialized for
    /// use as the global base register. This is used for PIC in some PIC
    /// relocation models.
    unsigned GlobalBaseReg;
    int GPFI; // Index of the frame object for restoring $gp
    ...
  
    public:  Cpu0FunctionInfo(MachineFunction& MF)
    : ..., GlobalBaseReg(0), ...
    {}

    bool globalBaseRegFixed() const;
    bool globalBaseRegSet() const;
    unsigned getGlobalBaseReg();
  };
  
  } // end of namespace llvm
  
  #endif // CPU0_MACHINE_FUNCTION_INFO_H

.. rubric:: lbdex/Chapter6_1/Cpu0MachineFunction.cpp
.. literalinclude:: ../../../lib/Target/Cpu0/Cpu0MachineFunction.cpp

.. rubric:: lbdex/Chapter6_1/Cpu0AsmPrinter.cpp
.. code-block:: c++

  /// EmitFunctionBodyStart - Targets can override this to emit stuff before
  /// the first basic block in the function.
  void Cpu0AsmPrinter::EmitFunctionBodyStart() {
    ...
    bool EmitCPLoad = (MF->getTarget().getRelocationModel() == Reloc::PIC_) &&
      Cpu0FI->globalBaseRegSet() &&
      Cpu0FI->globalBaseRegFixed();
    if (OutStreamer.hasRawTextSupport()) {
      ...
      OutStreamer.EmitRawText(StringRef("\t.set\tnoreorder"));
      // Emit .cpload directive if needed.
      if (EmitCPLoad)
        OutStreamer.EmitRawText(StringRef("\t.cpload\t$6"));
      OutStreamer.EmitRawText(StringRef("\t.set\tnomacro"));
      if (Cpu0FI->getEmitNOAT())
        OutStreamer.EmitRawText(StringRef("\t.set\tnoat"));
    } else if (EmitCPLoad) {
      SmallVector<MCInst, 4> MCInsts;
      MCInstLowering.LowerCPLOAD(MCInsts);
      for (SmallVector<MCInst, 4>::iterator I = MCInsts.begin();
         I != MCInsts.end(); ++I)
        OutStreamer.EmitInstruction(*I);
    }
  }

.. code-block:: bash

    ...
	  .set	noreorder
	  .cpload	$6
	  .set	nomacro
    ...

The **.cpload** is the assembly directive (macro) which 
will expand to several instructions. 
Issue **.cpload** before **.set nomacro** since the **.set nomacro** option 
causes the assembler to print a warning whenever 
an assembler operation generates more than one machine language instruction, 
reference Mips ABI [#]_.

Following code will exspand .cpload into machine instructions as below. 
"0fa00000 09aa0000 13aa6000" is the **.cpload** machine instructions 
displayed in comments of Cpu0MCInstLower.cpp.

.. rubric:: lbdex/Chapter6_1/Cpu0MCInstLower.cpp
.. literalinclude:: ../../../lib/Target/Cpu0/Cpu0MCInstLower.cpp
    :start-after: // lbd document - mark - LowerSymbolOperand
    :end-before: MCOperand Cpu0MCInstLower::LowerOperand

.. code-block:: bash

  118-165-76-131:InputFiles Jonathan$ /Users/Jonathan/llvm/test/
  cmake_debug_build/bin/Debug/llc -march=cpu0 -relocation-model=pic -filetype=
  obj ch6_1.bc -o ch6_1.cpu0.o
  118-165-76-131:InputFiles Jonathan$ gobjdump -s ch6_1.cpu0.o 

  ch6_1.cpu0.o:     file format elf32-big

  Contents of section .text:
   0000 0fa00000 09aa0000 13aa6000  ...
  ...

  118-165-76-131:InputFiles Jonathan$ gobjdump -tr ch6_1.cpu0.o 
  ...
  RELOCATION RECORDS FOR [.text]:
  OFFSET   TYPE              VALUE 
  00000000 UNKNOWN           _gp_disp
  00000008 UNKNOWN           _gp_disp
  00000020 UNKNOWN           gI

.. note::

  // **Mips ABI: _gp_disp**
  After calculating the gp, a function allocates the local stack space and saves 
  the gp on the stack, so it can be restored after subsequent function calls. 
  In other words, the gp is a caller saved register. 
  
  ...
  
  _gp_disp represents the offset between the beginning of the function and the 
  global offset table. 
  Various optimizations are possible in this code example and the others that 
  follow. 
  For example, the calculation of gp need not be done for a position-independent 
  function that is strictly local to an object module. 

The _gp_disp as above is a relocation record, it means both the machine 
instructions 09a00000 (offset 0) which equal to assembly 
"addiu $gp, $zero, %hi(_gp_disp)" and 09aa0000 (offset 8) which equal to 
assembly "addiu $gp, $gp, %lo(_gp_disp)" are relocated records depend on 
_gp_disp. The loader or OS can caculate _gp_disp by (x - start address of .data) 
when load the dynamic function into memory x, and adust these two 
instructions offet correctly.
Since shared function is loaded when this function be called, the relocation 
record "ld $2, %got(gI)($gp)" cannot be resolved in link time. 
In spite of the reloation record is solved on load time, the name binding 
is static since linker deliver the memory address to loader and loader can solve 
this just by caculate the offset directly. No need to search the variable name 
at run time.
The ELF relocation records will be introduced in Chapter ELF Support. 
Don't worry, if you don't quite understand it at this point.

The code fragment of LowerGlobalAddress() as the following corresponding option 
``llc -relocation-model=pic`` will translate DAG (GlobalAddress<i32* @gI> 0) into  
(load EntryToken, (Cpu0ISD::Wrapper Register %GP, TargetGlobalAddress<i32* @gI> 0)) 
in stage "Legalized selection DAG" as below.

.. rubric:: lbdex/Chapter6_1/Cpu0ISelLowering.cpp
.. code-block:: c++

  SDValue Cpu0TargetLowering::getAddrGlobal(SDValue Op, SelectionDAG &DAG,
                                            unsigned Flag) const {
    DebugLoc DL = Op.getDebugLoc();
    EVT Ty = Op.getValueType();
    SDValue Tgt = DAG.getNode(Cpu0ISD::Wrapper, DL, Ty, getGlobalReg(DAG, Ty),
                              getTargetNode(Op, DAG, Flag));
    return DAG.getLoad(Ty, DL, DAG.getEntryNode(), Tgt,
                       MachinePointerInfo::getGOT(), false, false, false, 0);
  }

  SDValue Cpu0TargetLowering::LowerGlobalAddress(SDValue Op,
                                                 SelectionDAG &DAG) const {
    ...
    if (TLOF.IsGlobalInSmallSection(GV, getTargetMachine()))
      return getAddrGlobal(Op, DAG, Cpu0II::MO_GOT16);
    ...
  }
    
.. rubric:: lbdex/Chapter6_1/Cpu0ISelDAGToDAG.cpp
.. code-block:: c++

  /// ComplexPattern used on Cpu0InstrInfo
  /// Used on Cpu0 Load/Store instructions
  bool Cpu0DAGToDAGISel::
  SelectAddr(SDNode *Parent, SDValue Addr, SDValue &Base, SDValue &Offset) {
    ...
    // on PIC code Load GA
    if (Addr.getOpcode() == Cpu0ISD::Wrapper) {
      Base   = Addr.getOperand(0);
      Offset = Addr.getOperand(1);
      return true;
    }
    ...
  }

.. code-block:: bash

  ...
  Type-legalized selection DAG: BB#0 '_Z3funv:entry'
  SelectionDAG has 12 nodes:
    ...
        0x7fad7102cc10: <multiple use>
      0x7fad7102cf10: ch = store 0x7fad7102cd10, 0x7fad7102ca10, 0x7fad7102ce10, 
      0x7fad7102cc10<ST4[%c]> [ORD=2] [ID=-3]
  
      0x7fad7102d010: i32 = GlobalAddress<i32* @gI> 0 [ORD=3] [ID=-3]
  
      0x7fad7102cc10: <multiple use>
    0x7fad7102d110: i32,ch = load 0x7fad7102cf10, 0x7fad7102d010, 
    0x7fad7102cc10<LD4[@gI]> [ORD=3] [ID=-3]
    ...
  Legalized selection DAG: BB#0 '_Z3funv:entry'
  SelectionDAG has 15 nodes:
    0x7ff3c9c10b98: ch = EntryToken [ORD=1] [ID=0]
    ...
        0x7fad7102cc10: <multiple use>
      0x7fad7102cf10: ch = store 0x7fad7102cd10, 0x7fad7102ca10, 0x7fad7102ce10, 
      0x7fad7102cc10<ST4[%c]> [ORD=2] [ID=8]
  
        0x7fad70c10b98: <multiple use>
          0x7fad7102d610: i32 = Register %GP
  
          0x7fad7102d310: i32 = TargetGlobalAddress<i32* @gI> 0 [TF=1]
  
        0x7fad7102d710: i32 = Cpu0ISD::Wrapper 0x7fad7102d610, 0x7fad7102d310
  
        0x7fad7102cc10: <multiple use>
      0x7fad7102d810: i32,ch = load 0x7fad70c10b98, 0x7fad7102d710, 
      0x7fad7102cc10<LD4[<unknown>]>
      0x7ff3ca02cc10: <multiple use>
    0x7ff3ca02d110: i32,ch = load 0x7ff3ca02cf10, 0x7ff3ca02d810, 
    0x7ff3ca02cc10<LD4[@gI]> [ORD=3] [ID=9]
    ...


Finally, the pattern Cpu0 instruction **ld** defined before in Cpu0InstrInfo.td 
will translate DAG (load EntryToken, (Cpu0ISD::Wrapper Register %GP, 
TargetGlobalAddress<i32* @gI> 0)) into Cpu0 instruction as below.

.. code-block:: bash

    ...
  	ld	$2, %got(gI)($gp)
    ...

Remind in pic mode, Cpu0 use ".cpload" and "ld $2, %got(gI)($gp)" to access 
global variable. It take 5 instructions in Cpu0 and 4 instructions in Mips. 
The cost came from we didn't assume the register $gp is always assigned to 
address .sdata and fixed there. Even we reserve $gp in this function, the $gp
register can be changed at other functions. In last sub-section, the $gp is
assumed to preserve at any function. If $gp is fixed during the run time, then 
".cpload" can be removed here and have only one instruction cost in global 
variable access. The advantage of ".cpload" removing came from losing one 
general purpose register $gp which can be allocated for variables. 
In last sub-section, .sdata mode, we use ".cpload" removing since it is 
static link, and without ".cpload" will save four instructions which has the 
faster result in speed.
In pic mode, the dynamic loading takes too much time.
Romove ".cpload" with the cost of losing one general purpose register at all
functions is not deserved here. Anyway, in pic mode and used in static link, you
can choose ".cpload" removing. But we perfered use $gp for general purpose 
register as the solution.
The relocation records of ".cpload" from ``llc -relocation-model=pic`` can also 
be solved in link stage if we want to link this function by static link.


data or bss
++++++++++++

The code fragment of LowerGlobalAddress() as the following corresponding option 
``llc -relocation-model=pic`` will translate DAG (GlobalAddress<i32* @gI> 0) into  
(load EntryToken, (Cpu0ISD::Wrapper (add Cpu0ISD::Hi<gI offset Hi16>, Register %GP), 
TargetGlobalAddress<i32* @gI> 0)) 
in stage "Legalized selection DAG" as below.

.. rubric:: lbdex/Chapter6_1/Cpu0ISelLowering.cpp
.. code-block:: c++


  SDValue Cpu0TargetLowering::getAddrGlobalLargeGOT(SDValue Op, SelectionDAG &DAG,
                                                    unsigned HiFlag,
                                                    unsigned LoFlag) const {
    DebugLoc DL = Op.getDebugLoc();
    EVT Ty = Op.getValueType();
    SDValue Hi = DAG.getNode(Cpu0ISD::Hi, DL, Ty, getTargetNode(Op, DAG, HiFlag));
    Hi = DAG.getNode(ISD::ADD, DL, Ty, Hi, getGlobalReg(DAG, Ty));
    SDValue Wrapper = DAG.getNode(Cpu0ISD::Wrapper, DL, Ty, Hi,
                                  getTargetNode(Op, DAG, LoFlag));
    return DAG.getLoad(Ty, DL, DAG.getEntryNode(), Wrapper,
                       MachinePointerInfo::getGOT(), false, false, false, 0);
  }

  SDValue Cpu0TargetLowering::LowerGlobalAddress(SDValue Op,
                                                 SelectionDAG &DAG) const {
    ...
    if (TLOF.IsGlobalInSmallSection(GV, getTargetMachine()))
      ...
    else
      return getAddrGlobalLargeGOT(Op, DAG, Cpu0II::MO_GOT_HI16,
                                   Cpu0II::MO_GOT_LO16);
  }

.. code-block:: bash

  ...
  Type-legalized selection DAG: BB#0 '_Z3funv:'
  SelectionDAG has 10 nodes:
    ...
      0x7fb77a02cd10: ch = store 0x7fb779c10a08, 0x7fb77a02ca10, 0x7fb77a02cb10, 
      0x7fb77a02cc10<ST4[%c]> [ORD=1] [ID=-3]
  
      0x7fb77a02ce10: i32 = GlobalAddress<i32* @gI> 0 [ORD=2] [ID=-3]
  
      0x7fb77a02cc10: <multiple use>
    0x7fb77a02cf10: i32,ch = load 0x7fb77a02cd10, 0x7fb77a02ce10, 
    0x7fb77a02cc10<LD4[@gI]> [ORD=2] [ID=-3]
    ...
  
  Legalized selection DAG: BB#0 '_Z3funv:'
  SelectionDAG has 16 nodes:
    ...
      0x7fb77a02cd10: ch = store 0x7fb779c10a08, 0x7fb77a02ca10, 0x7fb77a02cb10, 
      0x7fb77a02cc10<ST4[%c]> [ORD=1] [ID=6]
  
        0x7fb779c10a08: <multiple use>
              0x7fb77a02d110: i32 = TargetGlobalAddress<i32* @gI> 0 [TF=19]
  
            0x7fb77a02d410: i32 = Cpu0ISD::Hi 0x7fb77a02d110
  
            0x7fb77a02d510: i32 = Register %GP
  
          0x7fb77a02d610: i32 = add 0x7fb77a02d410, 0x7fb77a02d510
  
          0x7fb77a02d710: i32 = TargetGlobalAddress<i32* @gI> 0 [TF=20]
  
        0x7fb77a02d810: i32 = Cpu0ISD::Wrapper 0x7fb77a02d610, 0x7fb77a02d710
  
        0x7fb77a02cc10: <multiple use>
      0x7fb77a02fe10: i32,ch = load 0x7fb779c10a08, 0x7fb77a02d810, 
      0x7fb77a02cc10<LD4[GOT]>
      
      0x7fb77a02cc10: <multiple use>
    0x7fb77a02cf10: i32,ch = load 0x7fb77a02cd10, 0x7fb77a02fe10, 
    0x7fb77a02cc10<LD4[@gI]> [ORD=2] [ID=7]
    ...


Finally, the pattern Cpu0 instruction **ld** defined before in Cpu0InstrInfo.td 
will translate DAG (load EntryToken, (Cpu0ISD::Wrapper (add Cpu0ISD::Hi<gI 
offset Hi16>, Register %GP), Cpu0ISD::Lo<gI offset Lo16>)) into Cpu0 
instructions as below.

.. code-block:: bash

    ...
	  addiu	$2, $zero, %got_hi(gI)
	  shl	$2, $2, 16
	  add	$2, $2, $gp
	  ld	$2, %got_lo(gI)($2)
    ...


Global variable print support
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Above code is for global address DAG translation. 
Next, add the following code to Cpu0MCInstLower.cpp, Cpu0InstPrinter.cpp and 
Cpu0ISelLowering.cpp for global variable printing operand function.

.. rubric:: lbdex/Chapter6_1/Cpu0MCInstLower.cpp
.. code-block:: c++

  MCOperand Cpu0MCInstLower::LowerSymbolOperand(const MachineOperand &MO,
                                                MachineOperandType MOTy,
                                                unsigned Offset) const {
    MCSymbolRefExpr::VariantKind Kind;
    const MCSymbol *Symbol;
    
    switch(MO.getTargetFlags()) {
    default:                   llvm_unreachable("Invalid target flag!"); 
  // Cpu0_GPREL is for llc -march=cpu0 -relocation-model=static 
  //  -cpu0-use-small-section=false (global var in .sdata) 
    case Cpu0II::MO_GPREL:     Kind = MCSymbolRefExpr::VK_Cpu0_GPREL; break; 
    
    case Cpu0II::MO_GOT16:     Kind = MCSymbolRefExpr::VK_Cpu0_GOT16; break; 
    case Cpu0II::MO_GOT:       Kind = MCSymbolRefExpr::VK_Cpu0_GOT; break; 
  // ABS_HI and ABS_LO is for llc -march=cpu0 -relocation-model=static 
  //  (global var in .data) 
    case Cpu0II::MO_ABS_HI:    Kind = MCSymbolRefExpr::VK_Cpu0_ABS_HI; break; 
    case Cpu0II::MO_ABS_LO:    Kind = MCSymbolRefExpr::VK_Cpu0_ABS_LO; break;
    }
    
    switch (MOTy) {
    case MachineOperand::MO_GlobalAddress:
      Symbol = Mang->getSymbol(MO.getGlobal());
      break;
    
    default:
      llvm_unreachable("<unknown operand type>");
    }
    ...
  }
    
  MCOperand Cpu0MCInstLower::LowerOperand(const MachineOperand& MO,
                                            unsigned offset) const {
    MachineOperandType MOTy = MO.getType();
    
    switch (MOTy) {
    ...
    case MachineOperand::MO_GlobalAddress:
      return LowerSymbolOperand(MO, MOTy, offset);
    ...
   }
    
.. rubric:: lbdex/Chapter6_1/InstPrinter/Cpu0InstPrinter.cpp
.. code-block:: c++

  static void printExpr(const MCExpr *Expr, raw_ostream &OS) {
    ...
    switch (Kind) {
    default:                                 llvm_unreachable("Invalid kind!");
    case MCSymbolRefExpr::VK_None:           break;
  // Cpu0_GPREL is for llc -march=cpu0 -relocation-model=static
    case MCSymbolRefExpr::VK_Cpu0_GPREL:     OS << "%gp_rel("; break;
    case MCSymbolRefExpr::VK_Cpu0_GOT16:     OS << "%got(";    break;
    case MCSymbolRefExpr::VK_Cpu0_GOT:       OS << "%got(";    break;
    case MCSymbolRefExpr::VK_Cpu0_ABS_HI:    OS << "%hi(";     break;
    case MCSymbolRefExpr::VK_Cpu0_ABS_LO:    OS << "%lo(";     break;
    }
    ...
  }

The following function is for llc -debug DAG node name printing.
  
.. rubric:: lbdex/Chapter6_1/Cpu0ISelLowering.cpp
.. code-block:: c++

  const char *Cpu0TargetLowering::getTargetNodeName(unsigned Opcode) const {
    switch (Opcode) {
    case Cpu0ISD::JmpLink:           return "Cpu0ISD::JmpLink";
    case Cpu0ISD::Hi:                return "Cpu0ISD::Hi";
    case Cpu0ISD::Lo:                return "Cpu0ISD::Lo";
    case Cpu0ISD::GPRel:             return "Cpu0ISD::GPRel";
    case Cpu0ISD::Ret:               return "Cpu0ISD::Ret";
    case Cpu0ISD::DivRem:            return "MipsISD::DivRem";
    case Cpu0ISD::DivRemU:           return "MipsISD::DivRemU";
    case Cpu0ISD::Wrapper:           return "Cpu0ISD::Wrapper";
    default:                         return NULL;
    }
  }



OS is the output stream which output to the assembly file.


Summary
~~~~~~~~

The global variable Instruction Selection for DAG translation is not like the 
ordinary IR node translation, it has static (absolute address) and PIC mode. 
Backend deals this translation by create DAG nodes in function 
LowerGlobalAddress() which called by LowerOperation(). 
Function LowerOperation() take care all Custom type of operation. 
Backend set global address as Custom operation by 
**”setOperationAction(ISD::GlobalAddress, MVT::i32, Custom);”** in 
Cpu0TargetLowering() constructor. 
Different address mode has it's own DAG list be created. 
By set the pattern Pat<> in Cpu0InstrInfo.td, the llvm can apply the compiler 
mechanism, pattern match, in the Instruction Selection stage.

There are three type for setXXXAction(), Promote, Expand and Custom. 
Except Custom, the other two maybe no need to coding. 
The section "Instruction Selector" of [#]_ is the references.

As shown in the section, the global variable can be laid in 
.sdata/.sbss by option -cpu0-use-small-section=true. 
It is possible, the small data section (16 bits
addressable) is full out at link stage. When this happens, linker will highlight
this error and force the toolchain user to fix it. The toolchain user, need to
reconsider which global variables should be move from .sdata/.sbss to .data/.bss
by set option -cpu0-use-small-section=false for that global variables declared
file. The rule for global variables allocation is "set the small and frequent
variables in small 16 addressable area".



.. _section Global variable:
    http://jonathan2251.github.com/lbd/globalvar.html#global-variable

.. _section Array and struct support:
    http://jonathan2251.github.com/lbd/globalvar.html#array-and-struct-support

.. [#] http://llvm.org/docs/CommandLine.html

.. [#] http://www.linux-mips.org/pub/linux/mips/doc/ABI/mipsabi.pdf

.. [#] http://llvm.org/docs/WritingAnLLVMBackend.html
