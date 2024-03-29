//------------------------------------------------------------------------------
// Arrays: user supplied arrays are assumed to be big enough for each operation.
// Philip R Brenan at appaapps dot com, Appa Apps Ltd. Inc. 2023
//------------------------------------------------------------------------------
#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include <assert.h>
#include <stdarg.h>
#include <x86intrin.h>
#include "basics/basics.c"

static void Array(Push)                                                         // Push onto an array
 (ArrayDataType *a, long l, ArrayDataType b)
 {a[l] = b;
 }

static void Array(PushArray)                                                    // Push an array onto an array
 (ArrayDataType *A, long a, ArrayDataType *B, long b)
 {for(long i = 0; i < b; ++i)
   {A[a+i] = B[i];
   }
 }

static ArrayDataType Array(Pop)                                                 // Pop from an array
 (ArrayDataType *a, long l)
 {return a[l-1];
 }

static long Array(Shift)                                                        // Shift from an array
 (ArrayDataType *a, long l)
 {const long b = a[0];
  memmove(a, a+1, (l-1)*sizeof(ArrayDataType));
  return b;
 }

static void Array(UnShift)                                                      // Unshift onto an array
 (ArrayDataType *a, long l, ArrayDataType b)
 {memmove(a+1, a, (l-1)*sizeof(ArrayDataType));
  a[0] = b;
 }

static void Array(UnShiftArray)                                                 // Unshift an array onto an array
 (ArrayDataType *A, long a, ArrayDataType *B, long b)
 {memmove(A+b, A, a * sizeof(ArrayDataType));
  memmove(A,   B, b * sizeof(ArrayDataType));
 }

static void Array(Insert)                                                       // Insert into an array
 (ArrayDataType *a, long l, ArrayDataType b, long i)                            // Array, length of array, item to insert, index to insert at
 {if (i < l) memmove(a+i+1, a+i, (l-i-1)*sizeof(ArrayDataType));
  a[i] = b;
 }

static long Array(Delete)                                                       // Delete from an array returning the deleted element
 (ArrayDataType *A, long l, long i)                                             // Array, length of array, index to delete at
 {const long a = A[i];
  for(long j = 0; j < l-i-1; ++j) *(A+i+j) = *(A+i+1+j);
  return a;
 }

void Array(Print)(ArrayDataType *a, long l)                                     // Print an array
 {for(long i = 0; i < l; ++i)
   {say("%2d  %ld", i, (ArrayDataType)a[i]);
   }
 }

#if (__INCLUDE_LEVEL__ == 1)

void test1()                                                                    // Tests
 {long N = 10;
  ArrayDataType A[N];
  for(long i = 0; i < N; ++i) A[i] = i;
  assert(Array(Delete)(A, N, 4) == 4);
  assert(A[3] == 3); assert(A[4] == 5);

  Array(Insert)(A, N, 44, 4);
  assert(A[3] == 3); assert(A[4] == 44); assert(A[5] == 5);

  assert(Array(Pop) (A, N)     ==  9);
         Array(Push)(A, N-1,      99);
  assert(Array(Pop) (A, N)     == 99);

  assert(Array(Shift)(A, N)    == 0);
  assert(Array(Shift)(A, N)    == 1);

  Array(UnShift)(A, N, 11);
  Array(UnShift)(A, N, 22);
  assert(A[0] == 22); assert(A[1] == 11);

  Array(PushArray)(A, 2, A, 2);
  assert(A[0] == 22); assert(A[1] == 11);
  assert(A[2] == 22); assert(A[3] == 11);

  assert(Array(Delete)(A, N, 3) == 11);
  assert(A[3] == 44);
 }

void test2()
 {long N = 10;
  ArrayDataType A[N];
  for(long i = 0; i < N; ++i) A[i] = i < N>>1 ? i : 0;
  Array(UnShiftArray)(A, N>>1, A, N>>1);
  assert(A[6] == 1);
  assert(A[9] == 4);
  //Array(Print)(A, N);
 }

void test3()
 {long N = 4;
  ArrayDataType A[N];
  for(long i = 0; i < N; ++i) A[i] = i+1;
  //Array(Print)(A, N);

  Array(Insert)(A, 1, 11, 1);
  //Array(Print)(A, N);
  assert(A[0] ==  1);
  assert(A[1] == 11);
  assert(A[2] ==  3);

  Array(Insert)(A, 1, 10, 0);
  //Array(Print)(A, N);
  assert(A[0] == 10);
  assert(A[1] == 11);
  assert(A[2] ==  3);
  assert(A[3] ==  4);
 }

void test4()
 {long N = 4;
  ArrayDataType A[N];
  for(long i = 0; i < N; ++i) A[i] = i+1;
  //Array(Print) (A, N);

  Array(Insert)(A, 1, 11, 0);
  //Array(Print) (A, N);
  assert(A[0] ==  11);
  assert(A[1] ==   2);
  assert(A[2] ==   3);
 }

void test5()
 {long N = 4;
  ArrayDataType A[N];
  for(long i = 0; i < N; ++i) A[i] = i+1;
  //Array(Print) (A, N);

  Array(Insert)(A, 3, 11, 2);
  //Array(Print) (A, N);
  assert(A[0] ==   1);
  assert(A[1] ==   2);
  assert(A[2] ==  11);
  assert(A[3] ==   4);
 }

void tests()                                                                    // Tests
 {test1();
  test2();
  test3();
  test4();
  test5();
 }

int main()                                                                      // Run tests
 {tests();
  return 0;
 }
#endif
