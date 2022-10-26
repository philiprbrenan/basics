//------------------------------------------------------------------------------
// 2d vector library
// Philip R Brenan at appaapps dot com, Appa Apps Ltd. Inc. 2022, 2022
//------------------------------------------------------------------------------
#ifndef C2d
#define C2d
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include <math.h>
#include <stdarg.h>

typedef struct Vector2d
 {double x, y;
 } Vector2d;

static Vector2d *new(double x, double y)                                        // Create a vector
 {Vector2d *v = calloc(1, sizeof(Vector2d));
  v->x = x; v->y = y;
  return v;
 }
Vector2d *vector2New(double x, double y) {return new(x, y);}                    // Create a vector

static Vector2d *Add(Vector2d *a, Vector2d *b)                                  // Add two vectors placing the result in the first vector
 {a->x += b->x;
  a->y += b->y;
  return a;
 }
Vector2d *vector2dAdd(Vector2d * a, Vector2d * b) {return Add(a, b);}           // Add two vectors placing their sum in the first vector

static Vector2d *Minus(Vector2d *a, Vector2d *b)                                // Subtract the second vector from the first placing the result in the first vector
 {a->x -= b->x;
  a->y -= b->y;
  return a;
 }
Vector2d * vector2dMinus(Vector2d * a, Vector2d * b) {return Minus(a, b);}      // Subtract the second vector from the first placing the result in the first vector

static Vector2d * Mul(double s, Vector2d * a)                                   // Multiply a vector by a scalar
 {a->x *= s;
  a->y *= s;
  return a;
 }
Vector2d * vector2dMul(double s, Vector2d * a) {return Mul(s, a);}              // Multiply a vector by a scalar

static int near(double a, double b)                                             // Check that two scalars are near to each other
 {return fabs(a - b) < 1e-6;
 }
int vector2dNear(double a, double b) {return near(a, b);}                       // Check that two scalars are near to each other

static int close(Vector2d * a, Vector2d * b)                                    // Check that two vectors are close to each other
 {return near(a->x, b->x) && near(a->y, b->y);
 }
int vector2dClose(Vector2d * a, Vector2d * b) {return close(a, b);}             // Check that two vectors are close to each other

static double length(Vector2d * a)                                              // Length of a vector
 {return hypot(a->x, a->y);
 }
double vector2dLength(Vector2d * a) {return length(a);}                         // Length of a vector

#if (__INCLUDE_LEVEL__ == 0)

void tests()
 {Vector2d * a = new(1, 0);
  assert(near(length(a), 1));
 }

int main()                                                                      // Read and test string
 {tests();
  return 0;
 }
#endif
#endif
/*

void say(char *format, ...)
 {va_list p;
  va_start (p, format);
  int i = vfprintf(stderr, format, p);
  assert(i > 0);
  va_end(p);
  fprintf(stderr, "\n");
 }

void stop(char *format, ...)
 {va_list p;
  va_start (p, format);
  int i = vfprintf(stderr, format, p);
  assert(i > 0);
  va_end(p);
  fprintf(stderr, "\n");
  exit(1);
 }
*/
