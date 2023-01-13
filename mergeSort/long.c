//------------------------------------------------------------------------------
// In place stable merge sort.
// Philip R Brenan at appaapps dot com, Appa Apps Ltd. Inc. 2023
//------------------------------------------------------------------------------
#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include <assert.h>
#include <stdarg.h>

void mergeSortLong(long *A, const int N)                                        // In place stable merge sort
 {long *W = malloc(N * sizeof(long));                                           // Work area - malloc faster than calloc

  for (int s = 1; s < N; s <<= 1)                                               // Partition half size
   {const int S = s << 1;                                                       // Partition full size

    for (int p = 0; p < N; p += S)                                              // Partition start
     {int a = p, b = a+s, i = 0;                                                // Position in each half partition

      for (;i < S && a < p+s && b < p+S && a < N && b < N && p+i < N;)          // Choose next lowest element from each partition
       {W[i++] = A[A[a] <= A[b] ? a++ : b++];                                   // Stability: we take the lowest element first or the first equal element
       }

      for (     ; a < p+s && p+i < N;)     W[i++] = A[a++];                     // Add trailing elements
      for (     ; b < p+S && p+i < N;)     W[i++] = A[b++];
      for (i = 0; i < S   && p+i < N; i++) A[p+i] = W[i];                       // Copy back from work area to array being sorted
     }
   }

  free(W);                                                                      // Free work area
 }

#if (__INCLUDE_LEVEL__ == 0)
void test1()                                                                    // Tests
 {const int N = 1;
  long array[1] = {1};

  mergeSortLong(array, N);

  assert(array[0] == 1);
 }

void test2()                                                                    // Tests
 {const int N = 2;
  long array[2] = {2,1};

  mergeSortLong(array, N);

  assert(array[0] == 1);
  assert(array[1] == 2);
 }

void test10()                                                                    // Tests
 {const int N = 10;
  long array[10] = {9,1,2,3,7,5,6,4,8,0};

  mergeSortLong(array, N);

  for(int i = 0; i < N; i++) assert(array[i] == i);                             // Check that the resulting array has the expected values
  for(int i = 1; i < N; i++) assert(array[i] >  array[i-1]);                    // Check that the resulting array is in ascending order
 }

void tests()                                                                    // Tests
 {test1();
  test2();
  test10();
 }

int main()                                                                      // Run tests
 {tests();
  return 0;
 }
#endif
// sde -mix -- ./long
// 201727
