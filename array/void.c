//------------------------------------------------------------------------------
// Arrays of void pointers
// Philip R Brenan at appaapps dot com, Appa Apps Ltd. Inc. 2023
//------------------------------------------------------------------------------
// sde -mix -- ./long
// 153,006 instructions executed
// The arrays are assumed to be large enough to accommodate these operations.

#define _GNU_SOURCE
#ifndef ArrayVoid
#define ArrayVoid
#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include <assert.h>
#include <stdarg.h>
#include <x86intrin.h>
#include "basics/basics.c"

static void ArrayVoidPush                                                       // Push onto an array
 (void **a, long l, void *b)
 {a[l] = b;
 }

static void ArrayVoidPushArray                                                  // Push an array onto an array
 (void **A, long a, void **B, long b)
 {for(long i = 0; i < b; ++i)
   {A[a+i] = B[i];
   }
 }

static void *ArrayVoidPop                                                       // Pop from an array
 (void **a, long l)
 {return a[l-1];
 }

static void *ArrayVoidShift                                                     // Shift from an array
 (void **a, long l)
 {void * const b = a[0];
  memmove(a, a+1, (l-1)*sizeof(long));
  return b;
 }

static void ArrayVoidUnShift                                                    // Unshift onto an array
 (void **a, long l, void *b)
 {memmove(a+1, a, (l-1)*sizeof(long));
  a[0] = b;
 }

static void ArrayVoidInsert                                                     // Insert into an array
 (void **a, long l, void *b, long i)                                            // Array, length of array, item to insert, index to insert at
 {memmove(a+i+1, a+i, (l-i-1)*sizeof(long));
  a[i] = b;
 }

static void *ArrayVoidDelete                                                    // Delete from an array returning the element deleted
 (void **A, long l, long i)                                                     // Array, length of array, index to delete at
 {void * const a = A[i];
  memmove(A+i, A+i+1, (l-i-1)*sizeof(long));
  return a;
 }

#if (__INCLUDE_LEVEL__ == 0)

void tests()                                                                    // Tests
 {long N = 10;
  void *A[N];
  long  B[N];
  for(long i = 0; i < N; ++i) {A[i] = B+i; B[i] = i;}

  assert(*(long *)ArrayVoidDelete(A, N, 4) == 4);
  assert(*(long *)A[3] == 3);
  assert(*(long *)A[4] == 5);

  long c1 = 44;
  ArrayVoidInsert(A, N, &c1, 4);
  assert(*(long *)A[3] == 3); assert(*(long *)A[4] == c1); assert(*(long *)A[5] == 5);

  long c99 = 99;
  assert(*(long *)ArrayVoidPop (A, N)   ==  9);
                  ArrayVoidPush(A, N-1,  &c99);
  assert(*(long *)ArrayVoidPop (A, N)   == c99);

  assert(*(long *)ArrayVoidShift(A, N) == 0);
  assert(*(long *)ArrayVoidShift(A, N) == 1);

  long c2 = 11, c3 = 22;
  ArrayVoidUnShift(A, N, &c3);
  ArrayVoidUnShift(A, N, &c2);
  assert(*(long *)A[0] == c2);
  assert(*(long *)A[1] == c3);


  ArrayVoidPushArray(A, 2, A, 2);
  assert(*(long *)A[0] == c2); assert(*(long *)A[1] == c3);
  assert(*(long *)A[2] == c2); assert(*(long *)A[3] == c3);

  assert(*(long *)ArrayVoidDelete(A, N, 3) == c3);
  assert(*(long *)A[3] == 44);
//  for(long i = 0; i < N; ++i) say("%2d %ld", i, *(long *)A[i]);

 }

int main()                                                                      // Run tests
 {tests();
  return 0;
 }
#endif
#endif
