/*
 * This file is Copyright Daniel Silverstone <dsilvers@digital-scurf.org> 2006
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT.  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 */

#include "config.h"
#include "libgfshare.h"

#include <errno.h>
#include <stdlib.h>
#include <string.h>

#define XMALLOC malloc
#define XFREE free

#define MEMCPY_W(A, B, C, D) memcpy(A, B, (C) * sizeof(D))

struct _gfshare_ctx {
  unsigned int sharecount;
  unsigned int threshold;
  unsigned int size;
  unsigned short* sharenrs;
  unsigned short* buffer;
  unsigned int buffersize;
};

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
      x ^= 0x1100b;
    }
  }
  logs[0] = 0; /* can't log(0) so just set it neatly to 0 */
}

static void
_gfshare_fill_rand_using_random( unsigned char* buffer,
                                 unsigned int count )
{
  unsigned int i;
  for( i = 0; i < count; ++i )
    buffer[i] = (random() & 0xff00) >> 8; /* apparently the bottom 8 aren't
                                           * very random but the middles ones
                                           * are
                                           */
}

gfshare_rand_func_t gfshare_fill_rand = _gfshare_fill_rand_using_random;

/* ------------------------------------------------------[ Preparation ]---- */

static gfshare_ctx *
_gfshare_ctx_init_core( unsigned short *sharenrs,
                        unsigned int sharecount,
                        unsigned int threshold,
                        unsigned int size )
{
  gfshare_ctx *ctx;
  
  ctx = XMALLOC( sizeof(struct _gfshare_ctx) );
  if( ctx == NULL )
    return NULL; /* errno should still be set from XMALLOC() */
  
  ctx->sharecount = sharecount;
  ctx->threshold = threshold;
  ctx->size = size;
  ctx->sharenrs = XMALLOC( sharecount * sizeof(short) );
  
  if( ctx->sharenrs == NULL ) {
    int saved_errno = errno;
    XFREE( ctx );
    errno = saved_errno;
    return NULL;
  }

  memcpy( ctx->sharenrs, sharenrs, sharecount * sizeof(short) );
  ctx->buffersize = threshold * size;
  ctx->buffer = XMALLOC( ctx->buffersize );
  
  if( ctx->buffer == NULL ) {
    int saved_errno = errno;
    XFREE( ctx->sharenrs );
    XFREE( ctx );
    errno = saved_errno;
    return NULL;
  }

  _gfshare_init_tables();
  
  return ctx;
}

/* Initialise a gfshare context for producing shares */
gfshare_ctx *
gfshare_ctx_init_enc( unsigned short* sharenrs,
                      unsigned int sharecount,
                      unsigned int threshold,
                      unsigned int size )
{
  unsigned int i;

  for (i = 0; i < sharecount; i++) {
    if (sharenrs[i] == 0) {
      /* can't have x[i] = 0 - that would just be a copy of the secret, in
       * theory (in fact, due to the way we use exp/log for multiplication and
       * treat log(0) as 0, it ends up as a copy of x[i] = 1) */
      errno = EINVAL;
      return NULL;
    }
  }

  return _gfshare_ctx_init_core( sharenrs, sharecount, threshold, size );
}

/* Initialise a gfshare context for recombining shares */
gfshare_ctx*
gfshare_ctx_init_dec( unsigned short* sharenrs,
                      unsigned int sharecount,
                      unsigned int size )
{
  gfshare_ctx *ctx = _gfshare_ctx_init_core( sharenrs, sharecount, sharecount, size );
  
  if( ctx != NULL )
    ctx->threshold = 0;
  
  return ctx;
}

/* Free a share context's memory. */
void 
gfshare_ctx_free( gfshare_ctx* ctx )
{
  gfshare_fill_rand( (unsigned char*)(ctx->buffer), ctx->buffersize );
  gfshare_fill_rand( (unsigned char*)(ctx->sharenrs), ctx->sharecount );
  XFREE( ctx->sharenrs );
  XFREE( ctx->buffer );
  gfshare_fill_rand( (unsigned char*)ctx, sizeof(struct _gfshare_ctx) );
  XFREE( ctx );
}

