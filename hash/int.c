//------------------------------------------------------------------------------
// Hash of integers in C
// Philip R Brenan at appaapps dot com, Appa Apps Ltd. Inc., 2022
//------------------------------------------------------------------------------
#ifndef CHashInt
#define CHashInt
#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include <stdarg.h>
#include <assert.h>

typedef struct HashInt
 {int size, used;
  int *keys, *values;
  int zeroKeySet, zeroValue;
 } HashInt;

void hashIntPut(HashInt *h, int key, int value);

HashInt *hashIntNew()                                                           // Create a new hash
 {HashInt *h = calloc(1, sizeof(HashInt));
  const int  size = h->size = 8;
  const int     s = size * sizeof(int);
  int *area = calloc(2, s);
  h->keys   = area;
  h->values = area + s;
  return h;
 }

void hashIntIncrease(HashInt *h)                                                // Increase the size of a hash
 {const int oldSize = h->size, newSize = oldSize * 2,
    os = oldSize * sizeof(int), ns = newSize * sizeof(int);
  int k[oldSize]; memcpy(k, h->keys,   os);                                     // Backup the keys on the stack
  int v[oldSize]; memcpy(v, h->values, os);                                     // Backup the values on the stack
  int *newArea = calloc(2,       ns);
  h->keys   = newArea;
  h->values = newArea + ns;
  h->size   = newSize;
  for(int i = 0; i < oldSize; ++i)
   {if (k[i])
     {h->used--;
      hashIntPut(h, k[i], v[i]);
     }
   }
 }

void hashIntPut(HashInt *h, int key, int value)                                 // Put a key,value pair into the hash
 {if (key == 0)
   {h->zeroKeySet = 1;
    h->zeroValue  = value;
    return;
   }
  if (h->used*2 > h->size) hashIntIncrease(h);
  const int size = h->size, p = key % size;
  int * const keys = h->keys, *values = h->values;
  for(int i = 0; i < size; ++i)
   {const int j = (p + i) % size;
    if (keys[j] == 0)
     {keys  [j] = key;
      values[j] = value;
      ++h->used;
      return;
     }
   }
 }
                                                                                // Get a value from a hash returning true if it was found
int hashIntGet(HashInt *h, int key, int *value)
 {if (key == 0)
   {if (!h->zeroKeySet) return 0;
    *value = h->zeroValue;
    return 1;
   }
  const int  size = h->size, p = key % size;
  const int *keys = h->keys, *values = h->values;
  for(int i = 0; i < size; ++i)
   {const int j = (p + i) % size;
    if (keys[j] == 0) return 0;
    if (keys[j] == key)
     {*value = values[j];
      return 1;
     }
   }
  return 0;
 }

void hashIntDump(HashInt *h)                                                    // Dump a hash
 {if (h->zeroKeySet == 0) fprintf(stderr, "Zero %d   %8d\n",
    h->zeroKeySet, h->zeroValue);
  const int  size = h->size;
  const int *keys = h->keys, *values = h->values;
  for(int i = 0; i < size; ++i)
   {fprintf(stderr, "%4d %d   %8d\n", i, keys[i], values[i]);
   }
 }

#if (__INCLUDE_LEVEL__ == 0)
int main()
 {HashInt *h = hashIntNew();
  for(size_t i = 1; i < 20; ++i)
   {hashIntPut(h, i, 2*i);
   }
  hashIntDump(h);
  int value;
  if (hashIntGet(h, 1, &value)) assert(value == 2); else assert(0);
  if (hashIntGet(h, 2, &value)) assert(value == 4); else assert(0);
  if (hashIntGet(h, 0, &value)) assert(0);          else assert(1);
 }
#endif
#endif
