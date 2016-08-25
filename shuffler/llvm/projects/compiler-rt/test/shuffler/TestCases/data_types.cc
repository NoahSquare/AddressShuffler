// RUN: %clangxx_shuffler -O0 %s -o %t
// RUN: %run %t 2>&1 | FileCheck %s

#include <stdio.h>

int main() {  
  int a = 5;
  char c = 'c';
  char * string = "string\n";
  double pi = 3.14;

  printf("a = %d\n", a);
  printf("c = %c\n", c);
  printf("string = %s\n", string);
  printf("pi = %f\n", pi);

  // CHECK: a = 5
  // CHECK: c = c
  // CHECK: string = string
  // CHECK: pi = 3.14

}
