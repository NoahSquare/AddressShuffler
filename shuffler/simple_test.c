#include <stdio.h>
int g, m;
char ch;

void foo() {
  printf("foo():\n g = %d\n ch = %c\n", g, ch);
  g++;
}

int main() {
  ch = 'a';
  g = 1;
  printf("main():\n g = %d\n", g);
  foo();
  printf("main():\n g++ = %d\n", g);
}
 
