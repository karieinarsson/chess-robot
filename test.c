#include <stdio.h>
 
/* global variable declaration */
int g = 20;
 
int main () {

  printf("%i", g);
  /* local variable declaration */
  for (int i; i < 2; i++){
    g = 10;
    printf ("value of g = %d\n",  g);
  }

  printf("%i", g);
  return 0;
}

