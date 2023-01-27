//------------------------------------------------------------------------------
// Arrays of longs
// Philip R Brenan at appaapps dot com, Appa Apps Ltd. Inc. 2023
//------------------------------------------------------------------------------
// sde -mix -- ./long
// 153,006 instructions executed
// The arrays are assumed to be large enough to accommodate these operations.

#define _GNU_SOURCE
#ifndef ArrayLong
#define ArrayLong
#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include <assert.h>
#include <stdarg.h>
#include <x86intrin.h>
#include "basics/basics.c"

static void ArrayLongPushLong                                                   // Push onto an array
 (long *a, long l, long b)
 {a[l] = b;
 }

static long ArrayLongPopLong                                                    // Pop from an array
 (long *a, long l)
 {return a[l-1];
 }

static long ArrayLongShiftLong                                                  // Shift from an array
 (long *a, long l)
 {const long b = a[0];
  memmove(a, a+1, (l-1)*sizeof(long));
  return b;
 }

static void ArrayLongUnShiftLong                                                // Unshift onto an array
 (long *a, long l, long b)
 {memmove(a+1, a, (l-1)*sizeof(long));
  a[0] = b;
 }

static void ArrayLongInsertLong                                                 // Insert into an array
 (long *a, long l, long b, long i)                                              // Array, length of array, item to insert, index to insert at
 {memmove(a+i+1, a+i, (l-i-1)*sizeof(long));
  a[i] = b;
 }

static void ArrayLongDeleteLong                                                 // Delete from an array
 (long *a, long l, long i)                                                      // Array, length of array, index to delete at
 {memmove(a+i, a+i+1, (l-i-1)*sizeof(long));
 }

void ArrayLongPrint(long *a, long l)                                            // Print an array
 {for(long i = 0; i < l; ++i)
   {say("%2d  %ld", i, a[i]);
   }
 }

#if (__INCLUDE_LEVEL__ == 0)

void tests()                                                                    // Tests
 {long N = 10;
  long A[N];
  for(long i = 0; i < N; ++i) A[i] = i;
  ArrayLongDeleteLong(A, N, 4);
  assert(A[3] == 3); assert(A[4] == 5);

  ArrayLongInsertLong(A, N, 44, 4);
  assert(A[3] == 3); assert(A[4] == 44); assert(A[5] == 5);

  assert(ArrayLongPopLong (A, N)   ==  9);
         ArrayLongPushLong(A, N-1,    99);
  assert(ArrayLongPopLong (A, N)   == 99);

  assert(ArrayLongShiftLong(A, N) == 0);
  assert(ArrayLongShiftLong(A, N) == 1);

  ArrayLongUnShiftLong(A, N, 11);
  ArrayLongUnShiftLong(A, N, 0);
  assert(A[0] == 0); assert(A[1] == 11);
 }

int main()                                                                      // Run tests
 {tests();
  return 0;
 }
#endif
#endif
