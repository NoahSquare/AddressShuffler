#include <stdio.h>
#include <sys/mman.h>
#include "llvm/IR/Value.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/IR/ValueMap.h"

typedef uintptr_t   uptr;

int a;
uptr tmp;
//extern "C" llvm::DenseMap<llvm::Value *, llvm::Value *> shuffler_map;

extern "C" void _save_mapping(void * v) {
	tmp = reinterpret_cast<uptr>(v);
	printf("pointer = %x\n", tmp);
	//syntax_error, should be detected if get compiled.
}

extern "C" void _load_mapping(void * ret) {
	//uptr * tmpret = reinterpret_cast<uptr*>(ret);
	//*tmpret = tmp;
	//syntax_error, should be detected if get compiled.
}