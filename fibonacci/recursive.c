//------------------------------------------------------------------------------
// Recursive fibonacci sequence
//------------------------------------------------------------------------------
#include <stdio.h>

long f(int n)
 {return n == 0 ? 0 : n == 1 ? 1 : f(n-1) + f(n-2);
 }

int main()
 {printf("%ld\n", f(40));
  return 0;
 }

/*

# Mix output version 10
# Intel(R) SDE version: 8.63.0 external
# Starting tid 0,  OS-TID 500482
# EMIT_GLOBAL_DYNAMIC_STATS   EMIT# 1
#
# $global-dynamic-counts

*total                                                        6599247142

*/
