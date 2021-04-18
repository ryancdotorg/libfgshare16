/* test_gfshare_isfield copyright 2006 Simon McVittie <smcv pseudorandom co uk>
 * Released under the same terms and lack-of-warranty as libgfshare itself.
 *
 * Demonstrate that the field used in libgfshare is in fact a field, by
 * exhaustive calculation. A proper proof would be much more elegant, but
 * I need to read up on Galois fields in order to produce one.
 */

#undef NDEBUG
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

//#include "libgfshare_tables.h"

typedef unsigned char byte;

/** Assert that predicate is true. If not, display it along with the values
 * of integer variables a, b and c, which must be in scope wherever this
 * macro is used, and abort.
 */
#define myassert(predicate) \
    do { \
        if (!(predicate)) { \
            fprintf(stderr, "Assertion failed: %s (with a=%d b=%d c=%d)\n", \
                    #predicate, a, b, c); \
            abort();\
        } \
    } while (0)

/* log/exp lookup table storage. The original code precomputed them, but for
 * operation over GF(2**16) the tables are nearly 400KB. Computing at runtime
 * the first time the library is used takes at most tens of milliseconds.
 */
static unsigned short logs[65536];
static unsigned short exps[131070];

static void
_gfshare_init_tables()
{
  unsigned int x;
  unsigned int i;

  /* noop if table already initialized */
  if( exps[0] == 1 ) return;

  x = 1;
  for( i = 0; i < 65535; ++i ) {
    /* The exp table we output from 0 to 131069 because that way when we
     * do the lagrange interpolation we don't have to be quite so strict
     * with staying inside the field which makes it quicker
     */
    exps[i+65535] = x;
    exps[i] = x;
    logs[x] = i;
    x <<= 1;
    if( x & 0x10000 ) {
      /* Unset the 16th bit and mix in 0x100b which corrosponds
       * to the irreducible polynomial x^16 + x^12 + x^3 + x + 1
       */
      //x ^= 0x1100b;
      x ^= 0x1100b;
    }
  }
  logs[0] = 0; /* can't log(0) so just set it neatly to 0 */
}

/* ----------------------------------------------------------------- */
/* Naive implementations of the field operations.
 * Note that by construction, these use no more than the following lookup
 * table elements:
 * - logs[1] up to logs[255]
 * - exps[0] up to exps[254] (but not exps[255]!)
 */

/** The field addition operation a(+)b. */
static inline unsigned short plus(unsigned short a, unsigned short b)
{
    return (a^b) & 65535;
}

/** The field multiplication operation a(x)b. */
static inline unsigned short times(unsigned short a, unsigned short b)
{
    if (a == 0 || b == 0) {
        return 0;
    }
    return exps[(logs[a]+logs[b]) % 65535];
}

/** The additive inverse (-)b. */
static inline unsigned short addinv(unsigned short b)
{
    return b;
}

/** The multiplicative inverse b^{-1}. */
static inline unsigned short multiinv(unsigned short b)
{
    assert(b != 0);     /* 0 has no multiplicative inverse */
    return exps[65535 - logs[b]];
}

static void verify_naive(void)
{
    register int a, b = -1, c = -1;

    for (a = 0; a < 65536; a++) {
        /* Identities */
        myassert(plus(a, 0) == a);
        myassert(times(a, 1) == a);
        /* Inverses */
        myassert(plus(addinv(a), a) == 0);
        if (a != 0) {
            myassert(times(multiinv(a), a) == 1);
        }
        for (b = 0; b < 65536; b++) {
	    printf("a = %05u, b = %05u\n", a, b);
            /* Commutativity */
            myassert(plus(a, b) == plus(b, a));
            myassert(times(a, b) == times(b, a));
            for (c = 0; c < 65536; c++) {
                /* Associativity */
                myassert(plus(plus(a, b), c) == plus(a, plus(b, c)));
                myassert(times(times(a, b), c) == times(a, times(b, c)));
                /* Distributivity */
                myassert(times(a, plus(b, c)) == plus(times(a, b), times(a, c)));
            }
        }
    }
}

static void verify_rand(void)
{
    int a, b, c;
    unsigned int i;
    for (i = 0; i < 100000000; i++) {
        a = random() & 0xffff;
        b = random() & 0xffff;
        c = random() & 0xffff;
        /* Identities */
        myassert(plus(a, 0) == a);
        myassert(times(a, 1) == a);
        /* Inverses */
        myassert(plus(addinv(a), a) == 0);
        if (a != 0) {
            myassert(times(multiinv(a), a) == 1);
        }
        /* Commutativity */
        myassert(plus(a, b) == plus(b, a));
        myassert(times(a, b) == times(b, a));
	/*  Associativity */
	myassert(plus(plus(a, b), c) == plus(a, plus(b, c)));
	myassert(times(times(a, b), c) == times(a, times(b, c)));
	/* Distributivity */
	myassert(times(a, plus(b, c)) == plus(times(a, b), times(a, c)));
    }
}

/* ----------------------------------------------------------------- */
/* Optimized versions:
 *
 * libgfshare in fact takes some unsigned short-cuts in its implementation.
 *
 * Firstly, the exps table is twice as long as would be required by the
 * naive implementation above, with exps[255] == exps[0], ...,
 * exps[509] == exps[254]. This means that the instance of
 * exps[(logs[a] + logs[b]) & 255] seen in the field multiplication operation
 * can be replaced by exps[logs[a] + logs[b]] if logs[a], logs[b] are known to
 * be no greater than 254.
 *
 * By construction, all elements of the logs table are no greater than 254,
 * so this can be used to simplify the multiplication operation.
 *
 * Secondly, libgfshare exploits the fact that exp and log are inverses
 * to reduce the number of exp operations needed: instead of implementing
 * a(x)b(x)...(x)z naively as
 *      exps[(logs[a]+logs[exps[(logs[b] + logs[exps[...]]) % 255]]) % 255]
 * it uses
 *      exps[(logs[a] + (logs[b] + ...) % 255) % 255]
 */

static void verify_opt(void)
{
    register int a;
    int b = -1, c = -1;

    for (a = 0; a < 65536; a++) {
        if (a != 65535) {
            myassert(exps[a] == exps[a + 65535]);
            myassert(logs[exps[a]] == a);
        }
        if (a != 0) {
            myassert(logs[a] <= 65534);
            myassert(exps[logs[a]] == a);
        }
    }
}

/* ----------------------------------------------------------------- */
int main(void)
{
    printf("_gfshare_init_tables\n");
    _gfshare_init_tables();
    printf("verify_opt\n");
    verify_opt();
    printf("verify_rand\n");
    verify_rand();
    printf("verify_naive\n");
    verify_naive();
    return 0;
}
