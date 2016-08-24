// RUN: %clangxx_shuffler -O0 %s -o %t && not %run %t 2>&1 | FileCheck %s

#include <stdio.h>

int a;
char c;

void foo() {
  a++;
  c = 'd';
}

int main() {  
  a = 5;
  c = 'c';

  printf("a = %d\n", a);
  printf("c = %c\n", c);
  
  foo();

  printf("a++ = %d\n", a);
  printf("c++ = %c\n", c);

  // CHECK: {a = 5}
  // CHECK: {c = c}
  // CHECK: {a++ = 6}
  // CHECK: {c++ = d}

}
