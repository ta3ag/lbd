//===-- R600MachineScheduler.cpp - R600 Scheduler Interface -*- C++ -*-----===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
/// \file
/// \brief R600 Machine Scheduler interface
//
//===----------------------------------------------------------------------===//

#define DEBUG_TYPE "misched"

#include "R600MachineScheduler.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/LiveIntervalAnalysis.h"
#include "llvm/Pass.h"
#include "llvm/PassManager.h"
#include "llvm/Support/raw_ostream.h"
#include <set>

using namespace llvm;

void R600SchedStrategy::initialize(ScheduleDAGMI *dag) {

  DAG = dag;
  TII = static_cast<const R600InstrInfo*>(DAG->TII);
  TRI = static_cast<const R600RegisterInfo*>(DAG->TRI);
  VLIW5 = !DAG->MF.getTarget().getSubtarget<AMDGPUSubtarget>().hasCaymanISA();
  MRI = &DAG->MRI;
  Available[IDAlu]->clear();
  Available[IDFetch]->clear();
  Available[IDOther]->clear();
  CurInstKind = IDOther;
  CurEmitted = 0;
  OccupedSlotsMask = 31;
  InstKindLimit[IDAlu] = TII->getMaxAlusPerClause();


  const AMDGPUSubtarget &ST = DAG->TM.getSubtarget<AMDGPUSubtarget>();
<<<<<<< HEAD
  if (ST.device()->getGeneration() <= AMDGPUDeviceInfo::HD5XXX) {
    InstKindLimit[IDFetch] = 7; // 8 minus 1 for security
  } else {
    InstKindLimit[IDFetch] = 15; // 16 minus 1 for security
  }
=======
  InstKindLimit[IDFetch] = ST.getTexVTXClauseSize();
  AluInstCount = 0;
  FetchInstCount = 0;
>>>>>>> llvmtrunk/master
}

void R600SchedStrategy::MoveUnits(ReadyQueue *QSrc, ReadyQueue *QDst)
{
  if (QSrc->empty())
    return;
  for (ReadyQueue::iterator I = QSrc->begin(),
      E = QSrc->end(); I != E; ++I) {
    (*I)->NodeQueueId &= ~QSrc->getID();
    QDst->push(*I);
  }
  QSrc->clear();
}

static
unsigned getWFCountLimitedByGPR(unsigned GPRCount) {
  assert (GPRCount && "GPRCount cannot be 0");
  return 248 / GPRCount;
}

SUnit* R600SchedStrategy::pickNode(bool &IsTopNode) {
  SUnit *SU = 0;
  IsTopNode = true;
  NextInstKind = IDOther;

  // check if we might want to switch current clause type
  bool AllowSwitchToAlu = (CurInstKind == IDOther) ||
      (CurEmitted > InstKindLimit[CurInstKind]) ||
      (Available[CurInstKind]->empty());
  bool AllowSwitchFromAlu = (CurEmitted > InstKindLimit[CurInstKind]) &&
      (!Available[IDFetch]->empty() || !Available[IDOther]->empty());

  if (CurInstKind == IDAlu && !Available[IDFetch].empty()) {
    // We use the heuristic provided by AMD Accelerated Parallel Processing
    // OpenCL Programming Guide :
    // The approx. number of WF that allows TEX inst to hide ALU inst is :
    // 500 (cycles for TEX) / (AluFetchRatio * 8 (cycles for ALU))
    float ALUFetchRationEstimate = 
        (AluInstCount + AvailablesAluCount() + Pending[IDAlu].size()) /
        (FetchInstCount + Available[IDFetch].size());
    unsigned NeededWF = 62.5f / ALUFetchRationEstimate;
    DEBUG( dbgs() << NeededWF << " approx. Wavefronts Required\n" );
    // We assume the local GPR requirements to be "dominated" by the requirement
    // of the TEX clause (which consumes 128 bits regs) ; ALU inst before and
    // after TEX are indeed likely to consume or generate values from/for the
    // TEX clause.
    // Available[IDFetch].size() * 2 : GPRs required in the Fetch clause
    // We assume that fetch instructions are either TnXYZW = TEX TnXYZW (need
    // one GPR) or TmXYZW = TnXYZW (need 2 GPR).
    // (TODO : use RegisterPressure)
    // If we are going too use too many GPR, we flush Fetch instruction to lower
    // register pressure on 128 bits regs.
    unsigned NearRegisterRequirement = 2 * Available[IDFetch].size();
    if (NeededWF > getWFCountLimitedByGPR(NearRegisterRequirement))
      AllowSwitchFromAlu = true;
  }

  if (!SU && ((AllowSwitchToAlu && CurInstKind != IDAlu) ||
      (!AllowSwitchFromAlu && CurInstKind == IDAlu))) {
    // try to pick ALU
    SU = pickAlu();
    if (!SU && !PhysicalRegCopy.empty()) {
      SU = PhysicalRegCopy.front();
      PhysicalRegCopy.erase(PhysicalRegCopy.begin());
    }
    if (SU) {
      if (CurEmitted >  InstKindLimit[IDAlu])
        CurEmitted = 0;
      NextInstKind = IDAlu;
    }
  }

  if (!SU) {
    // try to pick FETCH
    SU = pickOther(IDFetch);
    if (SU)
      NextInstKind = IDFetch;
  }

  // try to pick other
  if (!SU) {
    SU = pickOther(IDOther);
    if (SU)
      NextInstKind = IDOther;
  }

  DEBUG(
      if (SU) {
        dbgs() << "picked node: ";
        SU->dump(DAG);
      } else {
        dbgs() << "NO NODE ";
        for (int i = 0; i < IDLast; ++i) {
          Available[i]->dump();
          Pending[i]->dump();
        }
        for (unsigned i = 0; i < DAG->SUnits.size(); i++) {
          const SUnit &S = DAG->SUnits[i];
          if (!S.isScheduled)
            S.dump(DAG);
        }
      }
  );

  return SU;
}

