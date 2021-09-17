#include <stdio.h>
#include <stdlib.h>

int main (int argc, char** argv) {
  int i, j, k, size = atoi (argv[1]);

  // print blocking clauses
  for (i = 1; i <= size * 2 + 1; i++)
    for (j = 0; j < size; j++)
      for (k = j + 1; k < size; k++)
        printf("-%i -%i 0\n", i + j * (size*2+1), i + k * (size*2+1));

  for (j = 0; j < size; j++) // size
    for (i = 1; i <= (size-j) * 2 - 1; i++) {
      for (k = j + 1; k < size; k++) {
        printf("-%i %i -%i %i 0\n", i + j * (size*2+1), i + (k) * (size*2+1), i + k * (size*2+1) + 1, i + j * (size*2+1) + 1);
        printf("-%i -%i 0\n", i + j * (size*2+1), i + k * (size*2+1) + 1);
        printf("-%i %i -%i %i 0\n", i + j * (size*2+1), i + (k) * (size*2+1), i + (k) * (size*2+1) + 2, i + j * (size*2+1) + 2);
        printf("-%i -%i 0\n", i + j * (size*2+1), i + (k) * (size*2+1) + 2); }
      printf("-%i 0\n", i + j * (size*2+1)); }
}
