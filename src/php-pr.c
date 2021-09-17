#include <stdio.h>
#include <stdlib.h>

int main (int argc, char** argv) {
  int i, j, k, size = atoi (argv[1]);

  for (i = size; i >= 2; i--)
    for (j = 0; j < i; j++) {
      for (k = 1; k < i; k++) {
        printf("-%i -%i ", i + j*size, i*size + k);
        printf("-%i -%i %i %i 0\n", i + j*size, i*size + k, k + j*size, i*(size+1)); }
      printf("-%i 0\n", i + j*size); }
}
