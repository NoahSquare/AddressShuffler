#include <stdio.h>
#include <sys/mman.h>
#include "llvm/IR/Value.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/IR/ValueMap.h"

typedef uintptr_t   uptr;

uptr ptr = 0;

struct map_info {
	uptr mapFrom;
	uptr mapTo;
	int size;
};

// For Debugging purpose
struct map_info tmp;

extern "C" void _save_mapping(uptr mapFrom, uptr mapTo, int size) {
	printf("Running _save_mapping\n");
	printf(" mapping from %x\n", mapFrom);
	printf(" mapping to %x\n", mapTo);
	printf(" mapping size %x\n", size);

	// TODO: Save mapping info to lookup table
	memcpy(&tmp.mapFrom, &mapFrom, sizeof(uptr));
	memcpy(&tmp.mapTo, &mapTo, sizeof(uptr));
	memcpy(&tmp.size, &size, sizeof(size));
}

extern "C" void _load_mapping(uptr mapFrom, void * load_ptr) {
	printf("Running _load_mapping\n");
	printf(" looking up for mapFrom %x\n", mapFrom);
	// TODO: Look up for mapFrom
	memcpy(load_ptr, &tmp.mapTo, 4/*size*/);
}

extern "C" void _shuffler_init() {
	// TODO: Setup lookup table
}