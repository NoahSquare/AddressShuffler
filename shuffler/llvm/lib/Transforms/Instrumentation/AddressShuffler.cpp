#include "llvm/ADT/SmallString.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/DepthFirstIterator.h"
#include "llvm/ADT/SetVector.h"
#include "llvm/ADT/SmallSet.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/ADT/StringExtras.h"
#include "llvm/ADT/Triple.h"
#include "llvm/Analysis/MemoryBuiltins.h"
#include "llvm/Analysis/MemoryDependenceAnalysis.h"
#include "llvm/Analysis/ValueTracking.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/Dominators.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/CallSite.h"
#include "llvm/IR/DIBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/InlineAsm.h"
#include "llvm/IR/InstVisitor.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/MDBuilder.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/ValueMap.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/Support/Process.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/DataTypes.h"
#include "llvm/Support/Endian.h"
#include "llvm/Support/SwapByteOrder.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Compiler.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/Instrumentation.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/Transforms/Utils/Local.h"
#include "llvm/Transforms/Utils/ModuleUtils.h"
#include "llvm/Transforms/Utils/ASanStackFrameLayout.h"
#include "llvm/Transforms/Utils/Cloning.h"
#include "llvm/Transforms/Utils/PromoteMemToReg.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Target/TargetLibraryInfo.h"
#include "llvm/MC/MCSectionMachO.h"
#include <algorithm>
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

void warningMessage() {
	llvm::errs() << "====================================================\n";
	llvm::errs() << "-                                                  -\n";
	llvm::errs() << "-                                                  -\n";
	llvm::errs() << "-  Important: Compiling with AddressShuffler On!   -\n";
	llvm::errs() << "-                                                  -\n";
	llvm::errs() << "-                                                  -\n";
	llvm::errs() << "====================================================\n";
}

bool AddressShuffler::runOnFunction(Function &F) {
	LLVMContext& Ctx = F.getContext();
	warningMessage();
	int LongSize = F.getParent()->getDataLayout().getPointerSizeInBits();
	//Type * IntptrTy = Type::getIntNTy(Ctx, LongSize);
	Type * IntptrTy = IntegerType::getInt32Ty(Ctx);
	SmallSet<Value *, 16> TempsToInstrument;
	SmallVector<Instruction *, 16> ToInstrument;
	AllocaInst *DynamicAllocaLayout = nullptr;

	for (auto &BB : F) {
		for (auto &Inst : BB) {
			ToInstrument.push_back(&Inst);
		}
	}


	// Maps Alloca Value to an AllocaInst from which the Value is originated.
	typedef DenseMap<Value *, AllocaInst *> htlMapTy;
  	htlMapTy htlmap;

	int NumInstrumented = 0;
	int init_flag = 0;

	for (auto Inst : ToInstrument) {
		/*
		if(init_flag == 0) {

			// Initialize Shuffler
			Constant* initFunc = F.getParent()->getOrInsertFunction(
			  "_shuffler_init", Type::getVoidTy(Ctx),Type::getInt32Ty(Ctx), NULL);
			IRBuilder<> IRB(Inst, nullptr, None);
			//IRB.SetInsertPoint(Inst->getParent(), IRB.GetInsertPoint());
			IRB.CreateCall(initFunc, {}, "");
			init_flag = 1;
		}*/


		if(isa<AllocaInst>(Inst)) {
			// Handle Alloca instructions
			AllocaInst * AI = dyn_cast<AllocaInst>(Inst);
			// Get type of alloca inst
			Type *Ty = AI->getAllocatedType();
			// Get size of alloca inst
			Constant* AllocSize = ConstantExpr::getSizeOf(Ty);
			AllocSize = ConstantExpr::getTruncOrBitCast(AllocSize, IntptrTy);
			// Insert tmp malloc instruction
			Instruction * Malloc = llvm::CallInst::CreateMalloc(Inst,
                                         IntptrTy, Ty, AllocSize,
                                         nullptr, nullptr, "");

			AI->replaceAllUsesWith(Malloc);
			AI->removeFromParent();

			Constant* saveFunc = F.getParent()->getOrInsertFunction(
			  "_save_mapping", Type::getVoidTy(Ctx),IntptrTy, NULL);
			IRBuilder<> builder(Malloc, nullptr, None);
			// Insert after Store Instruction
			builder.SetInsertPoint(Malloc->getParent(), ++builder.GetInsertPoint());

			builder.CreateCall(saveFunc, {builder.CreatePtrToInt(Malloc, IntptrTy)}, "calltmp");
		}
		else if(isa<StoreInst>(Inst)) {

		}
		else if(isa<LoadInst>(Inst)) {
			/*
			Constant* tmpAddr = ConstantInt::get(Type::getInt32Ty(Ctx), 0x1234);
			Value* tmpValue = ConstantExpr::getIntToPtr(
			    tmpAddr , PointerType::getUnqual(Type::getInt32Ty(Ctx)));

			Constant* newAddr = ConstantInt::get(Type::getInt32Ty(Ctx), (uint64_t)tmpValue);
			Value* newValue = ConstantExpr::getIntToPtr(
			    newAddr , PointerType::getUnqual(Type::getInt32Ty(Ctx)));
			    */

			// Handle Load instructions
			LoadInst * LI = dyn_cast<LoadInst>(Inst);
			// Debugging Load value from malloc memory space

			Constant* loadFunc = F.getParent()->getOrInsertFunction(
			  "_load_mapping", Type::getVoidTy(Ctx),IntptrTy, NULL);
			IRBuilder<> builder(LI, nullptr, None);
			// Insert before Load instruction
			builder.SetInsertPoint(LI->getParent(), ++builder.GetInsertPoint());
			//builder.CreateCall(loadFunc, {newValue}, "rettmp");
			builder.SetInsertPoint(LI->getParent(), --builder.GetInsertPoint());
			DynamicAllocaLayout = builder.CreateAlloca(IntptrTy, nullptr);
			builder.CreateCall(loadFunc,
               {DynamicAllocaLayout}, "rettmp");
			//builder.CreateLoad(DynamicAllocaLayout);

			LoadInst * mallocLoad = new LoadInst(DynamicAllocaLayout,"",Inst);

			LI->removeFromParent();
			LI->replaceAllUsesWith(mallocLoad);

		}

		NumInstrumented++;
	}

	/****************************************/
	/*										*/
	/*      Reference code from Asan        */
	/*										*/
	/****************************************/
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

