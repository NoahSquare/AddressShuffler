#include <stdio.h>
#include <sys/mman.h>
#include "llvm/IR/Value.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/IR/ValueMap.h"

typedef uintptr_t   uptr;

uptr * tmp;
void * baseptr;
void * ptr = malloc(32);

extern "C" void _save_mapping(void * v) {
	printf("Running _save_mapping\n");

	int * addr = (int *)v;
	printf("*v = %d // this should be 5 if value from program is taken\n", *addr);

	int a = 2;
	memcpy(ptr, &a, sizeof(int));
}

extern "C" void _load_mapping(void * ret) {
	printf("Running _load_mapping\n");
	memcpy(ret, ptr, sizeof(int));
	printf("The program outputs 2 for now if _load_mapping loads _save_mapping saved value\n");
}

extern "C" void _shuffler_init() {
	//baseptr = malloc(32);
	char * c = (char *)ptr;
	int i = 0;
	for(;i<32; i++)
		c[i] = 0;
}