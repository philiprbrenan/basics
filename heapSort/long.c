//------------------------------------------------------------------------------
// Heap sort
// Philip R Brenan at appaapps dot com, Appa Apps Ltd. Inc. 2023
//------------------------------------------------------------------------------
#define _GNU_SOURCE
#ifndef CheapSortLong
#define CheapSortLong
#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include <stdarg.h>
#include <assert.h>
#include <math.h>
#include <x86intrin.h>
#include "basics/basics.c"
//1,305,391 instructions executed

static inline void heapSortLongPrint(long * const Z, const long N)              // Add an entry to the heap
 {say("Print Heap N=%4ld", N);

  for(long i = 0; i < N; ++i) say("%4ld  %4ld", i, Z[i]);
 }

static inline void heapSortLongCheck(long * const Z, const long N)              // Check that the heap is correctly constricted
 {heapSortLongPrint(Z, N);
  for(long i = 0; i < N; ++i)
   {long p = N - i - 1, q = (p-1) / 2;
    if (Z[p] < Z[q])
     {say("Failed at i=%2ld p=%2ld q=%2ld", i, p, q);
      exit(1);
     }
   }
 }

static inline void heapSortLongAdd(long * const Z, const long N)                // Add an entry to the heap
 {for(long q = N, p = 0; q != 0; q = p)                                         // Move last entry into position
   {p = q % 2 == 0 ? q / 2 - 1 : (q + 1) / 2 - 1;                               // Locate parent of last entry
    if (Z[p] > Z[q]) return;                                                    // Heap is is now in order
    swapLong(Z+p, Z+q);                                                         // Move up through heap
   }
 }

static inline void heapSortLongMoveMax(long * const Z, const long N)            // Extract the minimum element in the heap
 {const long r = Z[0];
  long p = 0;

  for(; 2 * p + 2 < N;)                                                         // Move entries up
   {const long a = 2 * p + 1, m = a + (Z[a] > Z[a+1] ? 0 : 1);
    Z[p] = Z[m];
    p = m;
   }

  if (p < N)
   {Z[p] = Z[N-1];
    for(long q = p; q != 0; q = p)                                              // Move last entry into position
     {p = q % 2 == 0 ? q / 2 - 1 : (q + 1) / 2 - 1;                             // Locate parent of last entry
      if (Z[p] > Z[q]) break;                                                   // Heap is is now in order
      swapLong(Z+p, Z+q);                                                       // Move up through heap
     }
   }

  Z[N-1] = r;                                                                   // Insert maximum element into position
 }

static inline void heapSortLong(long *Z, long N)                                // Sort an array of long integers
 {if (N <= 1) return;
  for(long i = 1; i < N; i++) heapSortLongAdd(Z, i);                            // Load heap
  for(long i = N; i > 0; i--) heapSortLongMoveMax(Z, i);                        // Unload heap
 }

#if (__INCLUDE_LEVEL__ == 0)
void test1()                                                                    // Tests
 {const long N = 1;
  long A[1] = {1};

  heapSortLong(A, N);

  assert(A[0] == 1);
 }

void test2()                                                                    // Tests
 {const long N = 2;
  long A[2] = {2,1};

  heapSortLong(A, N);

  assert(A[0] == 1);
  assert(A[1] == 2);
 }

void test3()                                                                    // Tests
 {const int N = 3;
  long A[3] = {2,1,3};

  heapSortLong(A, N);
  for(int i = 0; i < N; ++i) assert(A[i] == i+1);
 }

void test4()                                                                    // Tests
 {const int N = 4;
  long A[4] = {4,2,1,3};

  heapSortLong(A, N);
  for(int i = 0; i < N; ++i) assert(A[i] == i+1);
 }

void test10()                                                                   // Tests
 {const long N = 10;
  long A[10] = {9,1,2,3,7,5,6,4,8,0};

  heapSortLong(A, N);

  for(long i = 0; i < N; i++) assert(A[i] == i);                                // Check that the resulting A has the expected values
  for(long i = 1; i < N; i++) assert(A[i] >= A[i-1]);                           // Check that the resulting A is in ascending order
 }

void test1a()                                                                   // Tests
 {const int N = 64;
  long *A = malloc(sizeof(long) * N);
  for(long i = 0; i < N; i++) A[i] = (i * i) % N;                               // Load A in a somewhat random manner

  heapSortLong(A, N);

  for(long i = 1; i < N; i++) assert(A[i] >= A[i-1]);
  free(A);
  exit(1);
 }

void test1k()                                                                   // Tests
 {const long N = 5*1024;
  long *A = malloc(sizeof(long) * N);
  for(long i = 0; i < N; i++) A[i] = (i * i) % N;                               // Load A in a somewhat random manner

   heapSortLong(A, N);

  for(long i = 1; i < N; i++) assert(A[i] >= A[i-1]);                           // Check that the resulting A is in ascending order
  free(A);
 }

void tests()                                                                    // Tests
 {test1();
  test2();
  test3();
  test4();
  test10();
  test1k();
 }

int main()                                                                      // Run tests
 {//test1a();
  tests();
  return 0;
 }
#endif
#endif
