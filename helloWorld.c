//------------------------------------------------------------------------------
// C
// Philip R Brenan at appaapps dot com, Appa Apps Ltd. Inc. 2022, 2022
//------------------------------------------------------------------------------
#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include <stdarg.h>
#include <assert.h>
#include <math.h>
#include "basics/basics.c"
#include <x86intrin.h>
void tests()                                                                    // Tests
 {assert(1);
  printf("Hello World!\n");
 }

int main(int argc, char **argv)                                                 // Main
 {tests();
  return 0;
 }
