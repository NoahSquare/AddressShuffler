#include "llvm/ADT/SmallString.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/SetVector.h"
#include "llvm/ADT/SmallSet.h"
#include "llvm/Analysis/ValueTracking.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/ValueMap.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/Support/DataTypes.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/Instrumentation.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"

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
    AllocaInst *DynamicAllocaLayout = nullptr;
  };
}  // namespace



char AddressShuffler::ID = 0;

INITIALIZE_PASS( AddressShuffler, "shuffler",
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
  

  llvm::errs() << "Instrumenting function " << F.getName() << "\n";
  
  LLVMContext& Ctx = F.getContext();
  const DataLayout &DL = F.getParent()->getDataLayout();
  warningMessage();
  int LongSize = F.getParent()->getDataLayout().getPointerSizeInBits();
  //Type * IntptrTy = Type::getIntNTy(Ctx, LongSize);
  Type * IntptrTy = IntegerType::getInt64Ty(Ctx);
  Type * IntptrPtrTy = PointerType::get(IntptrTy, 0);
  SmallSet<Value *, 16> TempsToInstrument;
  SmallVector<Instruction *, 16> ToInstrument;

  for (auto &BB : F) {
    for (auto &Inst : BB) {
      ToInstrument.push_back(&Inst);
    }
  }

  int NumInstrumented = 0;
  int init_flag = 0;
  Value * testv = NULL;

  for (auto Inst : ToInstrument) {
    if(init_flag == 0) {
      // Initialieze Dynamic Alloca Storage
      IRBuilder<> IRB(Inst, nullptr, None);
      DynamicAllocaLayout = IRB.CreateAlloca(IntptrTy, nullptr);
      IRB.CreateStore(Constant::getNullValue(IntptrTy), DynamicAllocaLayout);
      DynamicAllocaLayout->setAlignment(32);
      init_flag = 1;
    }
    
    if(isa<AllocaInst>(Inst)) {
      // Handle Alloca instructions
      AllocaInst * AI = dyn_cast<AllocaInst>(Inst);
      //llvm::errs() << "Found malloc: isStaticAlloca()? " << AI->isStaticAlloca() << "\n ";
      // Get type of alloca inst
      Type *Ty = AI->getAllocatedType();
      // Get size of alloca inst
      Constant* AllocSize = ConstantExpr::getSizeOf(Ty);
      AllocSize = ConstantExpr::getTruncOrBitCast(AllocSize, IntptrTy);
      // Insert tmp malloc instruction
      Instruction * Malloc = llvm::CallInst::CreateMalloc(Inst,
                                         IntptrTy, Ty, AllocSize,
                                         nullptr, nullptr, "");

      uint64_t SizeInBytes = AI->getModule()->getDataLayout().getTypeStoreSize(Ty);
      Value * SizeInBytesV = ConstantInt::get(Type::getInt64Ty(Ctx), SizeInBytes);

      Constant* saveFunc = F.getParent()->getOrInsertFunction(
        "_save_mapping", Type::getVoidTy(Ctx),IntptrTy, NULL);
      IRBuilder<> builder(Malloc, nullptr, None);
      // Insert after Store Instruction
      builder.SetInsertPoint(Malloc->getParent(), ++builder.GetInsertPoint());
      builder.CreateCall(saveFunc, { builder.CreatePtrToInt(AI, IntptrTy), builder.CreatePtrToInt(Malloc, IntptrTy), SizeInBytesV }, "allocatmp");
      //AI->removeFromParent();
      
    }
    else if(isa<StoreInst>(Inst)) {
      // Handle Store instructions
      StoreInst * SI = dyn_cast<StoreInst>(Inst);
      Value * value = SI -> getValueOperand();
      Constant* loadFunc = F.getParent()->getOrInsertFunction(
        "_load_mapping", Type::getVoidTy(Ctx),IntptrTy, NULL);
      IRBuilder<> builder(SI, nullptr, None);
      DynamicAllocaLayout = builder.CreateAlloca(IntptrTy, nullptr);
      builder.CreateCall(loadFunc, { builder.CreatePtrToInt(SI -> getPointerOperand(), IntptrTy), DynamicAllocaLayout}, "storetmp");
      Value * tmpLoad = builder.CreateLoad(DynamicAllocaLayout);
      StoreInst * newStore = builder.CreateStore(value, builder.CreateIntToPtr(tmpLoad, IntptrPtrTy));
      //newStore->setAlignment(SI->getAlignment());
      // Remove SI instruction
      SI->removeFromParent();
    }
    else if(isa<LoadInst>(Inst)) {
      // Handle Load instructions
      LoadInst * LI = dyn_cast<LoadInst>(Inst);
      // Load value from malloc memory space
      Constant* loadFunc = F.getParent()->getOrInsertFunction(
        "_load_mapping", Type::getVoidTy(Ctx),IntptrTy, NULL);
      IRBuilder<> builder(LI, nullptr, None);
      // Insert before Load instruction
      builder.SetInsertPoint(LI->getParent(), ++builder.GetInsertPoint());
      //DynamicAllocaLayout = builder.CreateAlloca(IntptrTy, nullptr);
      DynamicAllocaLayout = builder.CreateAlloca(IntptrTy, nullptr);
      builder.CreateCall(loadFunc, { builder.CreatePtrToInt(LI -> getPointerOperand(), IntptrTy), DynamicAllocaLayout}, "loadtmp");
      Value * tmpLoad = builder.CreateLoad(DynamicAllocaLayout);

      Type * Int32ptrTy = IntegerType::getInt32Ty(Ctx);
      Type * Int32ptrPtrTy = PointerType::get(Int32ptrTy, 0);

      LoadInst * mallocLoad = builder.CreateLoad(LI->getType(), builder.CreateIntToPtr(tmpLoad, Int32ptrPtrTy), "");

      // Reallocate and update mapping info
      Constant* updateFunc = F.getParent()->getOrInsertFunction(
        "_update_mapping", Type::getVoidTy(Ctx),IntptrTy, NULL);
      
      builder.CreateCall(updateFunc, { builder.CreatePtrToInt(LI -> getPointerOperand(), IntptrTy)}, "updatetmp");

      // Remove LI instruction
      LI->replaceAllUsesWith(mallocLoad);
      LI->removeFromParent();
    }
    
    NumInstrumented++;
  }

  return false;
}
