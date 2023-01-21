//------------------------------------------------------------------------------
// In place stable merge sort
// Philip R Brenan at appaapps dot com, Appa Apps Ltd. Inc. 2023
//------------------------------------------------------------------------------
// sde -mix -- ./long
// No optimizations: 959,723 instructions executed
// Optimized       : 748,973 instructions executed
#define _GNU_SOURCE
#ifndef CmergeSort
#define CmergeSort
#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include <assert.h>
#include <stdarg.h>
#include <x86intrin.h>
#include "basics/basics.c"
#include "heapSort/long.c"

static inline void mergeSortLongCopy                                            // Copy elements from one location in memory to another
 (long * const T, long * const S, const long N)                                 // Move the specified number of longs
 {long p, q;
  const long B = 9;                                                             // Chosen by experimentation
  if (N == 1) {T[0] = S[0]; return;}
  for (p = 0; p + B < N; p += B) for (q = 0; q < B; q++) T[p+q] = S[p+q];       // The inner loop gets unrolled by gcc
  for (     ; p     < N; p++)                            T[p]   = S[p];
 }

static inline void mergeSortLongBlock                                           // Sort a partition of a specified half size
 (long * const Z, long * const W, const long N, const long s)
 {const long S = s << 1;                                                        // Partition full size

  for (long p = 0; p + s < N; p += S)                                           // Zip two partitions together from the top downwards
   {long a = p+s, b = p+S < N ? s : N-a, i = a+b;                               // Position in each half partition
    if (Z[a] >= Z[a-1]) continue;                                               // The partitions are already ordered
    mergeSortLongCopy(W, Z+a, b);                                               // Copy upper partition (which might be smaller than the lower partition) to work area so that we can merge back into main array from bottom.

    const long B = 13;                                                          // Chosen by experimentation
    for (;a > p+B && b > B;)                                                    // Choose next highest element from each partition
     {for(long j = 0; j < B; ++j)
       {Z[--i] = Z[a-1] > W[b-1] ? Z[--a] : W[--b];                             // Stability: we take the highest element first or the first equal element
       }
     }

    for (;a > p && b > 0;)                                                      // Choose next highest element from each partition
     {Z[--i] = Z[a-1] > W[b-1] ? Z[--a] : W[--b];                               // Stability: we take the highest element first or the first equal element
     }

    mergeSortLongCopy(Z+p, Z+p, a-p);                                           // Add trailing elements
    mergeSortLongCopy(Z+p, W,   b);
   }
 }

static void mergeSortLong(long * const Z, const long N)                         // In place stable merge sort
 {if (1)                                                                        // Shortest implementation for reference purposes
   {long * const W = malloc(sizeof(long) * N);
    for (long s = 1; s < N; s <<= 1) mergeSortLongBlock(Z, W, N, s);            // Sort at each partition size
    free(W);
    return;
   }

  if (1)                                                                        // Sort partitions of size 4 using direct swaps
   {long p = 0;
    const long  B = 10;                                                         // Chosen by experimentation
    for   (     p = 0; p+B+B < N;   p += B+B)                                   // Sort pairs
     {for (long q = 0; q+1   < B+B; q += 2)
       {if (Z[p+q+1] >= Z[p+q]) continue;                                       // Already sorted
        swapLong(Z+p+q+1, Z+p+q);                                               // Swap with xor as it is a little faster
       }
     }
    for (            ; p+1 < N; p += 2)                                         // Sort remaining pairs
     {if (Z[p+1] >= Z[p]) continue;                                             // Already sorted
      swapLong(Z+p+1, Z+p);                                                     // Swap with xor as it is a little faster
     }
   }

  if (1)                                                                        // Sort partitions of size 4 using direct swaps
   {long p = 0;
    for (; p + 3 < N; p += 4)
     {long * const p0 = Z+p+0, * const p1 = Z+p+1,
           * const p2 = Z+p+2, * const p3 = Z+p+3;
      if (*p2 >= *p1) continue;                                                 // Already sorted
      /**/               swapLong(p1, p2);                                      // Not sorted so swap
      if (*p2 >  *p3)    swapLong(p2, p3);

      if (*p0 >  *p1)  { swapLong(p0, p1);
        if (*p1 >  *p2)  swapLong(p1, p2);
       }
     }
    if (N == p-1)                                                               // Partition with 3 elements, (smaller blocks will already be sorted)
     {if (Z[p-3] >  Z[p-2]) swapLong(Z+p-3, Z+p-2);                             // Possibly swap into lowest position
      if (Z[p-4] >  Z[p-3]) swapLong(Z+p-4, Z+p-3);                             // Possibly swap into highest position
     }
   }

  if (1)                                                                        // Sort partitions of size 8 using direct swaps
   {long p = 0;
    for (; p + 7 < N; p += 8)
     {long * const p0 = Z+p+0, * const p1 = Z+p+1,
           * const p2 = Z+p+2, * const p3 = Z+p+3,
           * const p4 = Z+p+4, * const p5 = Z+p+5,
           * const p6 = Z+p+6, * const p7 = Z+p+7;
      if     (*p4 >= *p3) continue;                                             // Already sorted
      /**/                  swapLong(p3, p4);                                   // Not sorted so swap
      if     (*p4 >  *p5) { swapLong(p4, p5);
        if   (*p5 >  *p6) { swapLong(p5, p6);
          if (*p6 >  *p7)   swapLong(p6, p7);
         }
       }

      if       (*p2 >  *p3) { swapLong(p2, p3);
        if     (*p3 >  *p4) { swapLong(p3, p4);
          if   (*p4 >  *p5) { swapLong(p4, p5);
            if (*p5 >  *p6)   swapLong(p5, p6);
           }
         }
       }

      if       (*p1 >  *p2) { swapLong(p1, p2);
        if     (*p2 >  *p3) { swapLong(p2, p3);
          if   (*p3 >  *p4) { swapLong(p3, p4);
            if (*p4 >  *p5)   swapLong(p4, p5);
           }
         }
       }

      if       (*p0 >  *p1) { swapLong(p0, p1);
        if     (*p1 >  *p2) { swapLong(p1, p2);
          if   (*p2 >  *p3) { swapLong(p2, p3);
            if (*p3 >  *p4) swapLong(p3, p4);
           }
         }
       }
     }

    if (N == p-3 || N == p-2 || N == p-1)                                       // Partition with 5-7 elements, (smaller blocks will already be sorted)
     {for  (long i = p-4; i < N; ++i)                                           // Insertion sort the remaining 1-3 elements into position
       {for(long j = 0;   j < 4; ++j)
         {long * const a = Z+i-j, * const b = a - 1;
          if (*b > *a) swapLong(b, a);  else break;
         }
       }
     }
   }

  if (N >= 8)                                                                   // Normal merge sort of partitions of 8 and beyond
   {long W[512];           mergeSortLongBlock(Z, W, N, 1<<3);
    if       (N >= 1<<4) { mergeSortLongBlock(Z, W, N, 1<<4);
      if     (N >= 1<<5) { mergeSortLongBlock(Z, W, N, 1<<5);
        if   (N >= 1<<6) { mergeSortLongBlock(Z, W, N, 1<<6);
          if (N >= 1<<7)
           {long * const W = malloc(sizeof(long) * N);
            for (long s = 1<<7; s < N; s <<= 1)
             {mergeSortLongBlock(Z, W, N, s);
             }
            free(W);
           }
         }
       }
     }
   }
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

void test10()                                                                    // Tests
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
