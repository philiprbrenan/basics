//------------------------------------------------------------------------------
// Quick sort with two pivots
// Philip R Brenan at appaapps dot com, Appa Apps Ltd. Inc. 2023
//------------------------------------------------------------------------------
#define _GNU_SOURCE
#ifndef CquickSortLong
#define CquickSortLong
#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include <stdarg.h>
#include <assert.h>
#include <math.h>
#include <x86intrin.h>
#include "basics/basics.c"
#include "insertionSort/long.c"
#include "stack/long.c"
// 1,706,137 instructions executed

inline static long quickSortLongPartitionUp(long *Z, long s, long e)            // Partition a sub array starting at the low end and working up. Start and end of partition. Return index of pivot in Z
 {long p = s;                                                                   // Pivot point:  the pivot value is assumed to be the first element of the partition

  for(long i = s+1; i <= e; ++i)                                                // Partition around pivot
   {if (Z[i] < Z[p])                                                            // Less than pivot so move into lower partition
     {if (i == p + 1) swapLong(Z+p, Z+i);                                       // Swap adjacent elements into order
      else                                                                      // Move pivot up to make room for new element not adjacent with the pivot
       {long P = Z[p], Q = Z[p+1], A = Z[i];                                    // Both exist because i > p + 1 and t >= s
        Z[p] = A; Z[p+1] = P; Z[i] = Q;
       }
      ++p;                                                                      // Move pivot point up
     }
   }
  return p;                                                                     // Return pivot
 }

inline static void quickSortLong(long *Z, long N)                               // Quick Sort an array in place using two pivots and a stack (for speed) rather than recursion with the middle partition processed to pack mimal elements aginst the lower pivot and maximal elements against the upper pivot . Array to sort
 {StackLong *S = StackLongNew();                                                // Start of each partition to be sorted
  StackLong *E = StackLongNew();                                                // End   of each partition to be sorted
  const long L = N - 1;

  StackLongPush(S, 0); StackLongPush(E, L);                                     // Initial partition

  for(;StackLongN(S) > 0;)                                                      // Sort partitions waiting to be sorted.
   {long s = StackLongPop(S, 0), e = StackLongPop(E, 0);                        // Start of partition, end of partition
    if (s >= e || s >= L) continue;                                             // Empty or single element partition is already sorted
    if (e - s < 8)                                                              // Use Insertion Sort on small partitions of large arrays
     {insertionSortLong(Z+s, e - s+1);
      continue;
     }

    if (Z[s+0] > Z[s+1])   swapLong(Z+s+0, Z+s+1);                              // Sort the first two elements now known to exist
    if (s + 1 >= e) continue;                                                   // The array has two elements so it is now sorted
    if (s + 2 >= e) {                                                           // The array has three elements - use bubble sort to sort them as it is faster in this degenerate case
      if (Z[s+1] > Z[s+2]) swapLong(Z+s+1, Z+s+2);
      if (Z[s+0] > Z[s+1]) swapLong(Z+s+0, Z+s+1);
      continue;
     }

    long q = quickSortLongPartitionUp(Z, s+1, e);                               // Pivot points
    long p = quickSortLongPartitionUp(Z, s,   q);                               // Pivot point for first partition
    long ms = p + 1, me = q - 1;                                                // Unsorted range of middle partition
//    for(long i = ms; i < me; ++i) {                                             // Transform middle partition to that elements equal to the lower pivot are packed up against it and likewise elements equal to the upper pivot
//      if      (Z[i] == Z[p]) swapLong(Z+i, Z+(ms++));
//      else if (Z[i] == Z[q]) swapLong(Z+i, Z+(me--));
//     }

    StackLongPush(S, s);   StackLongPush(E, p-1);                               // New partitions to sort
    StackLongPush(S, ms);  StackLongPush(E, me);
    StackLongPush(S, q+1); StackLongPush(E, e);
  }
}

#if (__INCLUDE_LEVEL__ == 0)
void test1()                                                                    // Tests
 {const long N = 1;
  long A[1] = {1};

  quickSortLong(A, N);

  assert(A[0] == 1);
 }

void test2()                                                                    // Tests
 {const long N = 2;
  long A[2] = {2,1};

  quickSortLong(A, N);

  assert(A[0] == 1);
  assert(A[1] == 2);
 }

void test3()                                                                    // Tests
 {const int N = 3;
  long A[3] = {2,1,3};

  quickSortLong(A, N);
  for(int i = 0; i < N; ++i) assert(A[i] == i+1);
 }

void test4()                                                                    // Tests
 {const int N = 4;
  long A[4] = {4,2,1,3};

  quickSortLong(A, N);
  for(int i = 0; i < N; ++i) assert(A[i] == i+1);
 }

void test10()                                                                   // Tests
 {const long N = 10;
  long A[10] = {9,1,2,3,7,5,6,4,8,0};

  quickSortLong(A, N);

  for(long i = 0; i < N; i++) assert(A[i] == i);                                // Check that the resulting A has the expected values
  for(long i = 1; i < N; i++) assert(A[i] >= A[i-1]);                           // Check that the resulting A is in ascending order
 }

void test1a()                                                                   // Tests
 {const int N = 12;
  long *A = malloc(sizeof(long) * N);
  for(long i = 0; i < N; i++) A[i] = (i * i) % N;                               // Load A in a somewhat random manner

  quickSortLong(A, N);
//for(int i = 0; i < N; ++i) say("%2ld  %2ld", i, A[i]);

  for(long i = 1; i < N; i++) assert(A[i] >= A[i-1]);
  free(A);
  exit(1);
 }

void test1k()                                                                   // Tests
 {const long N = 5*1024;
  long *A = malloc(sizeof(long) * N);
  for(long i = 0; i < N; i++) A[i] = (i * i) % N;                               // Load A in a somewhat random manner

  quickSortLong(A, N);

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
