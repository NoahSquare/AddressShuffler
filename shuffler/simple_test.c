#include <stdio.h>
int main() {
  int a[2];
  a[0] = 0;
  a[1] = 5;
  printf("a[0] = %d\n", a[0]);
  a[0] = 2;
  printf("a[0] = %d\n", a[0]);
  printf("a[1] = %d\n", a[1]);
  int * a;
  int b = 5;
  a = &b;
  printf("a = %d\n", *a);
}
