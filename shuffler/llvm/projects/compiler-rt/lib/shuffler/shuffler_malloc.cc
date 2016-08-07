#include <stdio.h>
#include <sys/mman.h>
#include <string.h>
#include <inttypes.h>
#include "shuffler_map.h"
#include "llvm/IR/Value.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/IR/ValueMap.h"

typedef uintptr_t   uptr;
struct map_info * hashmap;

uptr ptr = 0;

struct map_info {
  uptr mapFrom;
  uptr mapTo;
  UT_hash_handle hh;
};

// For Debugging purpose
struct map_info tmp;

void add_node(uptr mapFrom, uptr mapTo) {
struct map_info * s;
  s = (struct map_info *)malloc(sizeof(struct map_info));
  s->mapFrom = mapFrom;
  memcpy(&(s->mapTo), &mapTo, sizeof(uptr));
  HASH_ADD_INT( hashmap, mapFrom, s ); 
}

struct map_info * find_node(uptr mapFrom) {
  struct map_info * s;
  HASH_FIND_INT( hashmap, &mapFrom, s); 
  return s;
}

extern "C" void _save_mapping(uptr mapFrom, uptr mapTo) {
  printf("Running _save_mapping\n");
  printf(" Saving mapping %x -> %x\n", mapFrom, mapTo);
  add_node(mapFrom, mapTo);
}

extern "C" void _load_mapping(uptr mapFrom, void * load_ptr) {
  printf("Running _load_mapping\n");
  printf(" Looking for Addr %x\n", mapFrom);
  struct map_info * node = find_node(mapFrom);
  printf(" Finded mapping %x -> %x\n", mapFrom, node->mapTo);
  memcpy(load_ptr, &node->mapTo, sizeof(uptr));
}

extern "C" void _update_mapping(uptr mapFrom, uptr mapTo) {
  printf("Running _update_mapping\n");
  struct map_info * s;
  HASH_FIND_INT( hashmap, &mapFrom, s); 
  printf(" updating mapping from \n  %x -> %x\n     to\n  %x -> %x\n", mapFrom, s->mapTo, mapFrom, mapTo);
  memcpy(&(s->mapTo), &mapTo, sizeof(uptr));
}

extern "C" void _shuffler_init() {
  // TODO: Setup lookup table
  hashmap = NULL;
}

extern "C" void _shuffler_print(uptr value) {
  printf(" Shuffler printing %x\n", value);
}