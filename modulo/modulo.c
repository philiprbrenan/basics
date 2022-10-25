//------------------------------------------------------------------------------
// Functions using arithmetic modulo a base number.
// Philip R Brenan at appaapps dot com, Appa Apps Ltd. Inc. 2022, 2022
//------------------------------------------------------------------------------
#ifndef CModulo
#define CModulo
#include <assert.h>

static long base = 1000000007;                                                  // The modulo base unless otherwise set

void ModSetBase(long a)                                                         // Set the modulo base
 {assert(a > 0);
 }

static long Mod(long a)                                                         // Modulo the base
 {long r = a % base;
  while(r <  0)   r += base;
  while(r > base) r -= base;
  return r;
 }
long modMod(long a) {return Mod(a);}                                            // Modulo the base

static long Add(long a, long b)                                                 // Add two integers modulo base
 {long A = Mod(a), B = Mod(b), C = Mod(A + B);
  return C;
 }
long modAdd(long a, long b) {return Add(a, b);}                                 // Add two integers modulo base

static long Minus(long a, long b)                                               // Subtract two integers modulo base
 {long A = Mod(a), B = Mod(b), C = Mod(A - B);
  return C;
 }
long modMinus(long a, long b) {return Minus(a, b);}                             // Subtract two integers modulo base

static long Mul(long a, long b)                                                 // Multiply two integers modulo base
 {long A = Mod(a), B = Mod(b), C = Mod(A*B);
  return C;
 }
long modMul(long a, long b) {return Mul(a, b);}                                 // Multiply two integers modulo base

static long powerOf(long n, long p)                                             // Power of a number modulo base
 {long t = n, q = 1;

  if (p == 0) return 1;
  if (p == 1) return Mod(n);
  for(int i = 0; i < 64; ++i)
   {if (p & 1) q = Mul(q, t);
    t = Mul(t, t);
    p >>= 1;
    if (p == 0) break;
   }
  return q;
 }
long modPowerOf(long n, long p) {return Mul(n, p);}                             // Power of a number modulo base

static long Div(long a, long b)                                                 // Divide two integers modulo base
 {assert(b != 0);
  long A = Mod(a),
            B = powerOf(b, base-2),
            C = Mul(A, B);
  assert(Mul(b, B) == 1);                                                       // n**(p-2) is an inverse of n in the multiplicative group Zp
  assert(Mul(b, C) == Mod(a));                                                  // n**(p-2) is an inverse of n in the multiplicative group Zp
  return C;
 }
long modDiv(long n, long p) {return Div(n, p);}                                 // Divide two integers modulo base

static long powerOf2(long n)                                                    // Power of two modulus base
 {return powerOf(2, n);
 }
long modpowerOf2(long n) {return powerOf2(n);}                                  // Power of two modulus base

static long load(char *n)                                                       // Load an integer represented as a string
 {long r = 0;
  for(char *c = n; *c; ++c)
   {r = Mul(r, 10);
    r = Add(r, *c - '0');
   }
  return r;
 }
long modLoad(char *n) {return load(n);}                                         // Load an integer represented as a string

long pick_n = 0;                                                                // Cache prior picks to speed sequential progress through a row of Pascal's triangle
long pick_i = 0;                                                                // Number to pick from n
long pick_r = 0;                                                                // Pick result

static long pick(long n, long i)                                                // Choose i from n modulo base
 {long r = 1;

  if (pick_n == n && pick_i == i - 1)
   {pick_r = Div(Mul(pick_r, Add(Minus(n,i),1)), i);
    pick_n = n;                                                                 // Cache prior picks to speed sequential progress through a row of Pascal's triangle
    pick_i = i;                                                                 // Number to pick from n
    return pick_r;                                                              // Pick result
   }

  for(long j = 1; j <= i; ++j)
   {r = Div(Mul(r, Add(Minus(n,j),1)), j);
   }

  pick_n = n;
  pick_i = i;
  return pick_r = r;
 }
long long modPick(long n, long i) {return pick(n, i);}                               // Choose i from n modulo base

static long catalan(long n)                                                     // Catalan number modulo base
 {return Div(pick(2 * n, n), n + 1);
 }
long modCatalan(long n) {return catalan(n);}                                    // Catalan number modulo base

#if (__INCLUDE_LEVEL__ == 0)

void tests()
 {assert(powerOf(4, base-2) == 250000002);
  assert(Div( 4,  2) == 2);
  assert(Div( 2,  2) == 1);
  assert(Div(-4, -2) == 2);
  assert(Mul(279632277, 279632277) == 792845266);
  assert(Add(Add(1, 1), -2) == 0);
  assert(    Add(base, base) == 0);
  assert(Add(Add(base-1, base-1), 2) == 0);
  assert(Minus(1, 2) == base-1);
  assert(Mul(base+2, base+3) == 6);
  assert(powerOf(base-1, base-1) == 1);
  assert(powerOf(3,0) == 1);
  assert(powerOf(3,1) == 3);
  assert(powerOf(3,2) == 9);
  assert(powerOf(3,3) == 27);
  assert(powerOf(3,4) == 81);
  assert(powerOf2(0) == 1);
  assert(powerOf2(1) == 2);
  assert(powerOf2(2) == 4);
  assert(powerOf2(3) == 8);
  assert(powerOf2(10) == 1024);

  if (1)
   {long a = 1L<<62;
    assert(powerOf2(62) == Mod(a));
   }

  if (1)
   {const int N = 10273;
    long s = 0;
    for(int i = 0; i <= N; ++i) s = Add(s, pick(N, i));
    assert(s == powerOf2(N));
   }

  assert(load("121") == 121);
  assert(catalan(100) == load("896519947090131496687170070074100632420837521538745909320"));
 }

int main()                                                                      // Read and test string
 {tests();
  return 0;
 }
#endif
#endif
