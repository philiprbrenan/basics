//------------------------------------------------------------------------------
// In place stable merge sort using AVX 512 to reduce execution count by 50% or so.
// Philip R Brenan at appaapps dot com, Appa Apps Ltd. Inc. 2023
//------------------------------------------------------------------------------
// sde -mix -- ./long
// Without vectorization: 2,619,211
// With    vectorization: 1,172,377 instructions executed
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

static inline void mergeSortLongSwap(long * const a, long * const b)            // Swap two numbers using xor as it is a little faster than using a temporary
 {*a = *a ^ *b;
  *b = *a ^ *b;
  *a = *a ^ *b;
 }

static inline void mergeSortLongMiniMax(long * const Z, const int p)            // Sort pairs using AVX512 instructions
 {__m512i a = _mm512_loadu_si512(Z+p+ 0);                                       // Pick up partition
  __m512i b = _mm512_loadu_si512(Z+p+ 8);

  __m512i A = _mm512_min_epu64 (a, b);                                          // Maximum and minimum of each pair
  __m512i B = _mm512_max_epu64 (a, b);

  for(int i = 0; i < 8; ++i) {Z[p+i+i] = A[i]; Z[p+i+i+1] = B[i];}              // Reload minimum and maximum back into partition
 }

static inline void mergeSortLongBlock                                           // Sort a partition of a specified half size
 (long * const Z, long * const W, const int N, const int s)
 {const int S = s << 1;                                                         // Partition full size

  for (int p = 0; p + s < N; p += S)                                            // Partition start
   {int a = p, b = a+s, i = 0;                                                  // Position in each half partition
    if (Z[b] >= Z[b-1]) continue;                                               // The partitions are already ordered

    const int aa = p+s, bb = p+S < N ? p+S : N;
    for (;a < aa && b < bb;) W[i++] = Z[Z[a] <= Z[b] ? a++ : b++];              // Choose the lowest element first or the first equal element to obtain a stable sort

    const int ma = p+s - a;
    if (ma) memcpy(W+i, Z+a, ma * sizeof(long));                                // Rest of first partition
    else
     {const int bpS = p+S - b, piN = N - (p+i), mb = bpS < piN ? bpS : piN;
      memcpy(W+i, Z+b, mb * sizeof(long));                                      // Rest of second partition
     }

    memcpy(Z+p, W, (S < N-p ? S : N-p) * sizeof(long));                         // Copy back from work area to array being sorted
   }
 }

