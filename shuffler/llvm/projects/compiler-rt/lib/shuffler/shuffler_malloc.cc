#include <stdio.h>
#include <sys/mman.h>
#include <string.h>
#include <inttypes.h>
#include "shuffler_map.h"
#include "llvm/IR/Value.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/IR/ValueMap.h"

typedef uintptr_t uptr;
struct map_info * hashmap;
struct array_info * array_metadata;

struct map_info {
  uptr mapFrom; // unmapped addr
  uptr mapTo; // mapped addr
  uptr isArray; // =0 if is not array; =baseptr if is array
  UT_hash_handle hh;
};

struct array_info {
  uptr baseptr;  // unmapped base ptr
  uptr mbaseptr;  // mapped level base ptr
  uptr size;  // unit_size * numOfElement
  uptr unit;  // unit_size
  UT_hash_handle hh;
};

// For Debugging purpose
struct map_info tmp;

void add_node(uptr mapFrom, uptr mapTo) {
  struct map_info * s;
  s = (struct map_info *)malloc(sizeof(struct map_info));
  printf("[Space Allocated for mapping table entry]\n");
  s->mapFrom = mapFrom;
  s->isArray = 0;
  memcpy(&(s->mapTo), &mapTo, sizeof(uptr));
  HASH_ADD_UPTR( hashmap, mapFrom, s ); 
}

void add_array_node(uptr mapFrom, uptr mapTo, uptr baseptr) {
  struct map_info * s;
  s = (struct map_info *)malloc(sizeof(struct map_info));
  printf("[Space Allocated for mapping table entry]\n");
  s->mapFrom = mapFrom;
  s->isArray = baseptr;
  memcpy(&(s->mapTo), &mapTo, sizeof(uptr));
  memcpy(&(s->isArray), &baseptr, sizeof(uptr));
  HASH_ADD_UPTR( hashmap, mapFrom, s ); 
}

struct map_info * find_node(uptr mapFrom) {
  struct map_info * s;
  HASH_FIND_UPTR( hashmap, &mapFrom, s); 
  return s;
}

extern "C" void _save_mapping(uptr mapFrom, uptr mapTo) {
  printf("Running _save_mapping\n");
  printf(" >> Saving mapping 0x%08x -> 0x%08x\n", mapFrom, mapTo);
  add_node(mapFrom, mapTo);
}

extern "C" void _load_mapping(uptr mapFrom, void * load_ptr) {
  printf("Running _load_mapping\n");
  printf(" >> Looking for Addr 0x%08x\n", mapFrom);
  struct map_info * node = find_node(mapFrom);
  printf(" >> Finded mapping 0x%08x -> 0x%08x\n", mapFrom, node->mapTo);
  //printf(" Copying to 0x%08x\n", (uptr)load_ptr);
  memcpy(load_ptr, &node->mapTo, sizeof(uptr));
}

extern "C" void _update_mapping(uptr mapFrom, uptr mapTo) {
  printf("Running _update_mapping\n");
  struct map_info * s;
  HASH_FIND_UPTR( hashmap, &mapFrom, s); 
  if(s->isArray == 0) {
    printf(" >> Updating mapping from \n  0x%08x -> 0x%08x\n     to\n  0x%08x -> 0x%08x\n", mapFrom, s->mapTo, mapFrom, mapTo);
    memcpy(&(s->mapTo), &mapTo, sizeof(uptr));
  }
  else {
    uptr baseptr = s->isArray;
    struct array_info * arr;
    HASH_FIND_UPTR( array_metadata, &baseptr, arr);
    printf(" ==== Updating array mapping ====\n Base pointer = 0x%08x\n", baseptr);
    printf(" NumOfElement = %d\n Mapped baseptr = 0x%08x\n", arr->size/arr->unit, arr->mbaseptr); 
    // update array_metadata
    void * newbaseptr = (void *)malloc(arr->size);
    memcpy(newbaseptr, (void *)arr->mbaseptr, arr->size);
    int numOfElement = arr->size/arr->unit;
    int i = 0;
    for(; i < numOfElement; i++){
      struct map_info * s;
      uptr tmp = arr->baseptr+i*arr->unit;
      HASH_FIND_UPTR( hashmap, &tmp, s);
      printf(" >> Finded array entry mapping 0x%08x -> 0x%08x\n", s->mapFrom, s->mapTo);
      uptr mtmp = (uptr)newbaseptr+i*arr->unit;
      memcpy(&(s->mapTo), &mtmp, sizeof(uptr));
      printf(" >> Updated to 0x%08x -> 0x%08x\n", s->mapFrom, s->mapTo);
    }
    arr->mbaseptr = (uptr)newbaseptr;
  }
  
}

extern "C" void _shuffler_init() {
  // Setup lookup table if needed
}

// For Debugging purpose
extern "C" void _shuffler_print(uptr value) {
  printf(" Shuffler printing 0x%08x\n", value);
}

extern "C" void _save_array(uptr baseptr, uptr mbaseptr, uptr size, uptr unit) {
  printf("Running _save_array\n");
  printf(" ==== Creating array mapping === \n Base pointer mapping = 0x%08x -> 0x%08x\n numOfElement = %d\n", baseptr, mbaseptr, size/unit);
  struct array_info * arr;
  arr = (struct array_info *)malloc(sizeof(struct array_info));
  arr->baseptr = baseptr;
  arr->mbaseptr = mbaseptr;
  arr->size = size;
  arr->unit = unit;
  HASH_ADD_UPTR( array_metadata, baseptr, arr );

  int numOfElement = size / unit;

  int i = 0;
  for(; i < numOfElement; i++){
    //printf(" Saving array node 0x%08x -> 0x%08x\n", baseptr+i*unit, mbaseptr+i*unit);
    add_array_node(baseptr+i*unit, mbaseptr+i*unit, baseptr);
  }
}