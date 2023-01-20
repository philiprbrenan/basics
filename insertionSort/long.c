//------------------------------------------------------------------------------
// Insertion sort
// Philip R Brenan at appaapps dot com, Appa Apps Ltd. Inc. 2023
//------------------------------------------------------------------------------
// 65,624,124 instructions executed
#define _GNU_SOURCE
#ifndef CinsertionSortLong
#define CinsertionSortLong
#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include <stdarg.h>
#include <assert.h>
#include <math.h>
#include <x86intrin.h>
#include "basics/basics.c"

void insertionSortLong(long *Z, long N)
 {for(long i = 1; i < N; ++i)
   {const long a = Z[i];
    for (long j = i-1; j >= 0; --j)
     {if (Z[j] > a) Z[j+1] = Z[j]; else break;
      Z[j] = a;
     }
   }
 }

#if (__INCLUDE_LEVEL__ == 0)
void test1()                                                                    // Tests
 {const long N = 1;
  long A[1] = {1};

  insertionSortLong(A, N);

  assert(A[0] == 1);
 }

void test2()                                                                    // Tests
 {const long N = 2;
  long A[2] = {2,1};

  insertionSortLong(A, N);

  assert(A[0] == 1);
  assert(A[1] == 2);
 }

void test3()                                                                    // Tests
 {const int N = 3;
  long A[3] = {2,1,3};

  insertionSortLong(A, N);
  for(int i = 0; i < N; ++i) assert(A[i] == i+1);
 }

void test4()                                                                    // Tests
 {const int N = 4;
  long A[4] = {4,2,1,3};

  insertionSortLong(A, N);
  for(int i = 0; i < N; ++i) assert(A[i] == i+1);
 }

void test10()                                                                   // Tests
 {const long N = 10;
  long A[10] = {9,1,2,3,7,5,6,4,8,0};

  insertionSortLong(A, N);

  for(long i = 0; i < N; i++) assert(A[i] == i);                                // Check that the resulting A has the expected values
  for(long i = 1; i < N; i++) assert(A[i] >= A[i-1]);                           // Check that the resulting A is in ascending order
 }

void test1a()                                                                   // Tests
 {const int N = 64;
  long *A = malloc(sizeof(long) * N);
  for(long i = 0; i < N; i++) A[i] = (i * i) % N;                               // Load A in a somewhat random manner

  insertionSortLong(A, N);

  for(long i = 1; i < N; i++) assert(A[i] >= A[i-1]);
  free(A);
  exit(1);
 }

void test1k()                                                                   // Tests
 {const long N = 5*1024;
  long *A = malloc(sizeof(long) * N);
  for(long i = 0; i < N; i++) A[i] = (i * i) % N;                               // Load A in a somewhat random manner

   insertionSortLong(A, N);

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
