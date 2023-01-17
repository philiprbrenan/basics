//------------------------------------------------------------------------------
// Bits in C
// Philip R Brenan at appaapps dot com, Appa Apps Ltd. Inc., 2022
//------------------------------------------------------------------------------
#define _GNU_SOURCE
#ifndef Cbits
#define Cbits
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <stdarg.h>
#include <x86intrin.h>
#include "basics/basics.c"

static void printZ8(__m512i z)
 {for(int i = 0; i < 8; ++i)
   {say("%2d  %8ld", i, z[i]);
   }
 }

static inline __m512i loadLowerValues(long *i)                                  // Given an array of 16 integers, partitioned into 8 pairs and load the lower half of each pair into a z
 {__m512i a = _mm512_loadu_si512(i);
  __m512i b = _mm512_loadu_si512(i+8);
  __m512i l = _mm512_maskz_compress_epi64 (0b1010101, b);                       // Upper values from first z
          l = _mm512_inserti64x4 (l, *(__m256i *)&l, 1);
          l = _mm512_mask_compress_epi64 (l, 0b1010101, a);                     // Lower values from first z

  return  l;
 }

static inline __m512i loadUpperValues(long *i)                                  // Given an array of 16 integers, partitioned into 8 pairs and load the upper half of each pair into a z
 {__m512i a = _mm512_loadu_si512(i);
  __m512i b = _mm512_loadu_si512(i+8);
  __m512i l = _mm512_maskz_compress_epi64 (0b10101010, b);                      // Upper values from first z
          l = _mm512_inserti64x4 (l, *(__m256i *)&l, 1);
          l = _mm512_mask_compress_epi64 (l, 0b10101010, a);                    // Lower values from first z

  return  l;
 }

static inline void storeLowerAndUpperValues(__m512i l, __m512i u, long *i)   // Interleave the lower and upper values into memory
 {__m512i a = _mm512_maskz_expand_epi64 (0b01010101, l);                     // Interleave lower half
  __m512i b = _mm512_maskz_expand_epi64 (0b10101010, u);
  __m512i c = _mm512_mask_mov_epi64     (a, 0b10101010, b);
              _mm512_storeu_si512       (i, c);

  __m512i L = _mm512_maskz_compress_epi64 (0b11110000, l);                      // Interleave upper half
  __m512i U = _mm512_maskz_compress_epi64 (0b11110000, u);

  __m512i A = _mm512_maskz_expand_epi64 (0b01010101, L);
  __m512i B = _mm512_maskz_expand_epi64 (0b10101010, U);
  __m512i C = _mm512_mask_mov_epi64     (A, 0b10101010, B);
              _mm512_storeu_si512       (i+8, C);
 }

static inline void compareAndSwapZ8(__m512i *a, __m512i *b)                     // Given an array of 16 integers, partitioned into 8 pairs, swap the lower half of each pair with the upper half if they and load the upper half of each pair into a z
 {__mmask8 k = _mm512_cmpgt_epu64_mask (*a, *b);
  *a = _mm512_mask_xor_epi64 (*a, k, *a, *b);
  *b = _mm512_mask_xor_epi64 (*b, k, *a, *b);
  *a = _mm512_mask_xor_epi64 (*a, k, *a, *b);
 }

#if (__INCLUDE_LEVEL__ == 0)
static int z8Eq(__m512i z, long l1, long l2, long l3, long l4, long l5, long l6, long l7, long l8)
 {if (z[0] != l1) return 1;
  if (z[1] != l2) return 2;
  if (z[2] != l3) return 3;
  if (z[3] != l4) return 4;

  if (z[4] != l5) return 5;
  if (z[5] != l6) return 6;
  if (z[6] != l7) return 7;
  if (z[7] != l8) return 8;
  return 0;
 }

static void test1()
 {long i[16] = {1, 2, 3, 4, 5, 6, 7, 8, 7, 6, 5, 4, 3, 2, 1, 0};
  __m512i l = loadLowerValues(i);

  assert(z8Eq(l, 1, 3, 5, 7, 7, 5, 3, 1) == 0);
 }

static void test2()
 {long i[16] = {1, 2, 3, 4, 5, 6, 7, 8, 7, 6, 5, 4, 3, 2, 1, 0};
  __m512i u = loadUpperValues(i);

  assert(z8Eq(u, 2, 4, 6, 8, 6, 4, 2, 0) == 0);
 }

static void test3()
 {long i[16] = {1, 2, 3, 4, 5, 6, 7, 8, 7, 6, 5, 4, 3, 2, 1, 0};
  __m512i l = loadLowerValues(i);
  __m512i u = loadUpperValues(i);
  compareAndSwapZ8(&l, &u);
  assert(z8Eq(l, 1, 3, 5, 7, 6, 4, 2, 0) == 0);
  assert(z8Eq(u, 2, 4, 6, 8, 7, 5, 3, 1) == 0);
 }

static void test4()
 {long i[9] = {1, 2, 3, 4, 5, 6, 7, 8, 9};
  long j[8] = {};

  __m512i z = _mm512_loadu_si512(i+1);
  _mm512_storeu_si512(j, z);
  __m512i y = _mm512_loadu_si512(j);

  assert(z8Eq(y, 2, 3, 4, 5, 6, 7, 8, 9) == 0);
 }

static void test5()
 {long A[16] = {1, 3, 5, 7, 9, 11, 13, 15, 2, 4, 6, 8, 10, 12, 14, 16};
  long B[16] = {};

  __m512i l = _mm512_loadu_si512(A);
  __m512i u = _mm512_loadu_si512(A+8);
  storeLowerAndUpperValues(l, u, B);
  for(int i = 0; i < 16; ++i) assert(B[i] == i+ 1);
 }

static void tests()
 {test1();
  test2();
  test3();
  test4();
  test5();
 }

int main()
 {tests();
  return 0;
 }
#endif
#endif
// 195134 with one add
// 195135 with two adds