static void mergeSortLong(long *Z, const int N)                                 // In place stable merge sort
 {if (0)                                                                        // Shortest implementation for reference purposes
   {long * const W = malloc(sizeof(long) * N);
    for (int s = 1; s < N; s <<= 1)                                             // Partition half size
     {const int S = s << 1;                                                     // Partition full size

      for (int p = 0; p < N; p += S)                                            // Zip two partitions together
       {int a = p, b = a+s, i = 0;                                              // Position in each half partition

        for (;i < S && a < p+s && b < p+S && a < N && b < N && p+i < N;)        // Choose next lowest element from each partition
         {W[i++] = Z[Z[a] <= Z[b] ? a++ : b++];                                 // Stability: we take the lowest element first or the first equal element
         }

        for (; a < p+s && p+i < N;) W[i++] = Z[a++];                            // Add trailing elements
        for (; b < p+S && p+i < N;) W[i++] = Z[b++];
        memcpy(Z+p, W, (S < N-p ? S : N-p) * sizeof(long));                     // Copy back from work area to array being sorted
       }
     }
    free(W);
    return;
   }

  if (1)                                                                        // Partition as pairs then sort the pairs using AVX512 instructions
   {int p = 0;
    if (N >= 16)                                                                // Whole blocks of 16 can be sorted using AVX512
     {for (p = 16; p + 16 < N; p += 16)  mergeSortLongMiniMax(Z, p);
     }
    for (; p+1 < N; p += 2)                                                     // Sort any remaining pairs
     {if               (Z[p+1] >= Z[p]) continue;                               // Already sorted
      mergeSortLongSwap(Z+p+1,    Z+p);                                         // Swap with xor as it is a little faster
     }
   }

  if (1)                                                                        // Sort partitions of size 4 using direct swaps
   {int p = 0;
    for (; p + 3 < N; p += 4)
     {long * const p0 = Z+p+0, * const p1 = Z+p+1,
           * const p2 = Z+p+2, * const p3 = Z+p+3;
      if (*p2 >= *p1) continue;                                                 // Already sorted
      /**/            mergeSortLongSwap(p1, p2);                                // Not sorted so swap
      if (*p2 >  *p3) mergeSortLongSwap(p2, p3);                                // Swap highest in first partition into position

      if (*p0 >  *p1)                                                           // Swap lowest in first partition into position
       {mergeSortLongSwap(p0, p1);
        if (*p1 >  *p2) mergeSortLongSwap(p1, p2);
       }
     }
    if (N == p-1)                                                               // Partition with 3 elements, (smaller blocks will already be sorted)
     {if (Z[p-3] >  Z[p-2]) mergeSortLongSwap(Z+p-3, Z+p-2);                    // Possibly swap into lowest position
      if (Z[p-4] >  Z[p-3]) mergeSortLongSwap(Z+p-4, Z+p-3);                    // Possibly swap into highest position
     }
   }

  if (1)                                                                        // Sort partitions of size 8 using direct swaps
   {int p = 0;
    for (; p + 7 < N; p += 8)
     {long * const p0 = Z+p+0, * const p1 = Z+p+1,
           * const p2 = Z+p+2, * const p3 = Z+p+3,
           * const p4 = Z+p+4, * const p5 = Z+p+5,
           * const p6 = Z+p+6, * const p7 = Z+p+7;
      if (*p4 >= *p3) continue;                                                 // Already sorted
      /**/            mergeSortLongSwap(p3, p4);                                // Not sorted so swap
      if (*p4 >  *p5)                                                           // Bubble element 3 up as far as possible
       {mergeSortLongSwap(p4, p5);
        if (*p5 >  *p6)
         {mergeSortLongSwap(p5, p6);
          if (*p6 >  *p7) mergeSortLongSwap(p6, p7);
         }
       }

      if (*p2 >  *p3)                                                           // Bubble element 2 up as far as possible
       {mergeSortLongSwap(p2, p3);
        if (*p3 >  *p4)
         {mergeSortLongSwap(p3, p4);
          if (*p4 >  *p5)
           {mergeSortLongSwap(p4, p5);
            if (*p5 >  *p6) mergeSortLongSwap(p5, p6);
           }
         }
       }

      if (*p1 >  *p2)                                                           // Bubble element 1 up as far as possible
       {mergeSortLongSwap(p1, p2);
        if (*p2 >  *p3)
         {mergeSortLongSwap(p2, p3);
          if (*p3 >  *p4)
           {mergeSortLongSwap(p3, p4);
            if (*p4 >  *p5) mergeSortLongSwap(p4, p5);
           }
         }
       }

      if (*p0 >  *p1)                                                           // Bubble element 0 up as far as possible
       {mergeSortLongSwap(p0, p1);
        if (*p1 >  *p2)
         {mergeSortLongSwap(p1, p2);
          if (*p2 >  *p3)
           {mergeSortLongSwap(p2, p3);
            if (*p3 >  *p4) mergeSortLongSwap(p3, p4);
           }
         }
       }
     }
    if (N == p-3 || N == p-2 || N == p-1)                                       // Partition with 5-7 elements, (smaller blocks will already be sorted)
     {for  (int i = p-4; i < N; ++i)                                            // Insertion sort the remaining 1-3 elements into position
       {for(int j = 0;   j < 4; ++j)
         {long * const a = Z+i-j, * const b = a - 1;
          if (*b > *a) mergeSortLongSwap(b, a);  else break;
         }
       }
     }
   }

  if (N >= 8)                                                                   // Normal merge sort of partitions of 8 and beyond
   {long W[256];
    mergeSortLongBlock        (Z, W, N,  8);
    if (N >= 16)
     {mergeSortLongBlock      (Z, W, N, 16);
      if (N >= 32)
       {mergeSortLongBlock    (Z, W, N, 32);
        if (N >= 64)
         {long * const W = malloc(sizeof(long) * N);
          for (int s = 64; s < N; s <<= 1)
           {mergeSortLongBlock(Z, W, N,  s);
           }
          free(W);
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

void test10()                                                                    // Tests
 {const int N = 10;
  long array[10] = {9,1,2,3,7,5,6,4,8,0};

  mergeSortLong(array, N);

  for(int i = 0; i < N; i++) assert(array[i] == i);                             // Check that the resulting array has the expected values
  for(int i = 1; i < N; i++) assert(array[i] >= array[i-1]);                    // Check that the resulting array is in ascending order
 }

void test1a()                                                                   // Tests
 {const int N = 64;
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
