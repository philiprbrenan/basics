//------------------------------------------------------------------------------
// Stack of long integers
// Philip R Brenan at appaapps dot com, Appa Apps Ltd. Inc. 2023
//------------------------------------------------------------------------------
#define _GNU_SOURCE
#ifndef CStackChar
#define CStackChar
#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include <assert.h>
#include <stdarg.h>
#include "basics/basics.c"

typedef struct StackChar                                                        // A stack of longs
 {char *arena;                                                                  // Array of elements
  long size;                                                                    // Number of elements allowed in arena
  long next;                                                                    // Next element to use
  long base;                                                                    // Base of stack
 } StackChar;

inline static StackChar *StackCharNew()                                         // Create a new stack
 {StackChar *s = calloc(sizeof(StackChar), 1);
  s->arena = 0;
  s->size  = 0;
  s->next  = 0;
  s->base  = 0;
  return s;
 }

inline static long StackCharN                                                   // The number of elements on the stack
 (StackChar *s)
 {return s->next - s->base;
 }

inline static long StackCharIsEmpty                                             // Whether the stack is empty
 (StackChar *s)
 {return StackCharN(s) == 0;
 }

inline static StackChar *StackCharClear                                         // Clear the stack to make it empty
 (StackChar *s)
 {s->next = 0;
  return s;
 }

inline static char StackCharGet                                                 // Get an element from the stack by its index
 (StackChar *s, long i, long *rc)
 {const long p = s->base + i;
  if (p < s->next)                                                              // Success
   {if (rc != 0) *rc = 1;
    return s->arena[i];
   }
  if (rc != 0) *rc = 0;                                                         // Failure
  return 0;
 }

inline static void StackCharPush                                                // Push an element onto the stack
 (StackChar *s, char c)
 {if (s->size == 0)
   {s->size  = 16;
    s->arena = calloc(s->size, sizeof(char));
    s->next  = 0;
   }
  if (s->next >= s->size - 1)
   {long ns = s->size * 2, n = s->next;
    s->size  = ns;
    char *oa = s->arena, *na = calloc(ns, sizeof(char));
    s->arena = na;
    for(long i = 0; i < n; ++i) na[i] = oa[i];
    free(oa);
   }
  s->arena[s->next++] = c;
 }

inline static StackChar *StackCharClone                                         // Clone a stack
 (StackChar *a)
 {StackChar *b = StackCharNew();
  const long N = StackCharN(a);
  for(long i = 0; i < N; ++i) StackCharPush(b, a->arena[i]);
  return b;
 }

inline static long StackCharFirstElement                                        // Return the first element of the stack
 (StackChar *s, long *rc)
 {if (s->next <= s->base)
   {if (rc != 0) *rc = 0;
    return 0;
   }
  if (rc != 0) *rc = 1;
  return s->arena[s->base];
 }

inline static long StackCharLastElement                                         // Return the last element of the stack
 (StackChar *s, long *rc)
 {if (s->next <= s->base)
   {if (rc != 0) *rc = 0;
    return 0;
   }
  if (rc != 0) *rc = 1;
  return s->arena[s->next-1];
 }

inline static long StackCharPop                                                 // Pop a value off the stack
 (StackChar *s, long *rc)
 {if (s->next <= s->base)
   {if (rc != 0) *rc = 0;
    return 0;
   }
  if (rc != 0) *rc = 1;
  return s->arena[--s->next];
 }

inline static long StackCharShift                                               // Shift a value off the stack
 (StackChar *s, long *rc)
 {if (s->next <= s->base)
   {if (rc != 0) *rc = 0;
    return 0;
   }
  if (rc != 0) *rc = 1;
  return s->arena[s->base++];
 }

inline static void StackCharPushString                                          // Push a string on to the stack
 (StackChar *s, char *t)                                                        // Stack, string
 {for(char *c = t; *c; ++c) StackCharPush(s, *c);
 }

inline static void StackCharErr                                                 // Write the contents of a stack of characters (a string) to stderr
 (StackChar *s)                                                                 // Stack
 {for(long i = s->base; i < s->next; ++i) fputc(s->arena[i], stderr);
 }

inline static void StackCharFree                                                //Free a stack
 (StackChar *s)                                                                 // Stack
 {free(s);
 }

static long StackCharEqText                                                     // Check the text on the stack matches the given string returning 1 if it does else 0
 (StackChar *s, char * const text)
 {return strncmp(s->arena+s->base, text, s->next-s->base) == 0;
 }

#if (__INCLUDE_LEVEL__ == 0)
static void tests()                                                             // Tests
 {const int N = 10;
  long rc;

  StackChar *s = StackCharNew();
  for(int i = 0; i < N; ++i) StackCharPush(s, i+1);
  assert(StackCharN(s) == N);

  StackChar *t = StackCharClone(s);
  assert(StackCharN(t) == N);
  assert(StackCharLastElement(t, &rc) == N); assert(rc == 1);

  assert(StackCharGet(t, 1, &rc) == 2);  assert(rc == 1);
  assert(StackCharGet(t, 2, &rc) == 3);  assert(rc == 1);
  assert(StackCharGet(t,22, &rc) == 0);  assert(rc == 0);

  assert(StackCharPop(t, 0) == N-0);
  assert(StackCharPop(t, 0) == N-1);
  assert(StackCharPop(t, 0) == N-2);
  assert(StackCharN(t) == N-3);

  assert(StackCharShift(t, 0) == 1);
  assert(StackCharShift(t, 0) == 2);
  assert(StackCharShift(t, 0) == 3);
  assert(StackCharN(t) == N-6);

  for(int i = 0; i < N; ++i)
   {assert(i + StackCharPop(s, &rc) == N);
    assert(rc == 1);
   }
  assert(StackCharN(s) == 0);
  assert(StackCharIsEmpty(s));

  assert(StackCharPop(s, &rc) == 0);
  assert(rc == 0);

  assert(StackCharN(s) == 0);

  StackCharClear(s);
  assert(StackCharIsEmpty(s));
  StackCharPushString(s, "abc");
  assert(StackCharEqText(s, "abc"));
  assert(StackCharPop(s, &rc) == 'c');
  assert(StackCharPop(s, &rc) == 'b');
  assert(StackCharPop(s, &rc) == 'a');
  StackCharFree(s);
 }

int main()                                                                      // Run tests and calculate from command line
 {tests();
  return 0;
 }
#endif
#endif
