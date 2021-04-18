#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include "gf65536.h"

void gf65536_init_tables()
{
  unsigned int x;
  unsigned int i;

  /* noop if table already initialized */
  if( gf65536_exps[0] == 1 ) return;

  x = 1;
  for( i = 0; i < 65535; ++i ) {
    /* The exp table we output from 0 to 131069 because that way when we
     * do the lagrange interpolation we don't have to be quite so strict
     * with staying inside the field which makes it quicker
     */
    gf65536_exps[i+65535] = x;
    gf65536_exps[i] = x;
    gf65536_logs[x] = i;
    x <<= 1;
    if( x & 0x10000 ) {
      /* Unset the 16th bit and mix in 0x100b which corrosponds
       * to the irreducible polynomial x^16 + x^12 + x^3 + x + 1
       */
      x ^= 0x1100b;
    }
  }
  gf65536_logs[0] = 0; /* can't log(0) so just set it neatly to 0 */
}

uint16_t gf65536_div(uint16_t a, uint16_t b) {
  if (a == 0)
    return 0;
  if (b == 0)
    abort();
  return gf65536_exps[(65535+gf65536_logs[a])-gf65536_logs[b]];
}

/* uses succesive squaring method */
uint16_t gf65536_pow(uint16_t a, uint16_t exp) {
  uint16_t total  = 1;
  uint16_t square = a;
  uint16_t mask   = 1; /*  */

  for (;;) { /*  mid-loop break on m == 0x8000 */
    if (mask > exp) break;
    if (exp & mask) {
      total = GF_MUL(total, square);
    }
    square = GF_MUL(square, square);
    if (mask == 0x8000) break;
    mask <<= 1;
  }

  return total;
}
