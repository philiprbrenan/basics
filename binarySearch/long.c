//------------------------------------------------------------------------------
// Binary search of an array of longs
// Philip R Brenan at appaapps dot com, Appa Apps Ltd. Inc. 2023
//------------------------------------------------------------------------------
// sde -mix -- ./long
// 421,559 instructions executed
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

static inline long binarySearchLong                                             // Search a sorted array for the index of a specified long or return -1.
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

#if (__INCLUDE_LEVEL__ == 0)
void test1()                                                                    // Tests
 {long array[1] = {1}, N = 1;

  assert(binarySearchLong(array, N, 0) == -1);
  assert(binarySearchLong(array, N, 1) ==  0);
  assert(binarySearchLong(array, N, 2) == -1);
 }

void test2()                                                                    // Tests
 {long array[2] = {1, 3}, N = 2;

  assert(binarySearchLong(array, N, 0) == -1);
  assert(binarySearchLong(array, N, 1) ==  0);
  assert(binarySearchLong(array, N, 2) == -1);
  assert(binarySearchLong(array, N, 3) ==  1);
  assert(binarySearchLong(array, N, 4) == -1);
 }

void test3()                                                                    // Tests
 {long array[3] = {1, 3, 5}, N = 3;

  assert(binarySearchLong(array, N, 0) == -1);
  assert(binarySearchLong(array, N, 1) ==  0);
  assert(binarySearchLong(array, N, 2) == -1);
  assert(binarySearchLong(array, N, 3) ==  1);
  assert(binarySearchLong(array, N, 4) == -1);
  assert(binarySearchLong(array, N, 5) ==  2);
  assert(binarySearchLong(array, N, 6) == -1);
 }

void test4()                                                                    // Tests
 {long array[4] = {1, 3, 5, 7}, N = 4;

  assert(binarySearchLong(array, N, 0) == -1);
  assert(binarySearchLong(array, N, 1) ==  0);
  assert(binarySearchLong(array, N, 2) == -1);
  assert(binarySearchLong(array, N, 3) ==  1);
  assert(binarySearchLong(array, N, 4) == -1);
  assert(binarySearchLong(array, N, 5) ==  2);
  assert(binarySearchLong(array, N, 6) == -1);
 }

void test5()                                                                    // Tests
 {long array[10] = {1, 3, 5, 7, 9, 11, 13, 15, 17, 19}, N = 10;
  assert(binarySearchLong(array, N, 11) ==  5);
  assert(binarySearchLong(array, N, 12) == -1);
  assert(binarySearchLong(array, N, 19) ==  9);
  assert(binarySearchLong(array, N, 20) == -1);
 }

void test1k()                                                                   // Tests
 {const long N = 1024;
  long Z[N];
  for(int i =  0; i < N; ++i) Z[i] = 2 * i + 1;
  int c = 0;
  for(int i = -1; i < N+N+1; ++i)
   {const long f = binarySearchLong(Z, N, i);
    assert(f != -1 ? Z[f] == i : 1);
    if (f > -1) ++c;
   }
  assert(c == N);
 }

void tests()                                                                    // Tests
 {test1();
  test2();
  test3();
  test4();
  test5();
  test1k();
 }

int main()                                                                      // Run tests
 {tests();
  return 0;
 }
#endif
#endif
