#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#define INIT	1000
//#define VERBOSE

#define FREE	1
#define FIXED	2
#define MUTED	3

int verbose;

struct vertex {
  int  name;
  int  var;
  int  stamp;
  int  depth;
  int  parent;
  int  path;
  int  sign;
  int  degree;
  int *next;
  int *edge; };

struct edge {
  int mode;
  int parent;
  int depth;
  int min;
  int max; };

int nVrt, nEdge;
int *table, table_size, table_alloc;
int stamp, *eStamp;
int *cycle, cycle_size;

struct edge*   edges;
struct vertex* vertices;

void removeEdge (int edge, int sign) {
  if (verbose) printf ("c removing edge %i\n", edge);
  assert (edges[edge].mode != FIXED);
  edges[edge].mode = FIXED;

  int min = edges[edge].min;
  int max = edges[edge].max;

  int i, flag = 0;
  for  (i = 0; i < vertices[min].degree; i++) {
    if (vertices[min].edge[i] == edge) flag = 1;
    else { vertices[min].edge[i-flag] = vertices[min].edge[i];
           vertices[min].next[i-flag] = vertices[min].next[i]; } }
  vertices[min].degree -= flag;
  vertices[min].sign *= sign;
//  if (vertices[min].degree == 2) { }
  assert (flag);

  flag = 0;
  for  (i = 0; i < vertices[max].degree; i++) {
    if (vertices[max].edge[i] == edge) flag = 1;
    else { vertices[max].edge[i-flag] = vertices[max].edge[i];
           vertices[max].next[i-flag] = vertices[max].next[i]; } }
  vertices[max].degree -= flag;
  vertices[max].sign *= sign;
  assert (flag);
}

// print the PR clauses that assign the first vertex in the cycle to true

void printPR (int size) {
  int i, j, mask;
  for (mask = (1 << size) - 1; mask >= (1 << (size - 1)); mask--) {
    int count = 0;
    printf("%i ", cycle[0]);
    for (i = size - 2; i >= 0; i--) {
      if (mask & (1 << i)) printf ("%i ", cycle[size - i - 1]), count = 0;
      else count++; }

    printf("%i ", cycle[0]);
    for (i = size - 2; i >= 0; i--) {
      if (mask & (1 << i)) printf ("%i ",  cycle[size - i - 1]);
      else                 printf ("-%i ", cycle[size - i - 1]); }
    printf ("0\n");

    for (j = size - count + 1; j <= size; j++) {
      printf("d %i ", cycle[0]);
      for (i = size - 2; i >= 0; i--)
        if (mask & (1 << i)) printf ("%i ", cycle[size - i - 1]);
      printf("%i 0\n", cycle[j - 1]); } } }

int find_cycle (int edge, int print) {
  if (verbose) printf ("c find cycle %i %i\n", edge, edges[edge].mode);
  assert (edges[edge].mode == FREE);

  int tie = vertices[edges[edge].min].degree;
  if (tie > vertices[edges[edge].max].degree)
    tie = vertices[edges[edge].max].degree;

  int i, j, d;
  for (i = 0; i < nVrt; i++) vertices[i].depth = 0;

  vertices[edges[edge].max].path   = edge;
  vertices[edges[edge].max].depth  = 1;
  vertices[edges[edge].max].parent = edges[edge].min;

  for (d = 1; d <= nVrt; d++) {
    for (i = 1; i <= nEdge; i++) {
      if (i == edge) continue;
      if (edges[i].mode == FIXED) continue;

      if ((vertices[edges[i].min].depth == d) &&
          (vertices[edges[i].max].depth == 0)) {
        if (edges[i].max == edges[edge].min) {
          if (verbose) printf("c found cycle %i %i ", edge, i);
          cycle_size = 0;
          cycle[cycle_size++] = edge;
          cycle[cycle_size++] = i;
          int v = edges[i].min;
          while (vertices[v].path != edge) {
            cycle[cycle_size++] = vertices[v].path;
            if (verbose) printf("%i ", vertices[v].path);
            v = vertices[v].parent; }
          if (verbose) printf("0\n");
          if (print) { printPR (cycle_size); removeEdge (edge, 1); }
          return (d + 1) * 1000 + tie; }
        if (verbose) printf("c adding vertex %i (%i)\n", edges[i].max, edges[i].min);
        vertices[edges[i].max].path   = i;
        vertices[edges[i].max].parent = edges[i].min;
        vertices[edges[i].max].depth  = d + 1; }

      if ((vertices[edges[i].min].depth == 0) &&
          (vertices[edges[i].max].depth == d)) {
        if (edges[i].min == edges[edge].min) {
          if (verbose) printf("c found cycle %i %i ", edge, i);
          int v = edges[i].max;
          cycle_size = 0;
          cycle[cycle_size++] = edge;
          cycle[cycle_size++] = i;
          while (vertices[v].path != edge) {
            cycle[cycle_size++] = vertices[v].path;
            if (verbose) printf("%i ", vertices[v].path);
            v = vertices[v].parent; }
          if (verbose) printf("0\n");
          if (print) { printPR (cycle_size); removeEdge (edge, 1); }
          return (d + 1) * 1000 + tie; }
        if (verbose) printf("c adding vertex %i (%i)\n", edges[i].min, edges[i].max);
        vertices[edges[i].min].path   = i;
        vertices[edges[i].min].parent = edges[i].max;
        vertices[edges[i].min].depth  = d + 1; } } }

  return (d + 1) * 1000 + tie; }

