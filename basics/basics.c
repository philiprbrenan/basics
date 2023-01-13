//------------------------------------------------------------------------------
// Basic routines in C
// Philip R Brenan at appaapps dot com, Appa Apps Ltd. Inc., 2022
//------------------------------------------------------------------------------
#define _GNU_SOURCE
#ifndef Cbasics
#define Cbasics
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <assert.h>
#include <memory.h>
#include <string.h>
#include <stdarg.h>

void say(char *format, ...)
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

void out(char *format, ...)
 {va_list p;
  va_start (p, format);
  int i = vfprintf(stdout, format, p);
  assert(i > 0);
	va_end(p);
  fprintf(stdout, "\n");
 }

int getNextInt()
 {int i = 0;
  const int j = scanf("%d", &i);
  assert(j > 0);
  return i;
 }

char *getNextLine()
 {char *s = 0;
  size_t n = 0;
  const int j = getline(&s, &n, stdin);
  assert(j != 0);
  return s;
 }

inline int max(const int i, const int j)
 {return i > j ? i : j;
 }

inline int min(const int i, const int j)
 {return i < j ? i : j;
 }

inline int abs(const int i)
 {return i < 0 ? -i : i;
 }

int compareMemory(const char * a, const int A, const char *b, const int B)
 {const int l = min(A, B);
  const int c = memcmp(a, b, l);
  if (c != 0) return c;
  if (A < B)  return -1;
  if (A > B)  return +1;
  return  0;
 }

void printNL(const FILE *f)
 {fputc('\n', (FILE *)f);
 }

void printChars(const FILE *f, const char * a, const int A)
 {for(int i = 0; i < A; ++i)
   {fputc(a[i], (FILE *)f);
   }
 }

void printHex(const FILE *f, const char * a, const int A)
 {for(int i = 0; i < A; ++i)
   {fprintf((FILE *)f, "%02x", a[i]);
   }
  fprintf((FILE *)f, "\n");
 }

char **split(char *String, char *separator)                                     // Split a string into words
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

#if (__INCLUDE_LEVEL__ == 0)
int main()
 {printf("%d\n", getNextInt());
  printf("%d\n", getNextInt());
  printf("%d\n", getNextInt());
  getNextLine();
  printf("%s",   getNextLine());
  assert(1 == min(1, 2));
  assert(2 == max(1, 2));
  assert(2 == abs(-2));
  if (1)
   {const char *a = "a", *b = "aa";
    assert( 0 == compareMemory(a, 1, b, 1));
    assert(-1 == compareMemory(a, 1, b, 2));
    assert(+1 == compareMemory(b, 2, a, 1));
   }
  printChars(stdout, "def", 2); printNL(stdout);
  printHex  (stdout, "abc", 3); //printNL(stdout);
  if (1) {int *i = boxInt(1); assert(*i == 1);}

  if (1)
   {char **c = split("a  b  c", " ");
    assert(strcmp(c[0], "a") == 0);
    assert(strcmp(c[1], "b") == 0);
    assert(strcmp(c[2], "c") == 0);
    assert(       c[3]       == 0);
    free(c);
   }

  return 0;
 }
#endif
#endif
//TEST
/*
11
12 13
a b c
----
11
12
13
a b c
de
616263
*/
