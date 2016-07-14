#include "llvm/Transforms/Instrumentation.h"
#include "llvm/ADT/SmallString.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/SmallSet.h"
#include "llvm/Analysis/MemoryDependenceAnalysis.h"
#include "llvm/Analysis/ValueTracking.h"
#include "llvm/ADT/StringExtras.h"
#include "llvm/ADT/Triple.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/Dominators.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/InlineAsm.h"
#include "llvm/IR/InstVisitor.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/MDBuilder.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/ValueMap.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Compiler.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/Transforms/Utils/Local.h"
#include "llvm/Transforms/Utils/ModuleUtils.h"
#include "llvm/Target/TargetLibraryInfo.h"
#include "llvm/Support/Process.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/DepthFirstIterator.h"
#include "llvm/ADT/SetVector.h"
#include "llvm/ADT/SmallSet.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/ADT/StringExtras.h"
#include "llvm/ADT/Triple.h"
#include "llvm/Analysis/MemoryBuiltins.h"
#include "llvm/IR/CallSite.h"
#include "llvm/IR/DIBuilder.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/Dominators.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/InlineAsm.h"
#include "llvm/IR/InstVisitor.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/MDBuilder.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Instructions.h"
#include "llvm/MC/MCSectionMachO.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/DataTypes.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/Endian.h"
#include "llvm/Support/SwapByteOrder.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/Instrumentation.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Transforms/Utils/ASanStackFrameLayout.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/Transforms/Utils/Cloning.h"
#include "llvm/Transforms/Utils/Local.h"
#include "llvm/Transforms/Utils/ModuleUtils.h"
#include "llvm/Transforms/Utils/PromoteMemToReg.h"
#include <algorithm>
#include <string>
#include <system_error>

using namespace llvm;

namespace {
	class AddressShuffler : public FunctionPass {
	 public:
	  AddressShuffler() : FunctionPass(ID) {}
	  const char *getPassName() const override { return "AddressShuffler"; }
	  bool runOnFunction(Function &F) override;
	  bool doInitialization(Module &M) override;
	  static char ID;
	 private:
	 	uint64_t getAllocaSizeInBytes(AllocaInst *AI);
	};
}  // namespace

char AddressShuffler::ID = 0;

INITIALIZE_PASS(AddressShuffler, "shuffler",
                "AddressShuffler: shuffle the address of memory accesses.",
                false, false);

FunctionPass *llvm::createAddressShufflerPass() {
  return new AddressShuffler();
}

bool AddressShuffler::doInitialization(Module &M) {
  // Do nothing.
  return true;
}

uint64_t AddressShuffler::getAllocaSizeInBytes(AllocaInst *AI) {
    uint64_t ArraySize = 1;
    if (AI->isArrayAllocation()) {
      ConstantInt *CI = dyn_cast<ConstantInt>(AI->getArraySize());
      assert(CI && "non-constant array size");
      ArraySize = CI->getZExtValue();
    }
    Type *Ty = AI->getAllocatedType();
    uint64_t SizeInBytes =
        AI->getModule()->getDataLayout().getTypeAllocSize(Ty);
    return SizeInBytes * ArraySize;
}


bool AddressShuffler::runOnFunction(Function &F) {
	llvm::errs() << "Compiling with AddressShuffler\n";
	SmallSet<Value *, 16> TempsToInstrument;
	SmallVector<Instruction *, 16> ToInstrument;
	for (auto &BB : F) {
		for (auto &Inst : BB) {
			ToInstrument.push_back(&Inst);
		}
	}

	int NumInstrumented = 0;
	for (auto Inst : ToInstrument) {
			if(isa<AllocaInst>(Inst)) {
				// handle Alloca instruction
				AllocaInst * AI = dyn_cast<AllocaInst>(Inst);
				// get type of alloca inst
				Type *Ty = AI->getAllocatedType();
				Type * ITy = Type::getInt32Ty(getGlobalContext());
				// get size of alloca inst
				//int size = getAllocaSizeInBytes(AI);
				Constant* AllocSize = ConstantExpr::getSizeOf(Ty);
				AllocSize = ConstantExpr::getTruncOrBitCast(AllocSize, ITy);
				// replacing alloca with malloc

				Instruction * Malloc = llvm::CallInst::CreateMalloc(Inst,
                                             ITy, Ty, AllocSize,
                                             nullptr, nullptr, "");

				Inst->removeFromParent();

				//Malloc->setName(Inst->getName());
				//Inst->eraseFromParent();

				llvm::errs() << "Before instrumentation: " << *Inst << "\n";
				llvm::errs() << "After instrumentation: " << * Malloc << "\n";

			}
			NumInstrumented++;
	}
	

	// We want to instrument every address only once per basic block (unless there
	// are calls between uses).
	/*
	SmallSet<Value *, 16> TempsToInstrument;
	SmallVector<Instruction *, 16> ToInstrument;
	SmallVector<Instruction *, 8> NoReturnCalls;
	SmallVector<BasicBlock *, 16> AllBlocks;
	SmallVector<Instruction *, 16> PointerComparisonsOrSubtracts;
	int NumAllocas = 0;
	bool IsWrite;
	unsigned Alignment;
	uint64_t TypeSize;

	for (auto &BB : F) {
	    AllBlocks.push_back(&BB);
	    TempsToInstrument.clear();
	    int NumInsnsPerBB = 0;
	    for (auto &Inst : BB) {
	      if (LooksLikeCodeInBug11395(&Inst)) return false;
	      if (Value *Addr = isInterestingMemoryAccess(&Inst, &IsWrite, &TypeSize,
	                                                  &Alignment)) {
	        if (ClOpt && ClOptSameTemp) {
	          if (!TempsToInstrument.insert(Addr).second)
	            continue;  // We've seen this temp in the current BB.
	        }
	      } else if (ClInvalidPointerPairs &&
	                 isInterestingPointerComparisonOrSubtraction(&Inst)) {
	        PointerComparisonsOrSubtracts.push_back(&Inst);
	        continue;
	      } else if (isa<MemIntrinsic>(Inst)) {
	        // ok, take it.
	      } else {
	        if (isa<AllocaInst>(Inst)) NumAllocas++;
	        CallSite CS(&Inst);
	        if (CS) {
	          // A call inside BB.
	          TempsToInstrument.clear();
	          if (CS.doesNotReturn()) NoReturnCalls.push_back(CS.getInstruction());
	        }
	        if (CallInst *CI = dyn_cast<CallInst>(&Inst))
	          maybeMarkSanitizerLibraryCallNoBuiltin(CI, TLI);
	        continue;
	      }
	      ToInstrument.push_back(&Inst);
	      NumInsnsPerBB++;
	      if (NumInsnsPerBB >= ClMaxInsnsToInstrumentPerBB) break;
	    }
	}
	*/

	return false;
}

