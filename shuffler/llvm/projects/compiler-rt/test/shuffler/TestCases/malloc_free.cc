// RUN: %clangxx_shuffler -O0 %s -o %t && not %run %t 2>&1 | FileCheck %s

#include <stdio.h>
#include <stdlib.h>

int main() {  
  int * a = (int *)malloc(sizeof(int));
  *a = 5;
  printf("a = %d\n", *a);

}