int match (struct vertex u, struct vertex v) {
  if (u.sign   != v.sign  ) return 0;
  if (u.degree != v.degree) return 0;

  int i;
  for (i = 0; i < u.degree; i++) {
    if (u.edge[i] != v.edge[i]) return 0; }
  return 1; }

int compare (const void * a, const void * b) {
  return ( *(int*)a - *(int*)b ); }

int main (int argc, char** argv) {
  FILE *file;

  file = fopen (argv[1], "r");

  char string[1024];
  int nVar, nCls, lit;

  verbose = 0;
#ifdef VERBOSE
  verbose = 1;
#endif

  table_size  = 0;
  table_alloc = INIT;
  table = (int*) malloc (sizeof(int) * table_alloc);

  vertices = (struct vertex *) malloc (sizeof (struct vertex) * INIT);

  for (;;) {
    int tmp = fscanf (file, " p cnf %i %i ", &nVar, &nCls);
    if (tmp == 2) break;
    tmp = fscanf (file, " %s ", string); }

  nEdge = nVar;

  for (;;) {
    int tmp = fscanf (file, " %i ", &lit);
    if (tmp != 1) break;
    if (table_size == table_alloc) {
      table_alloc *= 2;
      table = (int*) realloc (table, sizeof (int) * table_alloc); }
    table[table_size++] = lit; }

  fclose (file);

  printf ("c file has %i variables and %i clauses\n", nVar, nCls);

  eStamp = (int*) malloc (sizeof(int) * (nVar + 1));
  cycle  = (int*) malloc (sizeof(int) * (nVar + 1));

  int i, j;
  edges  = (struct edge*) malloc (sizeof (struct edge) * (nVar +1));
  for (i = 1; i <= nEdge; i++) edges[i].mode = FIXED;

  nVrt = 0;
  for (i = 0; i < table_size; i++) {
    int count = 0, sign = 1;
    struct vertex v;
    for (j = i; table[j]; j++) { count++; if (table[j] < 0) sign *= -1; }
    v.sign   = sign;
    v.degree = count;
    v.edge   = (int *) malloc (sizeof (int) * count);
    v.next   = (int *) malloc (sizeof (int) * count);
    count = 0;
    for (j = i; table[j]; j++) v.edge[count++] = abs(table[j]);
    qsort (v.edge, count, sizeof(int), compare);

    i = j;
    int flag = 1;
    for (j = 0; j < nVrt; j++) if (match (vertices[j], v)) flag = 0;
    if (flag) { v.name = nVrt; vertices[nVrt++] = v; } }

  for (i = 0; i < nVrt; i++) {
    for (j = 0; j < vertices[i].degree; j++) {
      int k, l, edge = vertices[i].edge[j];
      for (k = 0; k < nVrt; k++) {
        if (k == i) continue;
        for (l = 0; l < vertices[k].degree; l++)
          if (vertices[k].edge[l] == edge)
            vertices[i].next[j] = k; } } }

  for (i = 0; i < nVrt; i++)
    for (j = 0; j < vertices[i].degree; j++) {
      int edge = vertices[i].edge[j];
      edges[edge].mode = FREE;
      if (vertices[i].name < vertices[i].next[j]) {
        edges[edge].min = vertices[i].name;
        edges[edge].max = vertices[i].next[j]; }
      else {
        edges[edge].max = vertices[i].name;
        edges[edge].min = vertices[i].next[j]; } }

  if (verbose)
    for (i = 1; i <= nVar; i++)
      printf("c edge var %i: %i-%i\n", i, edges[i].min, edges[i].max);

  for (j = 1; j <= nEdge; j++) {
    int min = nEdge * 1000;
    int fix = 0;
    for (i = 1; i <= nEdge; i++) {
      if (edges[i].mode == FIXED) continue;
      int res = find_cycle (i, 0);
      if (res < min) { min = res; fix = i; }
      if (verbose) printf("c res = %i\n", res); }
    if (fix) find_cycle (fix, 1);
  }

  if (verbose)
    for (i = 0; i < nVrt; i++) {
      printf("c node: %i; sign: %i; degree: %i :: ", vertices[i].name, vertices[i].sign, vertices[i].degree);
      for (j = 0; j < vertices[i].degree; j++) printf("%i (%i) ", vertices[i].edge[j], vertices[i].next[j]);
      printf("0\n"); }

  return 1;
}
