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

void  say (char *format, ...);
void  stop(char *format, ...);
char *ssay(char *format, ...);

void tests()                                                                    // Tests
 {assert(1);
 }

int main(int argc, char **argv)                                                 // Main
 {tests();
  return 0;
 }

void say(char *format, ...)                                                     // Say something
 {va_list p;
  va_start (p, format);
  int i = vfprintf(stderr, format, p);
  assert(i > 0);
  va_end(p);
  fprintf(stderr, "\n");
 }

char *ssay(char *format, ...)                                                   // Say something into a string
 {va_list p;
  va_start (p, format);
  char *result;
  int i = vasprintf(&result, format, p);
  assert(i > 0);
  va_end(p);
  return result;
 }

void stop(char *format, ...)
 {va_list p;
  va_start (p, format);
  int i = vfprintf(stderr, format, p);                                          // Stop after saying something
  assert(i > 0);
  va_end(p);
  fprintf(stderr, "\n");
  exit(1);
 }
