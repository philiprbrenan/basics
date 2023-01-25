//------------------------------------------------------------------------------
// In place stable merge sort
// Philip R Brenan at appaapps dot com, Appa Apps Ltd. Inc. 2023
//------------------------------------------------------------------------------
// sde -mix -- ./long
// 1,058,796 instructions executed
// We only copy the upper partition into the work area and then work down because the upper partition can be smaller in size than the lower one.
// Using binary search seems to slow things down

#define _GNU_SOURCE
#ifndef MergeSortLong
#define MergeSortLong
#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include <assert.h>
#include <stdarg.h>
#include <x86intrin.h>
#include "basics/basics.c"
#include "binarySearch/long.c"

static inline void mergeSortCopyLong                                            // Copy elements from one location in memory to another - basic version
 (long * const T, long * const S, const long N)                                 // Target address, source address, number of longs
 {switch(N)
   {case 1: T[0] = S[0]; return;
    case 2: T[0] = S[0]; T[1] = S[1]; return;
    case 3: T[0] = S[0]; T[1] = S[1]; T[2] = S[2]; return;
    case 4: T[0] = S[0]; T[1] = S[1]; T[2] = S[2]; T[3] = S[3]; return;
    case 5: T[0] = S[0]; T[1] = S[1]; T[2] = S[2]; T[3] = S[3]; T[4] = S[4]; return;
    case 6: T[0] = S[0]; T[1] = S[1]; T[2] = S[2]; T[3] = S[3]; T[4] = S[4]; T[5] = S[5]; return;
    case 7: T[0] = S[0]; T[1] = S[1]; T[2] = S[2]; T[3] = S[3]; T[4] = S[4]; T[5] = S[5]; T[6] = S[6]; return;
    case 8: T[0] = S[0]; T[1] = S[1]; T[2] = S[2]; T[3] = S[3]; T[4] = S[4]; T[5] = S[5]; T[6] = S[6]; T[7] = S[7]; return;
    default:
     {long p = 0;
      const long A = 9;                                                         // Chosen by experimentation
      for (; p + A < N; p += A)
       {T[p+0] = S[p+0];
        T[p+1] = S[p+1];
        T[p+2] = S[p+2];
        T[p+3] = S[p+3];
        T[p+4] = S[p+4];
        T[p+5] = S[p+5];
        T[p+6] = S[p+6];
        T[p+7] = S[p+7];
        T[p+8] = S[p+8];
       }
      for (; p < N; p++) T[p] = S[p];
     }
   }
 }

static void mergeSortLong                                                       // In place stable merge sort
 (long * const Z, const long N)                                                 // Array to sort, size of array
 {long * const W = malloc(sizeof(long) * N);                                    // Work area
  for (long s = 1; s < N; s <<= 1)                                              // Sort at each partition size
   {const long S = s << 1;                                                      // Partition full size

    for (long p = 0; p + s < N; p += S)                                         // Zip two partitions together from the top downwards
     {long a = p+s, b = p+S < N ? s : N-a, i = a+b;                             // Position in each half partition
      if (Z[a] >= Z[a-1]) continue;                                             // The partitions are already ordered
      mergeSortCopyLong(W, Z+a, b);                                             // Copy upper partition (which might be smaller than the lower partition) to work area so that we can merge back into main array from bottom.

      for (;a > p && b > 0;)                                                    // Choose next highest element from each partition
       {Z[--i] = Z[a-1] > W[b-1] ? Z[--a] : W[--b];                             // Stability: we take the highest element first or the first equal element
       }

      mergeSortCopyLong(Z+p, W, b);                                             // Add trailing elements
     }
   }
  free(W);
 }

#if (__INCLUDE_LEVEL__ == 0)
void test1()                                                                    // Tests
 {const int N = 1;
  long array[1] = {1};

  mergeSortLong(array, N);

  assert(array[0] == 1);
 }

void test2()                                                                    // Tests
 {const int N = 2;
  long array[2] = {2,1};

  mergeSortLong(array, N);

  assert(array[0] == 1);
  assert(array[1] == 2);
 }

void test3()                                                                    // Tests
 {const int N = 3;
  long array[3] = {2,1,3};

  mergeSortLong(array, N);
  for(int i = 0; i < N; ++i) assert(array[i] == i+1);
 }

void test4()                                                                    // Tests
 {const int N = 4;
  long array[4] = {4,2,1,3};

  mergeSortLong(array, N);
  for(int i = 0; i < N; ++i) assert(array[i] == i+1);
 }

void test5()                                                                    // Tests
 {const int N = 5;
  long array[5] = {4,2,5,1,3};

  mergeSortLong(array, N);
  for(int i = 0; i < N; ++i) assert(array[i] == i+1);
 }

void test6()                                                                    // Tests
 {const int N = 6;
  long array[6] = {4,2,5,1,6,3};

  mergeSortLong(array, N);
  for(int i = 0; i < N; ++i) assert(array[i] == i+1);
 }

void test7()                                                                    // Tests
 {const int N = 7;
  long array[7] = {4,2,6,1,3,5,7};

  mergeSortLong(array, N);
  for(int i = 0; i < N; ++i) assert(array[i] == i+1);
 }

void test8()                                                                    // Tests
 {const int N = 8;
  long array[8] = {8,4,2,6,1,3,7,5};

  mergeSortLong(array, N);
  for(int i = 0; i < N; ++i) assert(array[i] == i+1);
 }

void test9()                                                                    // Tests
 {const int N = 9;
  long array[9] = {8,4,2,6,9,1,3,7,5};

  mergeSortLong(array, N);
  for(int i = 0; i < N; ++i) assert(array[i] == i+1);
 }

void test10()                                                                   // Tests
 {const int N = 10;
  long array[10] = {9,1,2,3,7,5,6,4,8,0};

  mergeSortLong(array, N);

  for(int i = 0; i < N; i++) assert(array[i] == i);                             // Check that the resulting array has the expected values
  for(int i = 1; i < N; i++) assert(array[i] >= array[i-1]);                    // Check that the resulting array is in ascending order
 }

void test1a()                                                                   // Tests
 {const int N = 6;
  long *A = malloc(sizeof(long) * N);
  for(int i = 0; i < N; i++) A[i] = (i * i) % N;                                // Load array in a somewhat random manner

  mergeSortLong(A, N);

  for(int i = 1; i < N; i++) assert(A[i] >= A[i-1]);
  free(A);
  exit(1);
 }

void test1k()                                                                   // Tests
 {const int N = 5*1024;
  long *A = malloc(sizeof(long) * N);
  for(int i = 0; i < N; i++) A[i] = (i * i) % N;                                // Load array in a somewhat random manner

  mergeSortLong(A, N);

  for(int i = 1; i < N; i++) assert(A[i] >= A[i-1]);                            // Check that the resulting array is in ascending order
  free(A);
 }

void tests()                                                                    // Tests
 {test1();
  test2();
  test3();
  test4();
  test5();
  test6();
  test7();
  test8();
  test9();
  test10();
  test1k();
 }

int main()                                                                      // Run tests
 {//test9(); exit(0);
  //test1a();
  tests();
  return 0;
 }
#endif
#endif