void R600SchedStrategy::schedNode(SUnit *SU, bool IsTopNode) {
<<<<<<< HEAD

  DEBUG(dbgs() << "scheduled: ");
  DEBUG(SU->dump(DAG));

=======
>>>>>>> llvmtrunk/master
  if (NextInstKind != CurInstKind) {
    DEBUG(dbgs() << "Instruction Type Switch\n");
    if (NextInstKind != IDAlu)
      OccupedSlotsMask |= 31;
    CurEmitted = 0;
    CurInstKind = NextInstKind;
  }

  if (CurInstKind == IDAlu) {
    AluInstCount ++;
    switch (getAluKind(SU)) {
    case AluT_XYZW:
      CurEmitted += 4;
      break;
    case AluDiscarded:
      break;
    default: {
      ++CurEmitted;
      for (MachineInstr::mop_iterator It = SU->getInstr()->operands_begin(),
          E = SU->getInstr()->operands_end(); It != E; ++It) {
        MachineOperand &MO = *It;
        if (MO.isReg() && MO.getReg() == AMDGPU::ALU_LITERAL_X)
          ++CurEmitted;
      }
    }
    }
  } else {
    ++CurEmitted;
  }


  DEBUG(dbgs() << CurEmitted << " Instructions Emitted in this clause\n");

  if (CurInstKind != IDFetch) {
    MoveUnits(Pending[IDFetch], Available[IDFetch]);
<<<<<<< HEAD
  }
  MoveUnits(Pending[IDOther], Available[IDOther]);
}

void R600SchedStrategy::releaseTopNode(SUnit *SU) {
  int IK = getInstKind(SU);

  DEBUG(dbgs() << IK << " <= ");
  DEBUG(SU->dump(DAG));

  Pending[IK]->push(SU);
}

void R600SchedStrategy::releaseBottomNode(SUnit *SU) {
=======
  } else
    FetchInstCount++;
}

static bool
isPhysicalRegCopy(MachineInstr *MI) {
  if (MI->getOpcode() != AMDGPU::COPY)
    return false;

  return !TargetRegisterInfo::isVirtualRegister(MI->getOperand(1).getReg());
}

void R600SchedStrategy::releaseTopNode(SUnit *SU) {
  DEBUG(dbgs() << "Top Releasing ";SU->dump(DAG););
}

void R600SchedStrategy::releaseBottomNode(SUnit *SU) {
  DEBUG(dbgs() << "Bottom Releasing ";SU->dump(DAG););
  if (isPhysicalRegCopy(SU->getInstr())) {
    PhysicalRegCopy.push_back(SU);
    return;
  }

  int IK = getInstKind(SU);

  // There is no export clause, we can schedule one as soon as its ready
  if (IK == IDOther)
    Available[IDOther].push_back(SU);
  else
    Pending[IK].push_back(SU);

>>>>>>> llvmtrunk/master
}

