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
  class AddressShufflerModule : public ModulePass {
   public:
    AddressShufflerModule() : ModulePass(ID) {}
    bool runOnModule(Module &M) override;
    bool doInitialization(Module &M) override;
    static char ID;  // Pass identification, replacement for typeid
    const char *getPassName() const override { return "AddressShufflerModule"; }
  };
}  // namespace

bool AddressShufflerModule::doInitialization(Module &M) {
  // Do nothing.
  return true;
}

ModulePass *llvm::createAddressShufflerModulePass() {
  return new AddressShufflerModule();
}

INITIALIZE_PASS( AddressShufflerModule, "shuffler-module",
    "AddressShufflerModule: shuffle the address of memory accesses.",
    false, false);

char AddressShufflerModule::ID = 0;

// Broken: Instrument global variables
bool AddressShufflerModule::runOnModule(Module &M) {
  
  llvm::errs() << "Running On Module ";
  llvm::errs() << M.getModuleIdentifier() << "\n";

  const DataLayout &DL = M.getDataLayout();
  LLVMContext & Ctx = M.getContext();
  SmallVector<GlobalVariable *, 16> globalvariables;
  Type * IntptrTy = IntegerType::getInt64Ty(Ctx);
  Type * IntptrPtrTy = PointerType::get(IntptrTy, 0);

  // Get first instruction of the program
    Function * main = M.getFunction("main");
    BasicBlock &FirstBB = *main->begin();
    Instruction * inst = dyn_cast<Instruction>(FirstBB.begin());

    // Initialize Asan Allocator
    Constant* initFunc = M.getOrInsertFunction(
      "__asan_init", Type::getVoidTy(Ctx),Type::getInt32Ty(Ctx), NULL);
    IRBuilder<> builder(inst);
    builder.CreateCall(initFunc, {}, "");

  for(auto &GV : M.getGlobalList()) {
    if (isa<GlobalVariable>(GV)) {
      GlobalVariable* G = dyn_cast<GlobalVariable>(&GV);
      llvm::errs() << "push back global: " << G->getName() << "\n";
      globalvariables.push_back(G);
    }
  }

  for (auto G : globalvariables) {
    llvm:errs() << "handling global: " << G->getName() << "\n";
    if (G->hasInitializer()) {
      llvm::errs() << " global " << G->getName() << " has an initializer!\n";
      //Constant * init = GV->getInitializer();
      Type *Ty = G->getValueType();
      llvm::errs() << " type: ";
      Ty->print(errs()); 
      llvm::errs() << "\n";
      uint64_t SizeInBytes = DL.getTypeAllocSize(Ty);
      llvm::errs() << " SizeInBytes: " << SizeInBytes << "\n";

      // Malloc for globals
      Constant* AllocSize = ConstantExpr::getSizeOf(Ty);
      AllocSize = ConstantExpr::getTruncOrBitCast(AllocSize, IntptrTy);
      Instruction * Malloc = llvm::CallInst::CreateMalloc(inst,
                                         IntptrTy, Ty, AllocSize,
                                         nullptr, nullptr, "");
      IRBuilder<> IRB(Malloc, nullptr, None);
      Constant* saveFunc = M.getOrInsertFunction(
            "_save_mapping", Type::getVoidTy(Ctx),IntptrTy, NULL);
      IRB.SetInsertPoint(Malloc->getParent(), ++IRB.GetInsertPoint());
      IRB.CreateCall(saveFunc, { IRB.CreatePtrToInt(G, IntptrTy), IRB.CreatePtrToInt(Malloc, IntptrTy) }, "globalAllocatmp");
    }
  }

  return false;
}

