//------------------------------------------------------------------------------
// In place stable merge sort.
// Philip R Brenan at appaapps dot com, Appa Apps Ltd. Inc. 2023
//------------------------------------------------------------------------------
#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include <assert.h>
#include <stdarg.h>

void mergeSortLong(long *A, int N)                                              // In place stable merge sort
 {long *W = calloc(N, sizeof(long));                                            // Work area
  for (int s = 1; s < N; s <<= 1)                                               // Partition half size
   {const int S = s+s;                                                          // Partition size

    for (int p = 0; p < N; p += S)                                              // Partition start
     {int a = 0, b = 0, i = 0;                                                  // Position in each half partition
      for (; p+i < N && i < S && a < s && p+a < N && b < s && p+s+b < N;)       // Choose next lowest element from each partition
       {W[i++] = A[p+a] <= A[p+s+b] ? A[p+a++] : A[p+s+b++];                    // Stability: we take the lowest element first or the first equal element
       }

      for(;a < s && p+i < N;) W[i++] = A[p+a++];                                // Add trailing elements
      for(;b < s && p+i < N;) W[i++] = A[p+s+b++];

      for(i = 0; i < S && p+i < N; i++) A[p+i] = W[i];                          // Copy back from work area to array being sorted
     }
   }
  free(W);                                                                      // Free work area
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
