//------------------------------------------------------------------------------
// Basic routines in C
// Philip R Brenan at appaapps dot com, Appa Apps Ltd. Inc., 2022
//------------------------------------------------------------------------------
#define _GNU_SOURCE
#ifndef Basics
#define Basics
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <assert.h>
#include <memory.h>
#include <string.h>
#include <stdarg.h>

static void say(char *format, ...)                                              // Say something
 {va_list p;
  va_start (p, format);
  int i = vfprintf(stderr, format, p);
  assert(i > 0);
	va_end(p);
  fprintf(stderr, "\n");
 }

static char *ssay(char *format, ...)                                            // Say something into a string
 {va_list p;
  va_start (p, format);
  char *result;
  int i = vasprintf(&result, format, p);
  assert(i > 0);
  va_end(p);
  return result;
 }

static void stop(char *format, ...)                                             // Stop after saying something
 {va_list p;
  va_start (p, format);
  int i = vfprintf(stderr, format, p);
  assert(i > 0);
  va_end(p);
  fprintf(stderr, "\n");
  exit(1);
 }

static void out(char *format, ...)                                              // Say something to stdout
 {va_list p;
  va_start (p, format);
  int i = vfprintf(stdout, format, p);
  assert(i > 0);
	va_end(p);
  fprintf(stdout, "\n");
 }

static int getNextInt()                                                         // Get the next integer from stdin
 {int i = 0;
  const int j = scanf("%d", &i);
  assert(j > 0);
  return i;
 }

static char *getNextLine()                                                      // Get the next line of input from the current position up to and including the next new line
 {char *s = 0;
  size_t n = 0;
  const int j = getline(&s, &n, stdin);
  assert(j != 0);
  return s;
 }

static inline int max(const int i, const int j)                                 // Maximum of two integers
 {return i > j ? i : j;
 }

static inline int min(const int i, const int j)                                 // Minimum ot two integers
 {return i < j ? i : j;
 }

static void printNL(const FILE *f)
 {fputc('\n', (FILE *)f);
 }

static void printChars(const FILE *f, const char * a, const int A)
 {for(int i = 0; i < A; ++i)
   {fputc(a[i], (FILE *)f);
   }
 }

static void printHex(const FILE *f, const char * a, const int A)
 {for(int i = 0; i < A; ++i)
   {fprintf((FILE *)f, "%02x", a[i]);
   }
  fprintf((FILE *)f, "\n");
 }

static char **split(char *String, char *separator)                                     // Split a string into words
 {int        l = strlen(String), b = (l+1)*(1 + sizeof(char *));
  void *     m = malloc(b);                                                     // Space for words
  char **words = m;
  char *string = m + (l+1)*sizeof(char *);
  memset(m, 0, b);
  memcpy(string, String, l);
  int w = 0;
  for(char *p = strtok(string, separator); p; p = strtok(0, separator))
   {words[w++] = p;
   }
  return words;
 }

static inline void swapLong(long * const a, long * const b)                     // Swap two long integers using xor as it is a little faster than using a temporary
 {*a = *a ^ *b;
  *b = *a ^ *b;
  *a = *a ^ *b;
 }

//static void printZ8(__m512i z)
// {for(int i = 0; i < 8; ++i)
//   {say("%2d  %8ld", i, z[i]);
//   }
// }

#if (__INCLUDE_LEVEL__ == 0)
int main()
 {assert(1 == min(1, 2));
  assert(2 == max(1, 2));
  assert(2 == abs(-2));

  if (1)
   {char **c = split("a  b  c", " ");
    assert(strcmp(c[0], "a") == 0);
    assert(strcmp(c[1], "b") == 0);
    assert(strcmp(c[2], "c") == 0);
    assert(       c[3]       == 0);
    free(c);
   }

  if (1)
   {long a = 1, b = 2;
    swapLong(&a, &b);
    assert(a == 2);
    assert(b == 1);
   }

  return 0;
 }
#endif
#endif
