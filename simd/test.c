//------------------------------------------------------------------------------
// Bits in C
// Philip R Brenan at appaapps dot com, Appa Apps Ltd. Inc., 2022
//------------------------------------------------------------------------------
#ifndef Cbits
#define Cbits
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <stdarg.h>
#include <immintrin.h>
#include <x86intrin.h>

void say(char *format, ...)                                                     // Say something
 {va_list p;
  va_start (p, format);
  int i = vfprintf(stderr, format, p);
  assert(i > 0);
  va_end(p);
  fprintf(stderr, "\n");
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
// 0         2         1
// 1         4         3
// 2         6         5
// 3         8         7
// 4         7         6
// 5         5         4
// 6         3         2
// 7         1         0

static inline void compareAndSwapZ8(__m512i a, __m512i b)                            // Given an array of 16 integers, partitioned into 8 pairs, swap the lower half of weach pair with the upper half if they  and load the upper half of each pair into a z
 {__mmask8 k = _mm512_cmpgt_epu64_mask (a, b);
  a = _mm512_mask_xor_epi64 (a, k, a, b);
  b = _mm512_mask_xor_epi64 (b, k, a, b);
  a = _mm512_mask_xor_epi64 (a, k, a, b);

  say("%d", k);
  for(int i = 0; i < 8; ++i)
   {say("%2d  %8ld  %8ld", i, a[i], b[i]);
   }
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
  compareAndSwapZ8(l, u);
 }

static void tests()
 {test1();
  test2();
  test3();
 }

int main()
 {tests();
  return 0;
 }
#endif
#endif
// 195134 with one add
// 195135 with two adds
