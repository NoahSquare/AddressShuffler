// RUN: %clangxx_shuffler -O0 %s -o %t && not %run %t 2>&1 | FileCheck %s

#include <stdio.h>
int main() {
  int a = 5;
  int b = 10;
  int c = 'c';
  printf("a = %d\n", a);
  printf("a = %d\n", a);
  printf("c = %c\n", c);
  printf("b = %d\n", b);
  a++;
  printf("a++ = %d\n", a);
}
