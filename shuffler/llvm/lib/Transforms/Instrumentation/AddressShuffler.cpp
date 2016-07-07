#include "llvm/Transforms/Instrumentation.h"
#include "llvm/ADT/DepthFirstIterator.h"
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

using namespace llvm;

namespace {

	class AddressShuffler : public FunctionPass {
	 public:
	  AddressShuffler() : FunctionPass(ID) {}
	  const char *getPassName() const override { return "AddressShuffler"; }
	  bool runOnFunction(Function &F) override;
	  bool doInitialization(Module &M) override;
	  static char ID;
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

bool AddressShuffler::runOnFunction(Function &F) {
  errs() << "Compiling with AddressShuffler\n";
  return false;
}