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

void BitsSet(char * string, int index, int value)
 {const int i = index >> 3;
  const int r = index % 8;
  const char d = 1 << r;
  char c = string[i];

  if (value) c |= d; else c &= ~d;
  string[i] = c;
 }

void BitsInvert(char * string, int length)
 {for(int i = 0; i < length; ++i)
   {string[i] = ~string[i];
   }
 }

int BitsGet(char * string, int index)
 {const int i = index >> 3;
  const int r = index % 8;
  const char c = string[i];
  return (c >> r) & 1;
 }

#if (__INCLUDE_LEVEL__ == 0)
int main()
 {char b[8];
  memset(b, 0, sizeof(b));
  for(int i = 1; i < 64; i *= 2)
   {BitsSet(b, i, 1);
   }
  BitsSet(b, 2, 0);
  assert( BitsGet(b, 1));
  assert(!BitsGet(b, 2));
  assert( BitsGet(b, 4));
  assert( BitsGet(b, 8));

  BitsInvert(b, 8);
  assert(!BitsGet(b, 1));
  assert( BitsGet(b, 2));
  assert(!BitsGet(b, 4));
  assert(!BitsGet(b, 8));
  return 0;
 }
#endif
#endif