bool R600SchedStrategy::regBelongsToClass(unsigned Reg,
                                          const TargetRegisterClass *RC) const {
  if (!TargetRegisterInfo::isVirtualRegister(Reg)) {
    return RC->contains(Reg);
  } else {
    return MRI->getRegClass(Reg) == RC;
  }
}

R600SchedStrategy::AluKind R600SchedStrategy::getAluKind(SUnit *SU) const {
  MachineInstr *MI = SU->getInstr();

  if (TII->isTransOnly(MI))
    return AluTrans;

    switch (MI->getOpcode()) {
    case AMDGPU::INTERP_PAIR_XY:
    case AMDGPU::INTERP_PAIR_ZW:
    case AMDGPU::INTERP_VEC_LOAD:
      return AluT_XYZW;
    case AMDGPU::COPY:
      if (TargetRegisterInfo::isPhysicalRegister(MI->getOperand(1).getReg())) {
        // %vregX = COPY Tn_X is likely to be discarded in favor of an
        // assignement of Tn_X to %vregX, don't considers it in scheduling
        return AluDiscarded;
      }
      else if (MI->getOperand(1).isUndef()) {
        // MI will become a KILL, don't considers it in scheduling
        return AluDiscarded;
      }
    default:
      break;
    }

    // Does the instruction take a whole IG ?
    // XXX: Is it possible to add a helper function in R600InstrInfo that can
    // be used here and in R600PacketizerList::isSoloInstruction() ?
    if(TII->isVector(*MI) ||
        TII->isCubeOp(MI->getOpcode()) ||
        TII->isReductionOp(MI->getOpcode()) ||
        MI->getOpcode() == AMDGPU::GROUP_BARRIER) {
      return AluT_XYZW;
    }

    if (TII->isLDSInstr(MI->getOpcode())) {
      return AluT_X;
    }

    // Is the result already assigned to a channel ?
    unsigned DestSubReg = MI->getOperand(0).getSubReg();
    switch (DestSubReg) {
    case AMDGPU::sub0:
      return AluT_X;
    case AMDGPU::sub1:
      return AluT_Y;
    case AMDGPU::sub2:
      return AluT_Z;
    case AMDGPU::sub3:
      return AluT_W;
    default:
      break;
    }

    // Is the result already member of a X/Y/Z/W class ?
    unsigned DestReg = MI->getOperand(0).getReg();
    if (regBelongsToClass(DestReg, &AMDGPU::R600_TReg32_XRegClass) ||
        regBelongsToClass(DestReg, &AMDGPU::R600_AddrRegClass))
      return AluT_X;
    if (regBelongsToClass(DestReg, &AMDGPU::R600_TReg32_YRegClass))
      return AluT_Y;
    if (regBelongsToClass(DestReg, &AMDGPU::R600_TReg32_ZRegClass))
      return AluT_Z;
    if (regBelongsToClass(DestReg, &AMDGPU::R600_TReg32_WRegClass))
      return AluT_W;
    if (regBelongsToClass(DestReg, &AMDGPU::R600_Reg128RegClass))
      return AluT_XYZW;

    // LDS src registers cannot be used in the Trans slot.
    if (TII->readsLDSSrcReg(MI))
      return AluT_XYZW;

    return AluAny;

}

int R600SchedStrategy::getInstKind(SUnit* SU) {
  int Opcode = SU->getInstr()->getOpcode();

  if (TII->isALUInstr(Opcode)) {
    return IDAlu;
  }

  switch (Opcode) {
  case AMDGPU::COPY:
  case AMDGPU::CONST_COPY:
  case AMDGPU::INTERP_PAIR_XY:
  case AMDGPU::INTERP_PAIR_ZW:
  case AMDGPU::INTERP_VEC_LOAD:
  case AMDGPU::DOT4_eg_pseudo:
  case AMDGPU::DOT4_r600_pseudo:
    return IDAlu;
  case AMDGPU::TEX_VTX_CONSTBUF:
  case AMDGPU::TEX_VTX_TEXBUF:
  case AMDGPU::TEX_LD:
  case AMDGPU::TEX_GET_TEXTURE_RESINFO:
  case AMDGPU::TEX_GET_GRADIENTS_H:
  case AMDGPU::TEX_GET_GRADIENTS_V:
  case AMDGPU::TEX_SET_GRADIENTS_H:
  case AMDGPU::TEX_SET_GRADIENTS_V:
  case AMDGPU::TEX_SAMPLE:
  case AMDGPU::TEX_SAMPLE_C:
  case AMDGPU::TEX_SAMPLE_L:
  case AMDGPU::TEX_SAMPLE_C_L:
  case AMDGPU::TEX_SAMPLE_LB:
  case AMDGPU::TEX_SAMPLE_C_LB:
  case AMDGPU::TEX_SAMPLE_G:
  case AMDGPU::TEX_SAMPLE_C_G:
  case AMDGPU::TXD:
  case AMDGPU::TXD_SHADOW:
    return IDFetch;
  default:
    DEBUG(
        dbgs() << "other inst: ";
        SU->dump(DAG);
    );
    return IDOther;
  }
}

