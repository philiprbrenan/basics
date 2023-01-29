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

static void ArrayLongPush                                                       // Push onto an array
 (long *a, long l, long b)
 {a[l] = b;
 }

static void ArrayLongPushArray                                                  // Push an array onto an array
 (long *A, long a, long *B, long b)
 {for(long i = 0; i < b; ++i)
   {A[a+i] = B[i];
   }
 }

static long ArrayLongPop                                                        // Pop from an array
 (long *a, long l)
 {return a[l-1];
 }

static long ArrayLongShift                                                      // Shift from an array
 (long *a, long l)
 {const long b = a[0];
  memmove(a, a+1, (l-1)*sizeof(long));
  return b;
 }

static void ArrayLongUnShift                                                    // Unshift onto an array
 (long *a, long l, long b)
 {memmove(a+1, a, (l-1)*sizeof(long));
  a[0] = b;
 }

static void ArrayLongUnShiftArray                                               // Unshift an array onto an array
 (long *A, long a, long *B, long b)
 {memmove(A+b, A, a * sizeof(long));
  memmove(A,   B, b * sizeof(long));
 }

static void ArrayLongInsert                                                     // Insert into an array
 (long *a, long l, long b, long i)                                              // Array, length of array, item to insert, index to insert at
 {memmove(a+i+1, a+i, (l-i-1)*sizeof(long));
  a[i] = b;
 }

static long ArrayLongDelete                                                     // Delete from an array returning the deleted element
 (long *A, long l, long i)                                                      // Array, length of array, index to delete at
 {const long a = A[i];
  memmove(A+i, A+i+1, (l-i-1)*sizeof(long));
  return a;
 }

void ArrayLongPrint(long *a, long l)                                            // Print an array
 {for(long i = 0; i < l; ++i)
   {say("%2d  %ld", i, a[i]);
   }
 }

#if (__INCLUDE_LEVEL__ == 0)

void test1()                                                                    // Tests
 {long N = 10;
  long A[N];
  for(long i = 0; i < N; ++i) A[i] = i;
  assert(ArrayLongDelete(A, N, 4) == 4);
  assert(A[3] == 3); assert(A[4] == 5);

  ArrayLongInsert(A, N, 44, 4);
  assert(A[3] == 3); assert(A[4] == 44); assert(A[5] == 5);

  assert(ArrayLongPop (A, N)     ==  9);
         ArrayLongPush(A, N-1,      99);
  assert(ArrayLongPop (A, N)     == 99);

  assert(ArrayLongShift(A, N)    == 0);
  assert(ArrayLongShift(A, N)    == 1);

  ArrayLongUnShift(A, N, 11);
  ArrayLongUnShift(A, N, 22);
  assert(A[0] == 22); assert(A[1] == 11);

  ArrayLongPushArray(A, 2, A, 2);
  assert(A[0] == 22); assert(A[1] == 11);
  assert(A[2] == 22); assert(A[3] == 11);

  assert(ArrayLongDelete(A, N, 3) == 11);
  assert(A[3] == 44);
 }

void test2()                                                                    // Tests
 {long N = 10;
  long A[N];
  for(long i = 0; i < N; ++i) A[i] = i < N>>1 ? i : 0;
  ArrayLongUnShiftArray(A, N>>1, A, N>>1);
  assert(A[6] == 1);
  assert(A[9] == 4);
  ArrayLongPrint(A, N);
 }

void tests()                                                                    // Tests
 {test1();
  test2();
 }

int main()                                                                      // Run tests
 {tests();
  return 0;
 }
#endif
#endif
