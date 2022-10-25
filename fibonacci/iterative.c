//------------------------------------------------------------------------------
// Iterative fibonacci sequence
//------------------------------------------------------------------------------
#include <stdio.h>

long f(int n)
 {long a = 1, b = 0;
  for(int i = 0; i < n; ++i)
   {long c = a + b;
    b = a;
    a = c;
   }
  return b;
 }

int main()
 {printf("%ld\n", f(40));
  return 0;
 }

/*

# Intel(R) SDE version: 8.63.0 external
# Starting tid 0,  OS-TID 500482
# EMIT_GLOBAL_DYNAMIC_STATS   EMIT# 1
#
# $global-dynamic-counts//
*total                                                            199838

*/
