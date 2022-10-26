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

static Vector2d *X()                                                            // Create an X vector
 {return new(1, 0);
 }
Vector2d *vector2X() {return X();}                                              // Create an X vector

static Vector2d *Y()                                                            // Create an Y vector
 {return new(0, 1);
 }
Vector2d *vector2Y() {return Y();}                                              // Create an Y vector

static Vector2d *clone(Vector2d *a)                                             // Clone a vector
 {Vector2d *v = calloc(1, sizeof(Vector2d));
  v->x = a->x; v->y = a->y;
  return v;
 }
Vector2d *vector2Clone(Vector2d *a) {return clone(a);}                          // Clone a vector

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
double Length(Vector2d * a) {return length(a);}                                 // Length of a vector

static Vector2d *normalize(Vector2d * a)                                        // Normalize a vector
 {return Mul(1.0/length(a), a);
 }
Vector2d *vector2dNormalize(Vector2d * a) {return normalize(a);}                // Normalize a vector

static Vector2d *rot90(Vector2d *a)                                             // Rotate 90 degrees ant-clockwise
 {const double x = a->x, y = a->y;
  a->x = -y; a->y = x;
  return a;
 }
Vector2d *vector2dRot90(Vector2d * a) {return rot90(a);}                        // Rotate 90 degrees ant-clockwise

static Vector2d *rot180(Vector2d *a)                                            // Rotate 180 degrees ant-clockwise
 {return rot90(rot90(a));
 }
Vector2d *vector2dRot180(Vector2d * a) {return rot180(a);}                      // Rotate 180 degrees ant-clockwise

static Vector2d *rot270(Vector2d *a)                                            // Rotate 270 degrees ant-clockwise
 {return rot90(rot90(a));
 }
Vector2d *vector2dRot270(Vector2d * a) {return rot270(a);}                      // Rotate 270 degrees ant-clockwise

static double dot(Vector2d *a, Vector2d *b)                                     // Dot product
 {return a->x * b->x + a->y * b->y;
 }
double vector2dDot(Vector2d * a, Vector2d * b) {return dot(a, b);}              // Dot product

#if (__INCLUDE_LEVEL__ == 0)

void tests()                                                                    // Tests
 {assert(near(length(X()),         1));
  assert(near(length(rot90 (X())), 1));
  assert(near(length(rot180(X())), 1));
  assert(near(length(rot270(X())), 1));
  assert(close(normalize(Mul(2, Add(X(), Y()))), new(1.0/sqrt(2), 1.0/sqrt(2))));
  assert(near(dot(X(), Y()), 0));
 }

int main()                                                                      //Test
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
