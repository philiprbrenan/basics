//------------------------------------------------------------------------------
// In place stable merge sort using AVX 512 to reduce execution count.
// Philip R Brenan at appaapps dot com, Appa Apps Ltd. Inc. 2023
//------------------------------------------------------------------------------
// sde -mix -- ./long
// Without vectorization: 766,929,373
// With    vectorization: 373,045,138 instructions executed
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

static inline void mergeSortLongSwap(long *a, long *b)                          // Swap two numbers using xor
 {*a = *a ^ *b;                                                                 // Swap with xor as it is a little faster
  *b = *a ^ *b;
  *a = *a ^ *b;
 }

static void mergeSortLong(long *Z, const int N)                                 // In place stable merge sort
 {if (0)                                                                        // Shortest implementation for reference purposes
   {long *W = malloc(sizeof(long) * N);
    for (int s = 1; s < N; s <<= 1)                                             // Partition half size
     {const int S = s << 1;                                                     // Partition full size

      for (int p = 0; p < N; p += S)                                            // Zip two partitions together
       {int a = p, b = a+s, i = 0;                                              // Position in each half partition

        for (;i < S && a < p+s && b < p+S && a < N && b < N && p+i < N;)        // Choose next lowest element from each partition
         {W[i++] = Z[Z[a] <= Z[b] ? a++ : b++];                                 // Stability: we take the lowest element first or the first equal element
         }

        for (     ; a < p+s && p+i < N;)     W[i++] = Z[a++];                   // Add trailing elements
        for (     ; b < p+S && p+i < N;)     W[i++] = Z[b++];
        for (i = 0; i < S   && p+i < N; i++) Z[p+i] = W[i];                     // Copy back from work area to array being sorted
       }
     }
    free(W);
    return;
   }

  if (1)                                                                        // Partition as pairs then sort the pairs using AVX512 instructions
   {int p;
    for (p = 0; p + 31 < N; p += 32)
     {__m512i a = _mm512_loadu_si512(Z+p+ 0);                                   // Pick up partition
      __m512i b = _mm512_loadu_si512(Z+p+ 8);
      __m512i c = _mm512_loadu_si512(Z+p+16);
      __m512i d = _mm512_loadu_si512(Z+p+24);

      __m512i A = _mm512_min_epu64 (a, b);                                      // Maximum and minimum of each pair
      __m512i B = _mm512_max_epu64 (a, b);
      __m512i C = _mm512_min_epu64 (c, d);
      __m512i D = _mm512_max_epu64 (c, d);

      Z[p+ 0] = A[0]; Z[p+ 1] = B[0];                                           // Reload back into partition
      Z[p+ 2] = A[1]; Z[p+ 3] = B[1];
      Z[p+ 4] = A[2]; Z[p+ 5] = B[2];
      Z[p+ 6] = A[3]; Z[p+ 7] = B[3];
      Z[p+ 8] = A[4]; Z[p+ 9] = B[4];
      Z[p+10] = A[5]; Z[p+11] = B[5];
      Z[p+12] = A[6]; Z[p+13] = B[6];
      Z[p+14] = A[7]; Z[p+15] = B[7];

      Z[p+16] = C[0]; Z[p+17] = D[0];                                           // Unroll the loop
      Z[p+18] = C[1]; Z[p+19] = D[1];
      Z[p+20] = C[2]; Z[p+21] = D[2];
      Z[p+22] = C[3]; Z[p+23] = D[3];
      Z[p+24] = C[4]; Z[p+25] = D[4];
      Z[p+26] = C[5]; Z[p+27] = D[5];
      Z[p+28] = C[6]; Z[p+29] = D[6];
      Z[p+30] = C[7]; Z[p+31] = D[7];
     }

    for (; p+1 < N; p += 2)                                                     // Sort any remaining pairs
     {if               (Z[p+1] >= Z[p]) continue;                               // Already sorted
      mergeSortLongSwap(Z+p+1,    Z+p);                                         // Swap with xor as it is a little faster
     }
   }

  if (1)                                                                        // Sort partitions of size 4 using direct swaps
   {int p = 3;
    for (; p < N; p += 4)                                                       // In blocks of 4
     {if (Z[p-1] >= Z[p-2]) continue;                                           // Already sorted
      /**/                  mergeSortLongSwap(Z+p-1, Z+p-2);                    // Not sorted so swap
      if (Z[p-3] >  Z[p-2]) mergeSortLongSwap(Z+p-3, Z+p-2);                    // Possibly swap into lowest position
      if (Z[p-1] >  Z[p-0]) mergeSortLongSwap(Z+p-1, Z+p-0);                    // Possibly swap into highest position
      if (Z[p-2] >  Z[p-1]) mergeSortLongSwap(Z+p-2, Z+p-1);                    // Possibly swap into middle position
     }
    if (N == p)                                                                 // Block of three (smaller blocks will already be sorted)
     {if (Z[p-2] >  Z[p-1]) mergeSortLongSwap(Z+p-2, Z+p-1);                    // Possibly swap into lowest position
      if (Z[p-3] >  Z[p-2]) mergeSortLongSwap(Z+p-3, Z+p-2);                    // Possibly swap into highest position
     }
   }

  if (1)                                                                        // Normal merge sort starting with half partitions of size 4
   {long *W = malloc(sizeof(long) * N);

    if (1)
     {const int s = 4, S = 8;                                                   // Partition full size

      for (int p = 0; p + s < N; p += S)                                        // Partition start
       {int a = p, b = a+s, i = 0;                                              // Position in each half partition
        if (Z[b] >= Z[b-1]) continue;                                           // The partitions are already ordered

        const int aa = p+s, bb = p+S < N ? p+S : N;
        for (;a < aa && b < bb;) W[i++] = Z[Z[a] <= Z[b] ? a++ : b++];          // Choose the lowest element first or the first equal element to obtain a stable sort

        const int ma = p+s - a;
        if (ma) memcpy(W+i, Z+a, ma * sizeof(long));                            // Rest of first partition
        else
         {const int bpS = p+S - b, piN = N - (p+i), mb = bpS < piN ? bpS : piN;
          memcpy(W+i, Z+b, mb * sizeof(long));                                  // Rest of second partition
         }

        memcpy(Z+p, W, (S < N-p ? S : N-p) * sizeof(long));                     // Copy back from work area to array being sorted
       }
     }

    for (int s = 8; s < N; s <<= 1)                                             // Partition sizes
     {const int S = s << 1;                                                     // Partition full size

      for (int p = 0; p + s < N; p += S)                                        // Partition start
       {int a = p, b = a+s, i = 0;                                              // Position in each half partition
        if (Z[b] >= Z[b-1]) continue;                                           // The partitions are already ordered

        const int aa = p+s, bb = p+S < N ? p+S : N;
        for (;a < aa && b < bb;) W[i++] = Z[Z[a] <= Z[b] ? a++ : b++];          // Choose the lowest element first or the first equal element to obtain a stable sort

        const int ma = p+s - a;
        if (ma) memcpy(W+i, Z+a, ma * sizeof(long));                            // Rest of first partition
        else
         {const int bpS = p+S - b, piN = N - (p+i), mb = bpS < piN ? bpS : piN;
          memcpy(W+i, Z+b, mb * sizeof(long));                                  // Rest of second partition
         }

        memcpy(Z+p, W, (S < N-p ? S : N-p) * sizeof(long));                     // Copy back from work area to array being sorted
       }
     }
    free(W);
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
 {const int N = 4*1025;
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
