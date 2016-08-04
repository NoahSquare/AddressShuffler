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
#include "llvm/Analysis/TargetLibraryInfo.h"
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
  Type * IntptrPtrTy = PointerType::get(IntptrTy, 0);
  SmallSet<Value *, 16> TempsToInstrument;
  SmallVector<Instruction *, 16> ToInstrument;
  AllocaInst *DynamicAllocaLayout = nullptr;

  for (auto &BB : F) {
  	llvm::errs() << "\n";
    for (auto &Inst : BB) {
    	Inst.print(errs());
    	llvm::errs() << "\n";
      ToInstrument.push_back(&Inst);
    }
  }

  int NumInstrumented = 0;
  int init_flag = 0;

  for (auto Inst : ToInstrument) {
    if(init_flag == 0) {
      // Initialize Shuffler
      Constant* initFunc = F.getParent()->getOrInsertFunction(
        "_shuffler_init", Type::getVoidTy(Ctx),Type::getInt32Ty(Ctx), NULL);
      IRBuilder<> IRB(Inst, nullptr, None);
      //IRB.SetInsertPoint(Inst->getParent(), IRB.GetInsertPoint());
      IRB.CreateCall(initFunc, {}, "");
      init_flag = 1;
    }


    if(isa<AllocaInst>(Inst)) {
    	llvm::errs() << "S Alloca\n";
    	Inst->print(errs());
    	llvm::errs() << "\n";
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

      Constant* saveFunc = F.getParent()->getOrInsertFunction(
        "_save_mapping", Type::getVoidTy(Ctx),IntptrTy, NULL);
      IRBuilder<> builder(Malloc, nullptr, None);
      // Insert after Store Instruction
      builder.SetInsertPoint(Malloc->getParent(), ++builder.GetInsertPoint());

      builder.CreateCall(saveFunc, { builder.CreatePtrToInt(AI, IntptrTy)/*mapFrom*/, builder.CreatePtrToInt(Malloc, IntptrTy)/*mapTo*/, AllocSize/*size*/ }, "savetmp");

      //AI->replaceAllUsesWith(Malloc);
      AI->removeFromParent();
      llvm::errs() << "E Alloca\n";
    }
    else if(isa<StoreInst>(Inst)) {
    	llvm::errs() << "S Store\n";
    	Inst->print(errs());
    	llvm::errs() << "\n";
      StoreInst * SI = dyn_cast<StoreInst>(Inst);
      Value * value = SI -> getValueOperand();

      Constant* loadFunc = F.getParent()->getOrInsertFunction(
        "_load_mapping", Type::getVoidTy(Ctx),IntptrTy, NULL);
      IRBuilder<> builder(SI, nullptr, None);
      DynamicAllocaLayout = builder.CreateAlloca(IntptrTy, nullptr);
      builder.CreateCall(loadFunc, { SI -> getPointerOperand()/*mapFrom*/, DynamicAllocaLayout}, "storetmp");
      Value * tmpLoad = builder.CreateLoad(DynamicAllocaLayout);
      StoreInst * newStore = builder.CreateStore(value, builder.CreateIntToPtr(tmpLoad, IntptrPtrTy));
      newStore->setAlignment(SI->getAlignment());

      llvm::errs() << "PointerOperand:" << SI -> getPointerOperand()->getName() << "\n";
      
      // Remove SI instruction
      SI->removeFromParent();
      llvm::errs() << "E Store\n";
    }
    else if(isa<LoadInst>(Inst)) {
    	llvm::errs() << "S Load\n";
    	Inst->print(errs());
    	llvm::errs() << "\n";
      // Handle Load instructions
      LoadInst * LI = dyn_cast<LoadInst>(Inst);
      // Debugging Load value from malloc memory space
      Constant* loadFunc = F.getParent()->getOrInsertFunction(
        "_load_mapping", Type::getVoidTy(Ctx),IntptrTy, NULL);
      IRBuilder<> builder(LI, nullptr, None);
      // Insert before Load instruction
      builder.SetInsertPoint(LI->getParent(), ++builder.GetInsertPoint());
      DynamicAllocaLayout = builder.CreateAlloca(IntptrTy, nullptr);
      builder.CreateCall(loadFunc, { LI -> getPointerOperand()/*mapFrom*/, DynamicAllocaLayout}, "loadtmp");
      Value * tmpLoad = builder.CreateLoad(DynamicAllocaLayout);
      LoadInst * mallocLoad = builder.CreateLoad(builder.CreateIntToPtr(tmpLoad, IntptrPtrTy));

      // Reallocate to achieve shuffling
      // Get type of loadinst
      Type *Ty = LI->getPointerOperand()->getType();
      // Get size of loadinst
      Constant* AllocSize = ConstantExpr::getSizeOf(Ty);
      AllocSize = ConstantExpr::getTruncOrBitCast(AllocSize, IntptrTy);
      // Insert new malloc instruction
      Instruction * Malloc = llvm::CallInst::CreateMalloc(Inst,
                                         IntptrTy, Ty, AllocSize,
                                         nullptr, nullptr, "");
      Constant* updateFunc = F.getParent()->getOrInsertFunction(
        "_update_mapping", Type::getVoidTy(Ctx),IntptrTy, NULL);
      
      builder.CreateCall(updateFunc, { LI -> getPointerOperand()/*mapFrom*//*mapFrom*/, builder.CreatePtrToInt(Malloc, IntptrTy)/*new mapTo*/}, "updatetmp");

      // Copy the content to the new address
      Value * value = LI;
      StoreInst * newStore = builder.CreateStore(value, Malloc, IntptrPtrTy);

      // Remove LI instruction
      LI->replaceAllUsesWith(mallocLoad);
      LI->removeFromParent();
      llvm::errs() << "E Load\n";
    }

    NumInstrumented++;
  }

  return false;
}
