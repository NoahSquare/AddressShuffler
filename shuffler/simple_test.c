#include <stdio.h>
#include <stdlib.h>

int main() {
  // Test creating array
  int b[10];

  // Test loading from array
  b[1] = 1;
  b[2] = 2;

  // Test updating array
  printf(" \n\n *** simple_test.c: b[1] = %d ***\n\n\n", b[1]);
  printf(" \n\n *** simple_test.c: b[2] = %d ***\n\n\n", b[2]);

  b[1] = 11;
  b[2] = 12;
  // Test updating array multipletimes
  printf(" \n\n *** simple_test.c: b[1] = %d ***\n\n\n", b[1]);
  printf(" \n\n *** simple_test.c: b[2] = %d ***\n\n\n", b[2]);
  printf(" \n\n *** simple_test.c: b[1] = %d ***\n\n\n", b[1]);
  printf(" \n\n *** simple_test.c: b[2] = %d ***\n\n\n", b[2]);

  // Test different types of array
  char c[12] = "hello world";
  printf(" *** simple_test.c: %s ***\n", c);

}
