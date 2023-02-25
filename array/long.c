//------------------------------------------------------------------------------
// Arrays of longs
// Philip R Brenan at appaapps dot com, Appa Apps Ltd. Inc. 2023
//------------------------------------------------------------------------------
// sde -mix -- ./long
#ifndef ArrayLong

#define ArrayLong
#define ArrayDataType long                                                      /* Array data type */
#define Array(name) ArrayLong##name                                             /* Function name */

#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include <assert.h>
#include <stdarg.h>
#include <x86intrin.h>
#include "basics/basics.c"
#include "array/generic.c"
#endif
