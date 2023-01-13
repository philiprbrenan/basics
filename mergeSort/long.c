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

  for (int p = 1; p < N; p += 2)                                                // Sorting the first set of partitions is easy
   {if (A[p]   > A[p-1]) continue;
        A[p-1] = A[p-1] ^ A[p];                                                 // Swap with xor as it is a little faster
        A[p]   = A[p-1] ^ A[p];
        A[p-1] = A[p-1] ^ A[p];
   }

  for (int s = 2; s < N; s <<= 1)                                               // Partition half size
   {const int S = s << 1;                                                       // Partition full size

    for (int p = 0; p < N; p += S)                                              // Partition start
     {int a = p, b = a+s, i = 0;                                                // Position in each half partition

      if (A[b] >= A[b-1]) continue;                                             // The partitions are already ordered

      const int aa = p+s, bb = p+S < N ? p+S : N;
      for (;a < aa && b < bb;) W[i++] = A[A[a] <= A[b] ? a++ : b++];            // Choose the lowest element first or the first equal element to obtain a stable sort

      const int ma = p+s - a;
      if (ma) memcpy(W+i, A+a, ma * sizeof(long));                              // Rest of first partition
      else
       {const int bpS = p+S - b, piN = N - (p + i), mb = bpS < piN ? bpS : piN;
        memcpy(W+i, A+b, mb * sizeof(long));                                    // Rest of second partition
       }

      memcpy(A+p, W, (S < N-p ? S : N-p) * sizeof(long));                       // Copy back from work area to array being sorted
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
  for(int i = 1; i < N; i++) assert(array[i] >= array[i-1]);                    // Check that the resulting array is in ascending order
 }

void test1k()                                                                   // Tests
 {const int N = 1024;
  long array[N];
  for(int i = 0; i < N; i++) array[i] = (i * i) % N;                            // Load array in a somewhat random manner

  mergeSortLong(array, N);

  for(int i = 1; i < N; i++) assert(array[i] >= array[i-1]);                    // Check that the resulting array is in ascending order
 }

void tests()                                                                    // Tests
 {test1();
  test2();
  test10();
  test1k();
 }

int main()                                                                      // Run tests
 {tests();
  return 0;
 }
#endif
// sde -mix -- ./long
// 365085
