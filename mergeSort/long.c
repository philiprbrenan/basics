// Ashley to Puerta Vallarta for Easter
//------------------------------------------------------------------------------
// In place stable merge sort.
// Philip R Brenan at appaapps dot com, Appa Apps Ltd. Inc. 2023
//------------------------------------------------------------------------------
#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include <assert.h>
#include <stdarg.h>
void say(char *format, ...)                                                     // Say something
 {va_list p;
  va_start (p, format);
  int i = vfprintf(stderr, format, p);
  assert(i > 0);
  va_end(p);
  fprintf(stderr, "\n");
 }

static inline void mergeSortLongSwap(long *a, long *b)                          // Swap two numbers using xor
 {*a = *a ^ *b;                                                                 // Swap with xor as it is a little faster
  *b = *a ^ *b;
  *a = *a ^ *b;
 }

void mergeSortLong(long *A, const int N)                                        // In place stable merge sort
 {long W[N];                                                                    // Work area - how much stack space can we have?

  for (int p = 1; p < N; p += 2)                                                // Sorting the first set of partitions is easy
   {if (A[p] >= A[p-1]) continue;                                               // Already sorted
    mergeSortLongSwap(A+p-1, A+p);                                              // Swap with xor as it is a little faster
   }

  if (1)                                                                        // Sort the second set of partitions of size 4  by direct swaps
   {int p = 3;
    for (; p < N; p += 4)                                                       // In blocks of 4
     {if (A[p-1] >= A[p-2]) continue;                                           // Already sorted
      mergeSortLongSwap(A+p-1, A+p-2);                                          // Not sorted so swap
      if (A[p-3] >  A[p-2]) mergeSortLongSwap(A+p-3, A+p-2);                    // Possibly swap into lowest position
      if (A[p-1] >  A[p])   mergeSortLongSwap(A+p-1, A+p);                      // Possibly swap into highest position
      if (A[p-2] >  A[p-1]) mergeSortLongSwap(A+p-2, A+p-1);                    // Possibly swap into middle position
     }
    if (N == p)                                                                 // Block of three (smaller blocks will already be sorted
     {if (A[p-2] >  A[p-1]) mergeSortLongSwap(A+p-2, A+p-1);                    // Possibly swap into lowest position
      if (A[p-3] >  A[p-2]) mergeSortLongSwap(A+p-3, A+p-2);                    // Possibly swap into highest position
     }
   }

  for (int s = 4; s < N; s <<= 1)                                               // Partition half size for blocks of 16 or more for normal merge sort
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

void test1a()                                                                   // Tests
 {const int N = 21;
  long array[N];
  for(int i = 0; i < N; i++) array[i] = (i * i) % N;                            // Load array in a somewhat random manner

  mergeSortLong(array, N);

  for(int i = 1; i < N; i++) assert(array[i] >= array[i-1]);
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
 {//test1a();
  tests();
  return 0;
 }
#endif
// sde -mix -- ./long
// 288627 274511
