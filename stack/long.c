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

inline static StackLong *newStackLong()                                         // Create a new stack
 {StackLong *s = calloc(sizeof(StackLong), 1);
  s->arena = 0;
  s->size  = 0;
  s->next  = 0;
  s->base  = 0;
  return s;
 }

inline static long nStackLong(StackLong *s)                                     // The number of elements on the stack
 {return s->next - s->base;
 }

inline static int isEmptyStackLong(StackLong *s)                                // Whether the stack is empty
 {return nStackLong(s) == 0;
 }

inline static long getStackLong(StackLong *s, long i, int *rc)                  // Get an element from the stack by its index
 {const long p = s->base + i;
  if (p < s->next)                                                              // Success
   {if (rc != 0) *rc = 1;
    return s->arena[i];
   }
  if (rc != 0) *rc = 0;                                                         // Failure
  return 0;
 }

inline static void pushStackLong(StackLong *s, long l)                                 // Push an element onto the stack
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

inline static StackLong *cloneStackLong(StackLong *a)                           // Clone a stack
 {StackLong *b = newStackLong();
  const long N = nStackLong(a);
  for(long i = 0; i < N; ++i) pushStackLong(b, a->arena[i]);
  return b;
 }

inline static long firstElementStackLong(StackLong *s, int *rc)                 // Return the first element of the stack
 {if (s->next <= s->base)
   {if (rc != 0) *rc = 0;
    return 0;
   }
  if (rc != 0) *rc = 1;
  return s->arena[s->base];
 }

inline static long lastElementStackLong(StackLong *s, int *rc)                  // Return the last element of the stack
 {if (s->next <= s->base)
   {if (rc != 0) *rc = 0;
    return 0;
   }
  if (rc != 0) *rc = 1;
  return s->arena[s->next-1];
 }

inline static long popStackLong(StackLong *s, int *rc)                          // Pop a value off the stack
 {if (s->next <= s->base)
   {if (rc != 0) *rc = 0;
    return 0;
   }
  if (rc != 0) *rc = 1;
  return s->arena[--s->next];
 }

inline static long shiftStackLong(StackLong *s, int *rc)                        // Shift a value off the stack
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

  StackLong *s = newStackLong();
  for(int i = 0; i < N; ++i) pushStackLong(s, i+1);
  assert(nStackLong(s) == N);

  StackLong *t = cloneStackLong(s);
  assert(nStackLong(t) == N);
  assert(lastElementStackLong(t, &rc) == N); assert(rc == 1);

  assert(getStackLong(t, 1, &rc) == 2);  assert(rc == 1);
  assert(getStackLong(t, 2, &rc) == 3);  assert(rc == 1);
  assert(getStackLong(t,22, &rc) == 0);  assert(rc == 0);

  assert(popStackLong(t, 0) == N-0);
  assert(popStackLong(t, 0) == N-1);
  assert(popStackLong(t, 0) == N-2);
  assert(nStackLong(t) == N-3);

  assert(shiftStackLong(t, 0) == 1);
  assert(shiftStackLong(t, 0) == 2);
  assert(shiftStackLong(t, 0) == 3);
  assert(nStackLong(t) == N-6);

  for(int i = 0; i < N; ++i)
   {assert(i + popStackLong(s, &rc) == N);
    assert(rc == 1);
   }
  assert(nStackLong(s) == 0);
  assert(isEmptyStackLong(s));

  assert(popStackLong(s, &rc) == 0);
  assert(rc == 0);

  assert(nStackLong(s) == 0);
 }

int main()                                                                      // Run tests and calculate from command line
 {tests();
  return 0;
 }
#endif
#endif