<<<<<<< HEAD
SUnit *R600SchedStrategy::PopInst(std::multiset<SUnit *, CompareSUnit> &Q) {
=======
SUnit *R600SchedStrategy::PopInst(std::vector<SUnit *> &Q, bool AnyALU) {
>>>>>>> llvmtrunk/master
  if (Q.empty())
    return NULL;
  for (std::set<SUnit *, CompareSUnit>::iterator It = Q.begin(), E = Q.end();
      It != E; ++It) {
    SUnit *SU = *It;
    InstructionsGroupCandidate.push_back(SU->getInstr());
    if (TII->fitsConstReadLimitations(InstructionsGroupCandidate)
        && (!AnyALU || !TII->isVectorOnly(SU->getInstr()))
    ) {
      InstructionsGroupCandidate.pop_back();
      Q.erase(It);
      return SU;
    } else {
      InstructionsGroupCandidate.pop_back();
    }
  }
  return NULL;
}

void R600SchedStrategy::LoadAlu() {
  ReadyQueue *QSrc = Pending[IDAlu];
  for (ReadyQueue::iterator I = QSrc->begin(),
        E = QSrc->end(); I != E; ++I) {
      (*I)->NodeQueueId &= ~QSrc->getID();
      AluKind AK = getAluKind(*I);
      AvailableAlus[AK].insert(*I);
    }
    QSrc->clear();
}

void R600SchedStrategy::PrepareNextSlot() {
  DEBUG(dbgs() << "New Slot\n");
  assert (OccupedSlotsMask && "Slot wasn't filled");
  OccupedSlotsMask = 0;
//  if (HwGen == AMDGPUSubtarget::NORTHERN_ISLANDS)
//    OccupedSlotsMask |= 16;
  InstructionsGroupCandidate.clear();
  LoadAlu();
}

void R600SchedStrategy::AssignSlot(MachineInstr* MI, unsigned Slot) {
  int DstIndex = TII->getOperandIdx(MI->getOpcode(), AMDGPU::OpName::dst);
  if (DstIndex == -1) {
    return;
  }
  unsigned DestReg = MI->getOperand(DstIndex).getReg();
  // PressureRegister crashes if an operand is def and used in the same inst
  // and we try to constraint its regclass
  for (MachineInstr::mop_iterator It = MI->operands_begin(),
      E = MI->operands_end(); It != E; ++It) {
    MachineOperand &MO = *It;
    if (MO.isReg() && !MO.isDef() &&
        MO.getReg() == DestReg)
      return;
  }
  // Constrains the regclass of DestReg to assign it to Slot
  switch (Slot) {
  case 0:
    MRI->constrainRegClass(DestReg, &AMDGPU::R600_TReg32_XRegClass);
    break;
  case 1:
    MRI->constrainRegClass(DestReg, &AMDGPU::R600_TReg32_YRegClass);
    break;
  case 2:
    MRI->constrainRegClass(DestReg, &AMDGPU::R600_TReg32_ZRegClass);
    break;
  case 3:
    MRI->constrainRegClass(DestReg, &AMDGPU::R600_TReg32_WRegClass);
    break;
  }
}

SUnit *R600SchedStrategy::AttemptFillSlot(unsigned Slot, bool AnyAlu) {
  static const AluKind IndexToID[] = {AluT_X, AluT_Y, AluT_Z, AluT_W};
<<<<<<< HEAD
  SUnit *SlotedSU = PopInst(AvailableAlus[IndexToID[Slot]]);
  SUnit *UnslotedSU = PopInst(AvailableAlus[AluAny]);
  if (!UnslotedSU) {
    return SlotedSU;
  } else if (!SlotedSU) {
=======
  SUnit *SlotedSU = PopInst(AvailableAlus[IndexToID[Slot]], AnyAlu);
  if (SlotedSU)
    return SlotedSU;
  SUnit *UnslotedSU = PopInst(AvailableAlus[AluAny], AnyAlu);
  if (UnslotedSU)
>>>>>>> llvmtrunk/master
    AssignSlot(UnslotedSU->getInstr(), Slot);
    return UnslotedSU;
  } else {
    //Determine which one to pick (the lesser one)
    if (CompareSUnit()(SlotedSU, UnslotedSU)) {
      AvailableAlus[AluAny].insert(UnslotedSU);
      return SlotedSU;
    } else {
      AvailableAlus[IndexToID[Slot]].insert(SlotedSU);
      AssignSlot(UnslotedSU->getInstr(), Slot);
      return UnslotedSU;
    }
  }
}

<<<<<<< HEAD
bool R600SchedStrategy::isAvailablesAluEmpty() const {
  return Pending[IDAlu]->empty() && AvailableAlus[AluAny].empty() &&
      AvailableAlus[AluT_XYZW].empty() && AvailableAlus[AluT_X].empty() &&
      AvailableAlus[AluT_Y].empty() && AvailableAlus[AluT_Z].empty() &&
      AvailableAlus[AluT_W].empty() && AvailableAlus[AluDiscarded].empty();
=======
unsigned R600SchedStrategy::AvailablesAluCount() const {
  return AvailableAlus[AluAny].size() + AvailableAlus[AluT_XYZW].size() +
      AvailableAlus[AluT_X].size() + AvailableAlus[AluT_Y].size() +
      AvailableAlus[AluT_Z].size() + AvailableAlus[AluT_W].size() +
      AvailableAlus[AluTrans].size() + AvailableAlus[AluDiscarded].size() +
      AvailableAlus[AluPredX].size();
>>>>>>> llvmtrunk/master
}

SUnit* R600SchedStrategy::pickAlu() {
  while (AvailablesAluCount() || !Pending[IDAlu].empty()) {
    if (!OccupedSlotsMask) {
<<<<<<< HEAD
=======
      // Bottom up scheduling : predX must comes first
      if (!AvailableAlus[AluPredX].empty()) {
        OccupedSlotsMask |= 31;
        return PopInst(AvailableAlus[AluPredX], false);
      }
>>>>>>> llvmtrunk/master
      // Flush physical reg copies (RA will discard them)
      if (!AvailableAlus[AluDiscarded].empty()) {
        OccupedSlotsMask |= 31;
        return PopInst(AvailableAlus[AluDiscarded], false);
      }
      // If there is a T_XYZW alu available, use it
      if (!AvailableAlus[AluT_XYZW].empty()) {
        OccupedSlotsMask |= 15;
        return PopInst(AvailableAlus[AluT_XYZW], false);
      }
    }
    bool TransSlotOccuped = OccupedSlotsMask & 16;
    if (!TransSlotOccuped && VLIW5) {
      if (!AvailableAlus[AluTrans].empty()) {
        OccupedSlotsMask |= 16;
        return PopInst(AvailableAlus[AluTrans], false);
      }
      SUnit *SU = AttemptFillSlot(3, true);
      if (SU) {
        OccupedSlotsMask |= 16;
        return SU;
      }
    }
    for (unsigned Chan = 0; Chan < 4; ++Chan) {
      bool isOccupied = OccupedSlotsMask & (1 << Chan);
      if (!isOccupied) {
        SUnit *SU = AttemptFillSlot(Chan, false);
        if (SU) {
          OccupedSlotsMask |= (1 << Chan);
          InstructionsGroupCandidate.push_back(SU->getInstr());
          return SU;
        }
      }
    }
    PrepareNextSlot();
  }
  return NULL;
}

SUnit* R600SchedStrategy::pickOther(int QID) {
  SUnit *SU = 0;
  ReadyQueue *AQ = Available[QID];

  if (AQ->empty()) {
    MoveUnits(Pending[QID], AQ);
  }
  if (!AQ->empty()) {
    SU = *AQ->begin();
    AQ->remove(AQ->begin());
  }
  return SU;
}

