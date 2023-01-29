//------------------------------------------------------------------------------
// Stack of long integers
// Philip R Brenan at appaapps dot com, Appa Apps Ltd. Inc. 2023
//------------------------------------------------------------------------------
#define _GNU_SOURCE
#ifndef CStackLong
#define CStackLong
#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include <assert.h>
#include <stdarg.h>
#include "basics/basics.c"

typedef struct StackLong                                                        // A stack of longs
 {long *arena;                                                                  // Array of elements
  long size;                                                                    // Number of elements allowed in arena
  long next;                                                                    // Next element to use
  long base;                                                                    // Base of stack
 } StackLong;

inline static StackLong *StackLongNew()                                         // Create a new stack
 {StackLong *s = calloc(sizeof(StackLong), 1);
  s->arena = 0;
  s->size  = 0;
  s->next  = 0;
  s->base  = 0;
  return s;
 }

inline static long StackLongN(StackLong *s)                                     // The number of elements on the stack
 {return s->next - s->base;
 }

inline static int StackLongIsEmpty(StackLong *s)                                // Whether the stack is empty
 {return StackLongN(s) == 0;
 }

inline static long StackLongGet(StackLong *s, long i, int *rc)                  // Get an element from the stack by its index
 {const long p = s->base + i;
  if (p < s->next)                                                              // Success
   {if (rc != 0) *rc = 1;
    return s->arena[i];
   }
  if (rc != 0) *rc = 0;                                                         // Failure
  return 0;
 }

inline static void StackLongPush(StackLong *s, long l)                                 // Push an element onto the stack
 {if (s->size == 0)
   {s->size  = 16;
    s->arena = calloc(s->size, sizeof(long));
    s->next  = 0;
   }
  if (s->next >= s->size - 1)
   {long ns = s->size * 2, n = s->next;
    s->size  = ns;
    long *oa = s->arena, *na = calloc(ns, sizeof(long));
    s->arena = na;
    for(long i = 0; i < n; ++i) na[i] = oa[i];
    free(oa);
   }
  s->arena[s->next++] = l;
 }

inline static StackLong *StackLongClone(StackLong *a)                           // Clone a stack
 {StackLong *b = StackLongNew();
  const long N = StackLongN(a);
  for(long i = 0; i < N; ++i) StackLongPush(b, a->arena[i]);
  return b;
 }

inline static long StackLongFirstElement(StackLong *s, int *rc)                 // Return the first element of the stack
 {if (s->next <= s->base)
   {if (rc != 0) *rc = 0;
    return 0;
   }
  if (rc != 0) *rc = 1;
  return s->arena[s->base];
 }

inline static long StackLongLastElement(StackLong *s, int *rc)                  // Return the last element of the stack
 {if (s->next <= s->base)
   {if (rc != 0) *rc = 0;
    return 0;
   }
  if (rc != 0) *rc = 1;
  return s->arena[s->next-1];
 }

inline static long StackLongPop(StackLong *s, int *rc)                          // Pop a value off the stack
 {if (s->next <= s->base)
   {if (rc != 0) *rc = 0;
    return 0;
   }
  if (rc != 0) *rc = 1;
  return s->arena[--s->next];
 }

inline static long StackLongShift(StackLong *s, int *rc)                        // Shift a value off the stack
 {if (s->next <= s->base)
   {if (rc != 0) *rc = 0;
    return 0;
   }
  if (rc != 0) *rc = 1;
  return s->arena[s->base++];
 }

#if (__INCLUDE_LEVEL__ == 0)
static void tests()                                                             // Tests
 {const int N = 10;
  int rc;

  StackLong *s = StackLongNew();
  for(int i = 0; i < N; ++i) StackLongPush(s, i+1);
  assert(StackLongN(s) == N);

  StackLong *t = StackLongClone(s);
  assert(StackLongN(t) == N);
  assert(StackLongLastElement(t, &rc) == N); assert(rc == 1);

  assert(StackLongGet(t, 1, &rc) == 2);  assert(rc == 1);
  assert(StackLongGet(t, 2, &rc) == 3);  assert(rc == 1);
  assert(StackLongGet(t,22, &rc) == 0);  assert(rc == 0);

  assert(StackLongPop(t, 0) == N-0);
  assert(StackLongPop(t, 0) == N-1);
  assert(StackLongPop(t, 0) == N-2);
  assert(StackLongN(t) == N-3);

  assert(StackLongShift(t, 0) == 1);
  assert(StackLongShift(t, 0) == 2);
  assert(StackLongShift(t, 0) == 3);
  assert(StackLongN(t) == N-6);

  for(int i = 0; i < N; ++i)
   {assert(i + StackLongPop(s, &rc) == N);
    assert(rc == 1);
   }
  assert(StackLongN(s) == 0);
  assert(StackLongIsEmpty(s));

  assert(StackLongPop(s, &rc) == 0);
  assert(rc == 0);

  assert(StackLongN(s) == 0);
 }

int main()                                                                      // Run tests and calculate from command line
 {tests();
  return 0;
 }
#endif
#endif