/* --------------------------------------------------------[ Splitting ]---- */

/* Provide a secret to the encoder. (this re-scrambles the coefficients) */
void 
gfshare_ctx_enc_setsecret( gfshare_ctx* ctx,
                           unsigned short* secret)
{
  memcpy( ctx->buffer + ((ctx->threshold-1) * ctx->size * sizeof(short)),
          secret,
	  ctx->size * sizeof(short) );
  gfshare_fill_rand( (unsigned char*)(ctx->buffer), (ctx->threshold-1) * ctx->size * sizeof(short) );
}

/* Extract a share from the context. 
 * 'share' must be preallocated and at least 'size' bytes long.
 * 'sharenr' is the index into the 'sharenrs' array of the share you want.
 */
void 
gfshare_ctx_enc_getshare( gfshare_ctx* ctx,
                          unsigned short sharenr,
                          unsigned short* share)
{
  unsigned int pos, coefficient;
  unsigned int ilog = logs[ctx->sharenrs[sharenr]];
  unsigned short *coefficient_ptr = ctx->buffer;
  unsigned short *share_ptr;
  for( pos = 0; pos < ctx->size; ++pos )
    share[pos] = *(coefficient_ptr++);
  for( coefficient = 1; coefficient < ctx->threshold; ++coefficient ) {
    share_ptr = share;
    for( pos = 0; pos < ctx->size; ++pos ) {
      unsigned short share_short = *share_ptr;
      if( share_short )
        share_short = exps[ilog + logs[share_short]];
      *share_ptr++ = share_short ^ *coefficient_ptr++;
    }
  }
}

/* ----------------------------------------------------[ Recombination ]---- */

/* Inform a recombination context of a change in share indexes */
void 
gfshare_ctx_dec_newshares( gfshare_ctx* ctx,
                           unsigned short* sharenrs)
{
  memcpy( ctx->sharenrs, sharenrs, ctx->sharecount * sizeof(short));
}

/* Provide a share context with one of the shares.
 * The 'sharenr' is the index into the 'sharenrs' array
 */
void 
gfshare_ctx_dec_giveshare( gfshare_ctx* ctx,
                           unsigned short sharenr,
                           unsigned short* share )
{
  memcpy( ctx->buffer + (sharenr * ctx->size * sizeof(short)), share, ctx->size * sizeof(short) );
}

/* Extract the secret by interpolation of the shares.
 * secretbuf must be allocated and at least 'size' bytes long
 */
void
gfshare_ctx_dec_extract( gfshare_ctx* ctx,
                         unsigned short* secretbuf )
{
  unsigned int i, j;
  unsigned short *secret_ptr, *share_ptr;

  for( i = 0; i < ctx->size; ++i )
    secretbuf[i] = 0;
  
  for( i = 0; i < ctx->sharecount; ++i ) {
    /* Compute L(i) as per Lagrange Interpolation */
    unsigned int Li_top = 0;
    unsigned int Li_bottom = 0;
    
    if( ctx->sharenrs[i] == 0 ) continue; /* this share is not provided. */
    
    for( j = 0; j < ctx->sharecount; ++j ) {
      if( i == j ) continue;
      if( ctx->sharenrs[j] == 0 ) continue; /* skip empty share */
      Li_top += logs[ctx->sharenrs[j]];
      if( Li_top >= 0xffff ) Li_top -= 0xffff;
      Li_bottom += logs[(ctx->sharenrs[i]) ^ (ctx->sharenrs[j])];
      if( Li_bottom >= 0xffff ) Li_bottom -= 0xffff;
    }
    if( Li_bottom  > Li_top ) Li_top += 0xffff;
    Li_top -= Li_bottom; /* Li_top is now log(L(i)) */
    
    secret_ptr = secretbuf; share_ptr = ctx->buffer + (ctx->size * i);
    for( j = 0; j < ctx->size; ++j ) {
      if( *share_ptr ) {
        *secret_ptr ^= exps[Li_top + logs[*share_ptr]];
      }
      share_ptr++; secret_ptr++;
    }
  }
}

