//------------------------------------------------------------------------------
// Binary search of an array of longs
// Philip R Brenan at appaapps dot com, Appa Apps Ltd. Inc. 2023
//------------------------------------------------------------------------------
// sde -mix -- ./long
// 421,557 instructions executed
#define _GNU_SOURCE
#ifndef BinarySearchLong
#define BinarySearchLong
#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include <assert.h>
#include <stdarg.h>
#include <x86intrin.h>
#include "basics/basics.c"

static inline long binarySearchEqLong                                           // Search a sorted array for the index of a specified long or return -1.
 (long * const Z, const long N, const long f)                                   // Array, length, value to search for
 {if (N == 0) return -1;                                                        // Cannot find any value in an empty array

  long s = 0, e = N-1;                                                          // Partition limits
  for (long i = 0; i < 99; ++i)                                                 // Search - oddly having a loop count check produces faster code with -O3
   {if (e - s <= 2)                                                             // Partition is small enoiugh to search in constant time
     {if (s+0 < N && Z[s+0] == f) return s+0;
      if (s+1 < N && Z[s+1] == f) return s+1;
      if (s+2 < N && Z[s+2] == f) return s+2;
      return -1;                                                                // Not found
     }
    long m = (s + e) / 2;                                                       // Middle point
    if (f < Z[m]) e = m; else s = m;                                            // Adjust bounds
   }

  return -1;
 }

static inline long binarySearchGtLong                                           // Search a sorted array for the index of the first element with a value large than the one specified or return -1 if no such index exists.
 (long * const Z, const long N, const long f)                                   // Array, length, value to search for
 {if (N == 0) return -1;                                                        // Cannot find any value in an empty array

  long s = 0, e = N-1;                                                          // Partition limits
  for (long i = 0; i < 99; ++i)                                                 // Search - oddly having a loop count check produces faster code with -O3
   {if (e - s <= 2)                                                             // Partition is small enoiugh to search in constant time
     {if (s+0 < N && Z[s+0] > f) return s+0;
      if (s+1 < N && Z[s+1] > f) return s+1;
      if (s+2 < N && Z[s+2] > f) return s+2;
      return -1;                                                                // Not found
     }
    long m = (s + e) / 2;                                                       // Middle point
    if (f < Z[m]) e = m; else s = m;                                            // Adjust bounds
   }

  return -1;
 }

static inline long binarySearchLtLong                                           // Search a sorted array for the index of the first element with a value less than the one specified or return -1 if no such index exists.
 (long * const Z, const long N, const long f)                                   // Array, length, value to search for
 {if (N == 0) return -1;                                                        // Cannot find any value in an empty array

  long s = 0, e = N-1;                                                          // Partition limits
  for (long i = 0; i < 99; ++i)                                                 // Search - oddly having a loop count check produces faster code with -O3
   {if (e - s <= 2)                                                             // Partition is small enough to search in constant time
     {if (s+2 < N && Z[s+2] < f) return s+2;
      if (s+1 < N && Z[s+1] < f) return s+1;
      if (s+0 < N && Z[s+0] < f) return s+0;
      return -1;                                                                // Not found
     }
    long m = (s + e) / 2;                                                       // Middle point
    if (f <= Z[m]) e = m; else s = m;                                           // Adjust bounds
   }

  return -1;
 }

#if (__INCLUDE_LEVEL__ == 0)
void test1()                                                                    // Tests
 {long array[1] = {1}, N = 1;

  assert(binarySearchEqLong(array, N, 0) == -1);
  assert(binarySearchEqLong(array, N, 1) ==  0);
  assert(binarySearchEqLong(array, N, 2) == -1);
 }

void test2()
 {long array[2] = {1, 3}, N = 2;

  assert(binarySearchEqLong(array, N, 0) == -1);
  assert(binarySearchEqLong(array, N, 1) ==  0);
  assert(binarySearchEqLong(array, N, 2) == -1);
  assert(binarySearchEqLong(array, N, 3) ==  1);
  assert(binarySearchEqLong(array, N, 4) == -1);
 }

void test3()
 {long array[3] = {1, 3, 5}, N = 3;

  assert(binarySearchEqLong(array, N, 0) == -1);
  assert(binarySearchEqLong(array, N, 1) ==  0);
  assert(binarySearchEqLong(array, N, 2) == -1);
  assert(binarySearchEqLong(array, N, 3) ==  1);
  assert(binarySearchEqLong(array, N, 4) == -1);
  assert(binarySearchEqLong(array, N, 5) ==  2);
  assert(binarySearchEqLong(array, N, 6) == -1);
 }

void test4()
 {long array[4] = {1, 3, 5, 7}, N = 4;

  assert(binarySearchEqLong(array, N, 0) == -1);
  assert(binarySearchEqLong(array, N, 1) ==  0);
  assert(binarySearchEqLong(array, N, 2) == -1);
  assert(binarySearchEqLong(array, N, 3) ==  1);
  assert(binarySearchEqLong(array, N, 4) == -1);
  assert(binarySearchEqLong(array, N, 5) ==  2);
  assert(binarySearchEqLong(array, N, 6) == -1);
 }

void test5()
 {long array[10] = {1, 3, 5, 7, 9, 11, 13, 15, 17, 19}, N = 10;
  assert(binarySearchEqLong(array, N, 11) ==  5);
  assert(binarySearchEqLong(array, N, 12) == -1);
  assert(binarySearchEqLong(array, N, 19) ==  9);
  assert(binarySearchEqLong(array, N, 20) == -1);
 }

void test1k()
 {const long N = 1024;
  long Z[N];
  for(int i =  0; i < N; ++i) Z[i] = 2 * i + 1;
  int c = 0;
  for(int i = -1; i < N+N+1; ++i)
   {const long f = binarySearchEqLong(Z, N, i);
    assert(f != -1 ? Z[f] == i : 1);
    if (f > -1) ++c;
   }
  assert(c == N);
 }

void test6()
 {const long N = 1024;
  long Z[N];
  for(int i =  0; i < N; ++i) Z[i] = i / 2;
  assert(Z[binarySearchGtLong(Z, N, 19)] == 20);
  assert(Z[binarySearchGtLong(Z, N, 20)] == 21);
 }

void test7()
 {const long N = 1024;
  long Z[N];
  for(int i =  0; i < N; ++i) Z[i] = i / 2;
  assert(Z[binarySearchLtLong(Z, N, 19)] == 18);
  assert(Z[binarySearchLtLong(Z, N, 20)] == 19);
 }

void tests()                                                                    // Tests
 {test1();
  test2();
  test3();
  test4();
  test5();
  test1k();
  test6();
  test7();
 }

int main()                                                                      // Run tests
 {tests();
  return 0;
 }
#endif
#endif
