// RUN: %clangxx_shuffler -O0 %s -o %t && not %run %t 2>&1 | FileCheck %s

#include <stdio.h>

int main() {  
  // Test creating array
  int b[10];

  // Test loading from array
  b[1] = 1;
  b[2] = 2;

  // Test updating array
  printf("b[1] = %d\n", b[1]);
  printf("b[2] = %d\n", b[2]);

  b[1] = 11;
  b[2] = 12;
  // Test updating array multipletimes
  printf("updated1 b[1] = %d\n", b[1]);
  printf("updated1 b[2] = %d\n", b[2]);
  printf("updated2 b[1] = %d\n", b[1]);
  printf("updated2 b[2] = %d\n", b[2]);

  // Test different data types
  char c[12] = "hello world";
  printf("string = %s\n", c);

  // CHECK: {b[1] = 1}
  // CHECK: {b[2] = 2}
  // CHECK: {updated1 b[1] = 11}
  // CHECK: {updated1 b[2] = 12}
  // CHECK: {updated2 b[1] = 11}
  // CHECK: {updated2 b[2] = 12}

}
