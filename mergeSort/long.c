//------------------------------------------------------------------------------
// In place stable merge sort.
// Philip R Brenan at appaapps dot com, Appa Apps Ltd. Inc. 2023
//------------------------------------------------------------------------------
#define _GNU_SOURCE
#ifndef CmergeSort
#define CmergeSort
#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include <assert.h>
#include <stdarg.h>
#include <x86intrin.h>

void say(char *format, ...)                                                     // Say something
 {va_list p;
  va_start (p, format);
  int i = vfprintf(stderr, format, p);
  assert(i > 0);
  va_end(p);
  fprintf(stderr, "\n");
 }

static inline __m512i mergeSortLongLoadLowerValues(long *i)                     // Given an array of 16 integers, partitioned into 8 pairs and load the lower half of each pair into a z
 {__m512i a = _mm512_loadu_si512(i);
  __m512i b = _mm512_loadu_si512(i+8);
  __m512i l = _mm512_maskz_compress_epi64 (0b1010101, b);                       // Upper values from first z
          l = _mm512_inserti64x4 (l, *(__m256i *)&l, 1);
          l = _mm512_mask_compress_epi64 (l, 0b1010101, a);                     // Lower values from first z

  return  l;
 }

static inline __m512i mergeSortLongLoadUpperValues(long *i)                     // Given an array of 16 integers, partitioned into 8 pairs and load the upper half of each pair into a z
 {__m512i a = _mm512_loadu_si512(i);
  __m512i b = _mm512_loadu_si512(i+8);
  __m512i l = _mm512_maskz_compress_epi64 (0b10101010, b);                      // Upper values from first z
          l = _mm512_inserti64x4 (l, *(__m256i *)&l, 1);
          l = _mm512_mask_compress_epi64 (l, 0b10101010, a);                    // Lower values from first z

  return  l;
 }

static inline void mergeSortLongCompareAndSwapZ8(__m512i *a, __m512i *b)        // Given an array of 16 integers, partitioned into 8 pairs, swap the lower half of each pair with the upper half if they and load the upper half of each pair into a z
 {__mmask8 k = _mm512_cmpgt_epu64_mask (*a, *b);
  *a = _mm512_mask_xor_epi64 (*a, k, *a, *b);
  *b = _mm512_mask_xor_epi64 (*b, k, *a, *b);
  *a = _mm512_mask_xor_epi64 (*a, k, *a, *b);
 }

static inline void mergeSortLongSwap(long *a, long *b)                          // Swap two numbers using xor
 {*a = *a ^ *b;                                                                 // Swap with xor as it is a little faster
  *b = *a ^ *b;
  *a = *a ^ *b;
 }

static void printZ8(__m512i z)
 {for(int i = 0; i < 8; ++i)
   {say("%2d  %8ld", i, z[i]);
   }
 }

static void mergeSortLong(long *A, const int N)                                 // In place stable merge sort
 {long W[N];                                                                    // Work area - how much stack space can we have?


  if (1)                                                                        // Sort the first 8 sets of pairs using AVX512
   {int p;
for(int i = 0; i < N; ++i) say("AAAA %2d  %2d", i, A[i]);
    for (p = 15; p < N; p += 16)                                                // Sort the first 8 sets of pairs using AVX512
     {__m512i l = mergeSortLongLoadLowerValues(A+p-15);
      __m512i u = mergeSortLongLoadUpperValues(A+p-7);
printZ8(l);
      mergeSortLongCompareAndSwapZ8(&l, &u);
      _mm512_storeu_si512(A+p-15, l);
      _mm512_storeu_si512(A+p-7,  u);
     }
for(int i = 0; i < N; ++i) say("BBBB %2d  %2d", i, A[i]);

    for (; p < N; p += 2)                                                       // Sort any remaining pairs
     {if (A[p] >= A[p-1]) continue;                                             // Already sorted
      mergeSortLongSwap(A+p-1, A+p);                                            // Swap with xor as it is a little faster
     }
   }

  if (1)                                                                        // Sort the second set of partitions of size 4  by direct swaps
   {int p = 3;
    for (; p < N; p += 4)                                                       // In blocks of 4
     {if (A[p-1] >= A[p-2]) continue;                                           // Already sorted
      mergeSortLongSwap(A+p-1, A+p-2);                                          // Not sorted so swap
      if (A[p-3] >  A[p-2]) mergeSortLongSwap(A+p-3, A+p-2);                    // Possibly swap into lowest position
      if (A[p-1] >  A[p])   mergeSortLongSwap(A+p-1, A+p);                      // Possibly swap into highest position
      if (A[p-2] >  A[p-1]) mergeSortLongSwap(A+p-2, A+p-1);                    // Possibly swap into middle position
     }
    if (N == p)                                                                 // Block of three (smaller blocks will already be sorted
     {if (A[p-2] >  A[p-1]) mergeSortLongSwap(A+p-2, A+p-1);                    // Possibly swap into lowest position
      if (A[p-3] >  A[p-2]) mergeSortLongSwap(A+p-3, A+p-2);                    // Possibly swap into highest position
     }
   }

  for (int s = 4; s < N; s <<= 1)                                               // Partition half size for blocks of 8 or more for normal merge sort
   {const int S = s << 1;                                                       // Partition full size

    for (int p = 0; p < N; p += S)                                              // Partition start
     {int a = p, b = a+s, i = 0;                                                // Position in each half partition

      if (A[b] >= A[b-1]) continue;                                             // The partitions are already ordered

      const int aa = p+s, bb = p+S < N ? p+S : N;
      for (;a < aa && b < bb;) W[i++] = A[A[a] <= A[b] ? a++ : b++];            // Choose the lowest element first or the first equal element to obtain a stable sort

      const int ma = p+s - a;
      if (ma) memcpy(W+i, A+a, ma * sizeof(long));                              // Rest of first partition
      else
       {const int bpS = p+S - b, piN = N - (p + i), mb = bpS < piN ? bpS : piN;
        memcpy(W+i, A+b, mb * sizeof(long));                                    // Rest of second partition
       }

      memcpy(A+p, W, (S < N-p ? S : N-p) * sizeof(long));                       // Copy back from work area to array being sorted
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
 {const int N = 21;
  long array[N];
  for(int i = 0; i < N; i++) array[i] = (i * i) % N;                            // Load array in a somewhat random manner

  mergeSortLong(array, N);

  for(int i = 1; i < N; i++) assert(array[i] >= array[i-1]);
 }

void test1k()                                                                   // Tests
 {const int N = 1024;
  long array[N];
  for(int i = 0; i < N; i++) array[i] = (i * i) % N;                            // Load array in a somewhat random manner

  mergeSortLong(array, N);

  for(int i = 1; i < N; i++) assert(array[i] >= array[i-1]);                    // Check that the resulting array is in ascending order
 }

void tests()                                                                    // Tests
 {test1();
  test2();
  test10();
  test1k();
 }

int main()                                                                      // Run tests
 {test1a();
  tests();
  return 0;
 }
#endif
#endif
// sde -mix -- ./long
// 274557
