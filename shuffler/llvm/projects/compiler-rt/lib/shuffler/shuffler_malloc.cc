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

struct map_info {
  uptr mapFrom; // unmapped addr
  uptr mapTo; // mapped addr
  uptr baseptr; // unmapped base addr
  uptr size;
  UT_hash_handle hh;
};


// For Debugging purpose
struct map_info tmp;

void add_node(uptr mapFrom, uptr mapTo, uptr baseptr, uptr size) {
  struct map_info * s;
  s = (struct map_info *)malloc(sizeof(struct map_info));
  printf("[Space Allocated for mapping table entry]\n");
  s->mapFrom = mapFrom;
  //s->baseptr = baseptr;
  s->size = size;
  memcpy(&(s->baseptr), &baseptr, sizeof(uptr));
  memcpy(&(s->mapTo), &mapTo, sizeof(uptr));
  HASH_ADD_UPTR( hashmap, mapFrom, s ); 
}


struct map_info * find_node(uptr mapFrom) {
  struct map_info * s;
  HASH_FIND_UPTR( hashmap, &mapFrom, s); 
  return s;
}

extern "C" void _save_mapping(uptr mapFrom, uptr mapTo, uptr size) {
  printf("Running _save_mapping\n");
  int i = 0;
  for(; i < size; i++) {
    printf(" >> Saving mapping 0x%08x -> 0x%08x, baseptr = 0x%08x\n", mapFrom+i, mapTo+i, mapFrom);
    add_node(mapFrom+i, mapTo+i, mapFrom, size);
  }
}

extern "C" void _load_mapping(uptr mapFrom, void * load_ptr) {
  printf("Running _load_mapping\n");
  printf(" >> Looking for Addr 0x%08x\n", mapFrom);
  struct map_info * s = find_node(mapFrom);
  if(s != NULL) {
    printf(" >> Finded mapping 0x%08x -> 0x%08x\n", mapFrom, s->mapTo);
    memcpy(load_ptr, &s->mapTo, sizeof(uptr));
  }
  else {
    memcpy(load_ptr, &mapFrom, sizeof(uptr));
  }
}

extern "C" void _update_mapping(uptr mapFrom, uptr mapTo) {
  printf("Running _update_mapping\n");
  struct map_info * tmp;
  HASH_FIND_UPTR( hashmap, &mapFrom, tmp); 

  if(tmp != NULL) {
    uptr baseptr = tmp->baseptr;
    uptr size = tmp->size;

    struct map_info * basenode;
    HASH_FIND_UPTR( hashmap, &baseptr, basenode); 
    
    printf(" ==== Updating mapping ====\n Base pointer = 0x%08x\n", baseptr);
    printf(" size = %d\n Mapped baseptr = 0x%08x\n", size, basenode->mapTo); 

    void * newbaseptr = (void *)malloc(size);
    memcpy(newbaseptr, (void *)basenode->mapTo, size);

    int i = 0;
    for(; i < size; i++){
      struct map_info * s;
      uptr tmp = baseptr+i;
      printf(" >> Looking for mapping 0x%08x\n", tmp);
      HASH_FIND_UPTR( hashmap, &tmp, s);
      printf(" >> Finded mapping 0x%08x -> 0x%08x, baseptr = 0x%08x\n", s->mapFrom, s->mapTo, s->baseptr);
      uptr mtmp = (uptr)newbaseptr+i;
      memcpy(&(s->mapTo), &mtmp, sizeof(uptr));
      printf(" >> Updated to 0x%08x -> 0x%08x\n", s->mapFrom, s->mapTo);
    }
  }
}

extern "C" void _shuffler_init() {
  // Setup lookup table if needed
}

// For Debugging purpose
extern "C" void _shuffler_print(uptr value) {
  printf(" Shuffler printing 0x%08x\n", value);
}
