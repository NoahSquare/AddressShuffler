#include <stdio.h>
#include <sys/mman.h>
#include "llvm/IR/Value.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/IR/ValueMap.h"

typedef uintptr_t   uptr;

uptr tmp;
void * ptr = malloc(32);

extern "C" void _save_mapping(void * v) {
	//ptr = malloc(32);
	memcpy(ptr, v, 32);
	tmp = reinterpret_cast<uptr>(v);
	printf("pointer v = %lx\n", tmp);
}

extern "C" void _load_mapping(void ** ret) {
	tmp = reinterpret_cast<uptr>(*ret);
	printf("pointer ret = %lx\n", tmp);
	
	*ret = malloc(32);

	printf("flag0\n");
	memcpy(*ret, ptr, 1);
	printf("flag1\n");

	//memcpy(ret, tmp, 32);
	//ret = tmp;
	//uptr * tmpret = reinterpret_cast<uptr*>(ret);
	//*tmpret = tmp;
	//syntax_error, should be detected if get compiled.
}