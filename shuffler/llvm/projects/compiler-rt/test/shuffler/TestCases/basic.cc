// RUN: %clangxx_shuffler -O0 %s -o %t
// RUN: %run %t 2>&1 | FileCheck %s

#include <stdio.h>
int main() {
  int a = 5;
  printf("a = %d\n", a);
  // CHECK: a = 5
}
