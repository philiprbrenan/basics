//------------------------------------------------------------------------------
// In place stable merge sort.
// Philip R Brenan at appaapps dot com, Appa Apps Ltd. Inc. 2023
//------------------------------------------------------------------------------
#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include <assert.h>
#include <stdarg.h>

int mergeSortCompareLong(long *array, int A, int B)                             // Compare two integer elements
 {long a = array[A], b = array[B];
  return a < b ? -1 : a > b ? +1 : 0;
 }

void mergeSortLong(long *array, int N)                                          // In place stable merge sort
 {int *copy = calloc(N, sizeof(int));                                           // Work area
  for (int s = 1; s < N; s *= 2)                                                // Partition half size
   {const int S = s+s;                                                          // Partition size

    for (int p = 0; p < N; p += S)                                              // Partition start
     {int a = 0, b = 0, i = 0;                                                  // Position in each half partition
      for (; p+i < N && i < S && a < s && p+a < N && b < s && p+s+b < N;)       // Choose next lowest element from each partition
       {copy[i++] = mergeSortCompareLong(array, p+a, p+s+b) <= 0 ?              // Stability: we take the lowest first or the first equal element
                    array[p+a++] : array[p+s+b++];
       }

      for(;a < s && p+i < N;) copy[i++] = array[p+a++];                         // Add trailing elements
      for(;b < s && p+i < N;) copy[i++] = array[p+s+b++];

      for(int j = 0; j < S && p+j < N; j++) array[p+j] = copy[j];               // Copy back from work area to array being sorted
     }
   }
  free(copy);                                                                   // Free work area
 }

#if (__INCLUDE_LEVEL__ == 0)
void tests()                                                                    // Tests
 {const int N = 10;
  long array[10] = {9,1,2,3,7,5,6,4,8,0};

  mergeSortLong(array, N);

  for(int i = 0; i < N; i++) assert(array[i] == i);
 }

int main()                                                                      // Run tests
 {tests();
  return 0;
 }
#endif
