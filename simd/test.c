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
#include <immintrin.h>
#include <x86intrin.h>

#if (__INCLUDE_LEVEL__ == 0)
long i[16] = {1, 2, 3, 4, 5, 6, 7, 8, 7, 6, 5, 4, 3, 2, 1, 0};

int main()
 {__m512i a = _mm512_loadu_si512(i);
  __m512i b = _mm512_loadu_si512(i+8);
  __m512i l = _mm512_maskz_compress_epi64 (0b1010101, b);                       // Upper values from first z
          l = _mm512_inserti64x4 (l, *(__m256i *)&l, 1);
          l = _mm512_mask_compress_epi64 (l, 0b1010101, a);                     // Lower values from first z

  printf("%lld  %lld  %lld  %lld  %lld  %lld  %lld  %lld\n", l[0], l[1], l[2], l[3], l[4], l[5], l[6], l[7]);

  return 0;
 }
#endif
#endif
// 195134 with one add
// 195135 with two adds
